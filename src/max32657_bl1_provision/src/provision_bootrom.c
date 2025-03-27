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

#include "mxc_device.h"
#include "mcr_regs.h" // For BBREG0 register.

#include "terminal.h"
#include "menu_funcs.h"
#include "infoblock.h"

/***** Defines *****/

/***** Functions *****/
extern int secure_boot_enable(const char *parentName);

// *****************************************************************************
int provision_bootrom(void)
{
    int ret;

    terminal_printf("BBREG0 (@ 0x%08X) Status: 0x%08X\r\n", &(MXC_MCR->bypass0), MXC_MCR->bypass0);
    terminal_printf("BBREG1 (@ 0x%08X) Status: 0x%08X\r\n", &(MXC_MCR->bypass1), MXC_MCR->bypass1);
    if (MXC_MCR->bypass0 == ME30_WARM_BOOT_MAGIC_VALUE) {
        terminal_printf("Warm Boot: Enabled\r\n");
    } else {
        terminal_printf("Warm Boot: Disabled\r\n");
    }

    ret = secure_boot_enable(NULL);

    return ret;
}
