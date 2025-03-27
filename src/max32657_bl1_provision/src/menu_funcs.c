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

/* **** Includes **** */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "mxc_device.h"
#include "mcr_regs.h" // For BBREG0 register.
#include "flc.h"

#include "menu_funcs.h"
#include "terminal.h"
#include "infoblock.h"
#include "swd_lock.h"

//******************************************************************************
int swd_lock(const char *parentName)
{
    int ret = 0;
    int debug_locked;
    debug_status_t debug_stat;

    debug_locked = debug_status(&debug_stat);
    terminal_printf("\n\rLocks left = %u, Unlocks left = %u, Debug port locked = %u\r\n",
                    debug_stat.locks, debug_stat.unlocks, debug_stat.locked);
    if (debug_locked) {
        terminal_printf("Debug port is already Locked.\r\n");
    } else {
        terminal_printf("Debug port is Unlocked, attempting Lock.\r\n");
        if (!debug_stat.locks) {
            terminal_printf(
                " Lock should fail, either no locks left or the permanent bit is set.\r\n");
        }

        debug_lock();
        debug_locked = debug_status(&debug_stat);
        if (debug_locked) {
            terminal_printf("Debug port is now Locked.\r\n");
        } else {
            terminal_printf("Error: Debug port remains Unlocked.\r\n");
        }
    }

    return ret;
}

int swd_unlock(const char *parentName)
{
    int ret = 0;
    int debug_locked;
    debug_status_t debug_stat;

    debug_locked = debug_status(&debug_stat);
    terminal_printf("\nLocks left = %u, Unlocks left = %u, Debug port locked = %u\r\n",
                    debug_stat.locks, debug_stat.unlocks, debug_stat.locked);
    if (debug_locked) {
        terminal_printf("Debug port is Locked, attempting Unlock.\r\n");
        if (!debug_stat.unlocks) {
            terminal_printf(
                " Unlock should fail, either no unlocks left or the permanent bit is set.\r\n");
        }
        debug_unlock();
        debug_locked = debug_status(&debug_stat);
        if (debug_locked) {
            terminal_printf("Error: Debug port is remains Locked.\r\n");
        } else {
            terminal_printf("Debug port is now Unlocked.\r\n");
        }
    } else {
        terminal_printf("Debug port already Unlocked. Nothing to do.\r\n");
    }

    return ret;
}

int swd_set_config_permanently(const char *parentName)
{
    int ret = 0;
    int debug_locked;
    debug_status_t debug_stat;

    debug_locked = debug_status(&debug_stat);

    terminal_printf("\n\rLocks left = %u, Unlocks left = %u, Debug port locked = %u\r\n",
                    debug_stat.locks, debug_stat.unlocks, debug_stat.locked);
    terminal_printf("Debug port is currently %s\r\n", debug_locked ? "Locked." : "Unlocked.");
    terminal_printf("Will now permanently freeze the state.\r\n");

    debug_set_config_permanently();
    debug_locked = debug_status(&debug_stat);

    if (debug_locked) {
        terminal_printf("Debug port is Locked %s.\r\n",
                        debug_stat.permanent ? "Permanently" : "NOT permanently");
    } else {
        terminal_printf("Debug port is Unlocked %s.\r\n",
                        debug_stat.permanent ? "Permanently" : "NOT permanently");
    }

    return ret;
}

int swd_status(const char *parentName)
{
    int ret = 0;
    int debug_locked;
    debug_status_t debug_stat;

    debug_locked = debug_status(&debug_stat);
    (void)debug_locked;

    terminal_printf("\n\rLocks left = %u, Unlocks left = %u, Debug port locked = %s\r\n",
                    debug_stat.locks, debug_stat.unlocks, debug_stat.locked ? "YES" : "NO");
    terminal_printf("Debug status is %s.\r\n",
                    debug_stat.permanent ? "Permanent" : "NOT permanent");

    return ret;
}

/*
 * Write the ECDSA key into the infoblock
 */
int crk_write(const char *parentName)
{
    int ret = 0;
    int i;
    extern unsigned char _p_key_start[]; // defined in linker script
    extern unsigned char _p_key_end; // defined in linker script

    unsigned int key_len = (&_p_key_end - _p_key_start);

    // Is all byte 0xff ?
    for (i = 0; i < key_len; i++) {
        if (_p_key_start[i] != 0xFF) {
            break;
        }
    }

    if (i == key_len) {
        terminal_printf("\n\rCRK Invalid!\r\n");
        return -1;
    }

    terminal_hexdump("CRK:", (char *)_p_key_start, key_len);
    ret = infoblock_write(INFOBLOCK_KEY_OFFSET, _p_key_start, key_len);
    if (ret == 0) {
        terminal_printf("\n\rCRK Written!\r\n");
    }

    return ret;
}

/*
 *  Dump the ECDSA key stored in the infoblock
 */
int crk_dump(const char *parentName)
{
    int ret = 0;
    unsigned char key[INFOBLOCK_KEY_SIZE] = {
        0,
    };

    ret = infoblock_read(INFOBLOCK_KEY_OFFSET, key, sizeof(key));
    if (ret == 0) {
        terminal_hexdump("CRK:", (char *)key, sizeof(key));
    }

    return ret;
}

int secure_boot_toggle_mode(const char *parentName)
{
    // Toggle boot mode
    if (MXC_MCR->bypass0 == ME30_WARM_BOOT_MAGIC_VALUE) {
        MXC_MCR->bypass0 = 0;
        terminal_printf("\n\rWarm Boot Disabled.\r\n");
    } else {
        MXC_MCR->bypass0 = ME30_WARM_BOOT_MAGIC_VALUE;
        terminal_printf("\n\rWarm Boot Enabled.\r\n");
    }

    return 0;
}

/*
 *  Perform a check to see if the part is properly set up to be in secure boot mode
 *  This routine checks the infoblock locations necessary to enable secure boot mode,
 *  which is that debug is locked out permanently and there is something programmed in for the key.
 */
int secure_boot_is_enable(const char *parentName)
{
    int state = 0;

    state = infoblock_issecurebootenabled();
    terminal_printf("\r\nSecure Boot: %s\r\n", (state == 1) ? "Enabled" : "NOT Enabled");

    return 0;
}

int secure_boot_enable(const char *parentName)
{
    int ret;

    if (infoblock_issecurebootenabled() == 0) {
        ret = crk_write(NULL);
        if (ret == 0) {
            ret = swd_lock(NULL);
            if (ret == 0) {
                ret = swd_set_config_permanently(NULL);
            }
        }

        if (ret == 0) {
            terminal_printf("\n\rSecure boot enabled.\r\n");
        } else {
            terminal_printf("\n\rSecure boot enable FAILED.\r\n");
        }
    } else {
        terminal_printf("\n\rSecure boot already enabled.\r\n");
    }

    return 0;
}

int dump_device_infoblock(const char *parentName)
{
    int addr = MXC_INFO_MEM_BASE;
    unsigned int i;
    unsigned char buf[INFOBLOCK_LINE_SIZE * 2];
    unsigned int last_size = 8 * 1024;

    while (last_size) {
        infoblock_unlock(MXC_INFO_MEM_BASE);
        memcpy(buf, (uint8_t *)addr, sizeof(buf));
        infoblock_lock(MXC_INFO_MEM_BASE);

        for (i = 0; i < sizeof(buf); i++) {
            if (!(i % 16)) {
                terminal_printf("\n\r0x%08x:", (addr + i));
            }
            terminal_printf(" %02x", buf[i]);
        }
        last_size -= sizeof(buf);
        addr += sizeof(buf);
    }
    terminal_printf("\r\n");

    return 0;
}

int dump_user_infoblock(const char *parentName)
{
    int addr = MXC_INFO_MEM_BASE + INFOBLOCK_USER_SECTION_OFFSET;
    unsigned int i;
    unsigned char buf[INFOBLOCK_LINE_SIZE * 2];
    unsigned int last_size = 4 * 1024;

    while (last_size) {
        MXC_FLC_UnlockInfoBlock(MXC_INFO_MEM_BASE);
        memcpy(buf, (uint8_t *)addr, sizeof(buf));
        MXC_FLC_LockInfoBlock(MXC_INFO_MEM_BASE);
        for (i = 0; i < sizeof(buf); i++) {
            if (!(i % 16)) {
                terminal_printf("\n\r0x%08x:", (addr + i));
            }
            // Convert 1 to 0, 0 to 1
            terminal_printf(" %02x", buf[i] ^ 0xff);
        }
        last_size -= sizeof(buf);
        addr += sizeof(buf);
    }
    terminal_printf("\n\r");
    terminal_printf("Note:\n\r");
    terminal_printf("Each byte of this section XOR with 0xff while dumping\r\n");
    terminal_printf("to convert 1 to 0 and 0 to 1\r\n");
    terminal_printf("that match with TF-M provisioned value\r\n");
    terminal_printf("\r\n");

    return 0;
}

int dump_flash(const char *parentName)
{
    int addr = MXC_FLASH_MEM_BASE + 0xf0000 + 16 * 1024;
    unsigned int i;
    unsigned char buf[INFOBLOCK_LINE_SIZE * 2];
    unsigned int last_size = 1024;

    while (last_size) {
        MXC_FLC_Read(addr, buf, sizeof(buf));

        for (i = 0; i < sizeof(buf); i++) {
            if (!(i % 16)) {
                terminal_printf("\n\r0x%08x:", (addr + i));
            }
            terminal_printf(" %02x", buf[i]);
        }
        last_size -= sizeof(buf);
        addr += sizeof(buf);
    }
    terminal_printf("\r\n");

    return 0;
}

int erase_user_infoblock(const char *parentName)
{
    int ret;

    ret = MXC_FLC_PageErase(MXC_INFO_MEM_BASE + INFOBLOCK_USER_SECTION_OFFSET);

    return ret;
}

int mass_erase_flash(const char *parentName)
{
    MXC_FLC_MassErase();

    return 0;
}
