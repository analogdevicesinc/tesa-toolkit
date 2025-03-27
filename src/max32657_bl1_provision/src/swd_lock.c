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

/***** Includes *****/
#include <stdio.h>
#include <stdint.h>

#include "swd_lock.h"
#include "infoblock.h"

#define E_NO_ERROR 0
#define E_BAD_STATE -7
#define E_UNKNOWN -8

//
// Locks the debug port, and returns E_NO_ERROR if successful.
// Returns E_BAD_STATE if unable to lock the debug port.
//
int debug_lock(void)
{
    debug_status_t st;
    int result = E_UNKNOWN;
    int i;
    uint16_t data16[INFOBLOCK_WRITE_LOCK_LINE_SIZE / sizeof(uint16_t)];
    uint32_t lockoffset;

    // Check for any existing lock
    if (debug_status(&st)) {
        // Already locked
        return E_NO_ERROR;
    }
    // Not locked, can we lock it?
    if (st.locks == 0) {
        // No unused lock locations, so we cannot lock again
        return E_BAD_STATE;
    }

    // We can unlock -- Look at all lock words if needed
    lockoffset = INFOBLOCK_ICE_LOCK_OFFSET & ~(INFOBLOCK_WRITE_LOCK_LINE_SIZE - 1);
    infoblock_read(lockoffset, (uint8_t *)data16, INFOBLOCK_WRITE_LOCK_LINE_SIZE);
    result = E_BAD_STATE;
    for (i = 0; i < (INFOBLOCK_ICE_LOCK_SIZE / sizeof(uint16_t)); ++i) {
        // look for an unmodified lock location to write to
        if (data16[i] != ICELOCK_UNMODIFIED_VALUE) {
            continue;
        }

        // found a location to write
        if ((i % 2) == 0) // even and odd locations have a different value
        {
            data16[i] = ICELOCK_EVEN_LOCK_VALUE;
        } else {
            data16[i] = ICELOCK_ODD_LOCK_VALUE;
            if (i == 3) { // Make sure that we don't set the permanent bit to 0
                // On some parts the lock bit is the top bit of the last lock location
                data16[3] |= (1 << 15);
            }
        }
        result = infoblock_write(lockoffset, (uint8_t *)data16, INFOBLOCK_WRITE_LOCK_LINE_SIZE);
        break;
    }

    return result; // return either E_BAD_STATE if no locations to write, or the infoblock_write status
}

//
// Unlocks the debug port, and returns E_NO_ERROR if successful.
//
int debug_unlock(void)
{
    debug_status_t st;
    int result, i;
    uint16_t data16[INFOBLOCK_WRITE_LOCK_LINE_SIZE / sizeof(uint16_t)];
    uint32_t lockoffset;

    // Check for any existing lock
    if (!debug_status(&st)) {
        // Already unlocked
        return E_NO_ERROR;
    }
    // Are we able to unlock?
    if (st.unlocks == 0) {
        // No locations left to unlock
        return E_BAD_STATE;
    }

    // align infoblock read to the line lock size since this may change from part to part
    lockoffset = INFOBLOCK_ICE_LOCK_OFFSET & ~(INFOBLOCK_WRITE_LOCK_LINE_SIZE - 1);
    infoblock_read(lockoffset, (uint8_t *)data16, INFOBLOCK_WRITE_LOCK_LINE_SIZE);
    // Otherwise, can unlock -- be thorough about finding all lock words
    result = E_BAD_STATE;
    for (i = 0; i < (INFOBLOCK_ICE_LOCK_SIZE / sizeof(uint16_t)); ++i) {
        // look for a location with the lock word, so we can clear it
        if ((i % 2) == 0) {
            if (data16[i] != ICELOCK_EVEN_LOCK_VALUE)
                continue;
            // found a locked location, so clear it.
            data16[i] = 0x0000;
        } else {
            // On some parts the permanent lock bit is the top bit of the last lock location
            if (i == 3) {
                // So the value can be either 0x5A5A or 0xDA5A
                if ((data16[i] & 0x7FFF) != ICELOCK_ODD_LOCK_VALUE)
                    continue;
                // found a locked location to clear
                // Do not set the permanent lock bit to 0 until we want it to be permanent
                data16[i] = (1 << 15);
            } else { // (i == 1) in this case
                if (data16[i] != ICELOCK_ODD_LOCK_VALUE)
                    continue;
                data16[i] = 0x0000;
            }
        }
        result = infoblock_write(lockoffset, (uint8_t *)data16, INFOBLOCK_WRITE_LOCK_LINE_SIZE);
        break;
    }

    return result;
}

// Returns the current state (locked/unlocked)
unsigned int debug_status(debug_status_t *ptr)
{
    unsigned int locks, unlocks, locked, i, permanent;
    unsigned int currently_locked_locations, currently_unmodified_locations;
    uint8_t data8[INFOBLOCK_WRITE_LOCK_LINE_SIZE];
    uint16_t *data16 = (uint16_t *)&data8[0];
    uint32_t lockoffset;

    locked = locks = unlocks = permanent = 0;
    currently_locked_locations = currently_unmodified_locations = 0;
    // align infoblock read to the line lock size since this may change from part to part
    lockoffset = INFOBLOCK_ICE_LOCK_OFFSET & ~(INFOBLOCK_WRITE_LOCK_LINE_SIZE - 1);
    infoblock_read(lockoffset, data8, INFOBLOCK_WRITE_LOCK_LINE_SIZE);

#ifdef SWD_LOCK_DEBUG
    printf("[debug_lock_words] Lock0=0x%04x Lock1=0x%04x Lock2=0x%04x Lock3=0x%04x Permanent=>%s\n",
           data16[0], data16[1], data16[2], data16[3],
           ((data8[INFOBLOCK_WRITE_LOCK_LINE_SIZE - 1] & (1 << 7)) == 0) ? "0=Yes" : "1=No");
#endif /* SWD_LOCK_DEBUG */

    for (i = 0; i < (INFOBLOCK_ICE_LOCK_SIZE / sizeof(uint16_t)); ++i) {
        // any unmodifed locations are available to set to lock
        if (data16[i] == ICELOCK_UNMODIFIED_VALUE) {
            currently_unmodified_locations++;
            continue;
        }
        // alternating 16 bit values are 0xA5A5 and then 0x5A5A
        if ((i % 2) == 0) {
            if (data16[i] == ICELOCK_EVEN_LOCK_VALUE) {
                currently_locked_locations++;
            }
        } else {
            // special case, sometimes the permanent bit is the top bit of this field, so it can be 0 or 1
            // top bit can be the permanent lock bit in some parts
            if (i == 3) {
                if ((data16[i] & 0x7FFF) == ICELOCK_ODD_LOCK_VALUE) {
                    currently_locked_locations++;
                }
            } else { // normal case, must match exactly
                if (data16[i] == ICELOCK_ODD_LOCK_VALUE) {
                    currently_locked_locations++;
                }
            }
        }
    }
    // NOTE: currently_unmodified_locations is the count of unmodified (0xFFFF) locations
    //       currently_unlocked_locations is the count of locations currently set to the 'lock' value
    //       Any currently locke location can be unlocked by clearing that location
    //       We need only one 'lock' value to be locked.
    if (currently_locked_locations) {
        // Part is locked
        locked = 1;
    }

    // unmodified locations can be set to the lock value
    // so the number of these is the number of times we can lock
    locks = currently_unmodified_locations;
    // any unmodified lock locations also count toward unlocks because we can lock and then unlock that location
    unlocks = currently_unmodified_locations + currently_locked_locations;

    if ((data8[INFOBLOCK_WRITE_LOCK_LINE_SIZE - 1] & (1 << 7)) == 0) {
        // Can't do anything more, we are frozen, the hardware prevents us from writing to this infoblock line
        // when the top bit is cleared.
        locks = unlocks = 0;
        permanent = 1;
    }

    if (ptr) {
        ptr->locks = locks;
        ptr->unlocks = unlocks;
        ptr->locked = locked;
        ptr->permanent = permanent;
    }

    return locked;
}

int debug_set_config_permanently(void)
{
    int result;

    uint8_t data8[INFOBLOCK_WRITE_LOCK_LINE_SIZE];
    uint32_t lockoffset;

    // align infoblock read to the line lock size since this may change from part to part
    lockoffset = INFOBLOCK_ICE_LOCK_OFFSET & ~(INFOBLOCK_WRITE_LOCK_LINE_SIZE - 1);
    result = infoblock_read(lockoffset, data8, INFOBLOCK_WRITE_LOCK_LINE_SIZE);
    if (result != E_NO_ERROR) {
        return (result);
    }

    // top bit if cleared = permanent
    if ((data8[INFOBLOCK_WRITE_LOCK_LINE_SIZE - 1] & (1 << 7)) == 0) {
#ifdef SWD_LOCK_DEBUG
        printf("Already permanently locked.\n");
#endif /* SWD_LOCK_DEBUG */
        result = E_NO_ERROR;
    } else {
        // clear top bit = permanent lock bit
        data8[INFOBLOCK_WRITE_LOCK_LINE_SIZE - 1] &= ~(1 << 7);
        result = infoblock_write(lockoffset, data8, INFOBLOCK_WRITE_LOCK_LINE_SIZE);
    }

    return result;
}
