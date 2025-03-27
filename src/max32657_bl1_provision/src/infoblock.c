/******************************************************************************
 *
 * Copyright (C) 2025 Analog Devices, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "infoblock.h"
#include "mxc_device.h"
#include "flc.h"

uint16_t crc15_highbitinput(uint16_t crc15val, uint8_t *input, int bitlength)
{
    uint16_t inputbit;
    uint16_t feedbackbit;
    int i;

    for (i = 0; i < bitlength; i++) {
        inputbit = (input[i / 8] >> (7 - i % 8)) & 0x01;
        feedbackbit = ((crc15val & 0x4000) >> 14) & 0x01;
        crc15val <<= 1;
        if (inputbit ^ feedbackbit) {
            crc15val ^= 0x4599;
        }
    }
    // Clean up.
    // Mask off any high bits we were ignoring in the loop.
    crc15val &= 0x7FFF;

    return crc15val;
}

int infoblock_readraw(uint32_t offset, uint8_t *data)
{
    int result;

    result = infoblock_unlock(MXC_INFO_MEM_BASE);
    if (result != E_NO_ERROR) {
        return result;
    }
    memcpy(data, (uint8_t *)(MXC_INFO_MEM_BASE + offset), INFOBLOCK_LINE_SIZE);

    result = infoblock_lock(MXC_INFO_MEM_BASE);

    return result;
}

int infoblock_read(uint32_t offset, uint8_t *data, int length)
{
    uint8_t oneinfoblockline[INFOBLOCK_LINE_SIZE];
    int lengthtocopy;
    uint16_t crc = 0;
    uint16_t crcexpected;
    int i;
    int result;
    lineformat_e lineformat;

    if (length > INFOBLOCK_MAXIMUM_READ_LENGTH) {
        return E_BAD_PARAM;
    }

    if (data == NULL) {
        return E_BAD_PARAM;
    }

    switch (offset) {
    case INFOBLOCK_USN_OFFSET:
        lineformat = INFOBLOCK_LINE_FORMAT_USN;
        break;
    case INFOBLOCK_FMV_OFFSET:
    case INFOBLOCK_ICE_LOCK_OFFSET:
        lineformat = INFOBLOCK_LINE_FORMAT_RAW;
        break;
    case INFOBLOCK_KEY_OFFSET:
        lineformat = INFOBLOCK_LINE_FORMAT_DESIGN;
        break;
    default:
        return E_BAD_PARAM;
        break;
    }

    while (length > 0) {
        if ((result = infoblock_readraw(offset, oneinfoblockline)) != E_NO_ERROR) {
            return result;
        }

        switch (lineformat) {
        case INFOBLOCK_LINE_FORMAT_USN:
            // NO CRC15, ignore lowest 15 bits
            // Lock bit is high bit, bit 63.
            // Data is middle 48 bits 62 to 15.
            // CRC15 is lower 15 bits (bits 0-14)
            // First shift data one bit left starting at the high byte.
            for (i = 7; i > 1; i--) {
                oneinfoblockline[i] <<= 1;
                oneinfoblockline[i] |= (oneinfoblockline[i - 1] & 0x80) >> 7;
            }
            // Then, shift data by two bytes
            memmove(oneinfoblockline, oneinfoblockline + INFOBLOCK_LINE_OVERHEAD, 6);
            lengthtocopy = INFOBLOCK_LINE_SIZE - INFOBLOCK_LINE_OVERHEAD;
            break;
        case INFOBLOCK_LINE_FORMAT_RAW:
            lengthtocopy = INFOBLOCK_LINE_SIZE;
            break;
        case INFOBLOCK_LINE_FORMAT_DESIGN:
            // Check for unprogrammed information block line
            crc = 0xFF;
            for (i = 0; i < sizeof(oneinfoblockline); i++) {
                crc &= oneinfoblockline[i];
            }
            if (crc == 0xFF) {
                // Line is unprogrammed, return error.
                memset(data, 0xff, length);
                return 0; //E_BAD_STATE;
            }
            // Lock bit is high bit, bit 63.
            // CRC15 is middle 15 bits [62:48]
            // Data is lower 48 bits [47:0].
            // First, CRC the lock bit.
            crc = crc15_highbitinput(0, oneinfoblockline + 7, 1);
            // Then CRC the data from high to low bits.  (bit 47 to 0)
            for (i = 5; i >= 0; i--) {
                crc = crc15_highbitinput(crc, oneinfoblockline + i, 8);
            }
            crcexpected = ((oneinfoblockline[7] & 0x7F) << 8) | oneinfoblockline[6];
            if (crc != crcexpected) {
                return E_BAD_STATE;
            }
            lengthtocopy = INFOBLOCK_LINE_SIZE - INFOBLOCK_LINE_OVERHEAD;
            break;
        default:
            // NOTE: Should never get here.
            return E_BAD_STATE;
            break;
        }

        if (lengthtocopy > length) {
            lengthtocopy = length;
        }
        memcpy(data, oneinfoblockline, lengthtocopy);
        data += lengthtocopy;
        length -= lengthtocopy;
        offset += INFOBLOCK_LINE_SIZE;
    }

    return E_NO_ERROR;
}

int infoblock_checkenable_generic(int offset, int minimumpatterncount)
{
    int i;
    int count = 0;
    uint32_t data[INFOBLOCK_ENABLE_SIZE / sizeof(uint32_t)];
    uint32_t value;

    // For an Information Block line enable to be valid, two 16-bit values out of 64 must be correct.
    count = 0;
    if (infoblock_read(offset, (uint8_t *)data, sizeof(data)) == E_NO_ERROR) {
        for (i = 0; i < (INFOBLOCK_ENABLE_SIZE / sizeof(uint32_t)); i++) {
            // XOR to compare expected vs actual value.
            value = data[i] ^ INFOBLOCK_ENABLE_PATTERN;
            // Mask low 16-bits
            if ((value & 0x0000FFFF) == 0) {
                count++;
            }
            // Mask high 16-bits
            if ((value & 0xFFFF0000) == 0) {
                count++;
            }
        }

        if (count >= minimumpatterncount) {
            return TRUE;
        }
    }

    return FALSE;
}

int infoblock_checkenable_icelock()
{
    return infoblock_checkenable_generic(INFOBLOCK_ICE_LOCK_OFFSET, INFOBLOCK_ICE_LOCK_MINIMUM);
}

int infoblock_checkpermicelock()
{
    // Make space to read entire flash line to check the flash line lock.
    uint8_t data[INFOBLOCK_WRITE_LOCK_LINE_SIZE];
    uint32_t infooffset, offset, length;

    // To enable secure boot
    // 1. ICE_LOCK must be enabled AND
    // 2. ICE_LOCK must be permanent (flash line lock enabled).

    // First, check to see if ICE_LOCK is enabled (ICE is locked out).
    if (infoblock_checkenable_icelock() == FALSE) {
        return FALSE;
    }

    // Then, check to see if the last bit of the write lock line is cleared (bit clear makes word read-only)
    offset = 0;
    length = INFOBLOCK_WRITE_LOCK_LINE_SIZE;
    // Get the proper aligned write lock line address which contains the ICE_LOCK word.
    infooffset = INFOBLOCK_ICE_LOCK_OFFSET & ~(INFOBLOCK_WRITE_LOCK_LINE_SIZE - 1);
    // Read as many INFOBOCK lines which fit into the INFOBLOCK write lock lines.
    // NOTE: On MAX32655, this will do 2 reads because the flash is 128-bit wide.
    // INFOBLOCK Write lock lines are thus 128 bits and the INFOBLOCK storage lines are 64-bit.
    while (length > 0) {
        if (infoblock_readraw(infooffset, data + offset) != E_NO_ERROR) {
            return FALSE;
        }
        infooffset += INFOBLOCK_LINE_SIZE;
        offset += INFOBLOCK_LINE_SIZE;
        length -= INFOBLOCK_LINE_SIZE;
    }

    // Check for zero lock bit (last bit of last byte in flash line).
    if ((data[INFOBLOCK_WRITE_LOCK_LINE_SIZE - 1] & 0x80) == 0) {
        return TRUE;
    }

    return FALSE;
}

int infoblock_issecurebootenabled()
{
    uint32_t oneinfoblockline[INFOBLOCK_LINE_SIZE / sizeof(uint32_t)];
    int i;
    uint32_t valueand;
    int keylength, infooffset;

    // Device is in Secure/Closed Mode if
    // 1. Key area of Information Block is written.
    // 2. ICE_LOCK is locked and Information Block Line Write bit is cleared. (bit 127)
    //
    // Read the raw key area of information block and see if it has been programmed.
    valueand = 0xFFFFFFFF;
    keylength = INFOBLOCK_KEY_SIZE;
    infooffset = INFOBLOCK_KEY_OFFSET;
    while (keylength > 0) {
        // Read one information block line.
        infoblock_readraw(infooffset, (uint8_t *)oneinfoblockline);

        // Loop across key and accumulate zeros and ones.
        for (i = 0; i < (INFOBLOCK_LINE_SIZE / sizeof(uint32_t)); i++) {
            valueand &= oneinfoblockline[i];
        }
        infooffset += INFOBLOCK_LINE_SIZE;
        keylength -= (INFOBLOCK_LINE_SIZE - INFOBLOCK_LINE_OVERHEAD);
    }
    // If key had all 1s, it is not programmed and the device is not in Secure Mode.
    if (valueand == 0xFFFFFFFF) {
        return FALSE;
    }
    // Key has been programmed, now check permanent ICE LOCK (aka SWD disable).
    if (infoblock_checkpermicelock() == TRUE) {
        // Key exists and ICE is locked out permanently.
        return TRUE;
    } else {
        return FALSE;
    }
}

int infoblock_writeraw(uint32_t offset, uint32_t *data)
{
    int result;
    int writeresult = E_NO_ERROR;

    if ((result = infoblock_unlock(MXC_INFO_MEM_BASE)) != E_NO_ERROR) {
        return result;
    }

    writeresult = MXC_FLC_Write((MXC_INFO_MEM_BASE + offset), INFOBLOCK_LINE_SIZE, data);

    if ((result = infoblock_lock(MXC_INFO_MEM_BASE)) != E_NO_ERROR) {
        return result;
    }

    return writeresult;
}

int infoblock_write(uint32_t offset, uint8_t *data, int length)
{
    uint32_t oneinfoblockline_32[INFOBLOCK_LINE_SIZE / sizeof(uint32_t)];
    uint8_t *oneinfoblockline = (uint8_t *)oneinfoblockline_32;
    int lengthtowrite;
    uint16_t crc = 0;
    int i;
    int result;
    lineformat_e lineformat;

    if (length > INFOBLOCK_MAXIMUM_READ_LENGTH) {
        return E_BAD_PARAM;
    }

    if (data == NULL) {
        return E_BAD_PARAM;
    }

    switch (offset) {
    case INFOBLOCK_ICE_LOCK_OFFSET:
        lineformat = INFOBLOCK_LINE_FORMAT_RAW;
        break;
    case INFOBLOCK_KEY_OFFSET:
        lineformat = INFOBLOCK_LINE_FORMAT_DESIGN;
        break;
    default:
        return E_BAD_PARAM;
        break;
    }

    while (length > 0) {
        switch (lineformat) {
        case INFOBLOCK_LINE_FORMAT_RAW:
            lengthtowrite = INFOBLOCK_LINE_SIZE;
            break;
        case INFOBLOCK_LINE_FORMAT_DESIGN:
            lengthtowrite = INFOBLOCK_LINE_SIZE - INFOBLOCK_LINE_OVERHEAD;
            break;
        default:
            // NOTE: Should never get here.
            return E_BAD_STATE;
            break;
        }

        if (lengthtowrite > length) {
            lengthtowrite = length;
        }
        memset(oneinfoblockline, 0, INFOBLOCK_LINE_SIZE);
        memcpy(oneinfoblockline, data, lengthtowrite);

        switch (lineformat) {
        case INFOBLOCK_LINE_FORMAT_RAW:
            // No change to oneinfoblockline
            break;
        case INFOBLOCK_LINE_FORMAT_DESIGN:
            // Lock bit is high bit, bit 63.
            // CRC15 is middle 15 bits [62:48]
            // Data is lower 48 bits [47:0].
            // First, CRC the lock bit.
            crc = crc15_highbitinput(0, oneinfoblockline + 7, 1);
            // Then CRC the data from high to low bits.  (bit 47 to 0) =  6 bytes (5..0)
            for (i = 5; i >= 0; i--) {
                crc = crc15_highbitinput(crc, oneinfoblockline + i, 8);
            }
            oneinfoblockline[7] &= ~0x7F; // keep only the lock bit
            oneinfoblockline[7] |= crc >> 8; // put in the top 7 bits of CRC
            oneinfoblockline[6] = (uint8_t)crc; // bottom 8 bits of crc
            break;
        default:
            // NOTE: Should never get here.
            return E_BAD_STATE;
            break;
        }

        if ((result = infoblock_writeraw(offset, oneinfoblockline_32)) != E_NO_ERROR) {
            return result;
        }

        data += lengthtowrite;
        length -= lengthtowrite;
        offset += INFOBLOCK_LINE_SIZE;
    }

    return E_NO_ERROR;
}

int infoblock_unlock(uint32_t address)
{
    int ret;

    ret = MXC_FLC_UnlockInfoBlock(address);

    return ret;
}

int infoblock_lock(uint32_t address)
{
    int ret;

    ret = MXC_FLC_LockInfoBlock(address);

    return ret;
}
