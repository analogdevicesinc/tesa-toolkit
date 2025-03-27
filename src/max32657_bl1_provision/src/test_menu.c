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
extern int dump_bl2_params(const char *parentName);


/***** Variables *****/
static list_t list[] = {
    { "Print BL2 Provision Configurations", dump_bl2_params },
    { "Dump Device Info Block", dump_device_infoblock },
    { "Dump User Info Block", dump_user_infoblock },
    { "Erase User Info Block", erase_user_infoblock },
    { "Mass Erase FLC", mass_erase_flash },
};

// *****************************************************************************
int test_menu(void)
{
    int ret = 0;

    while (1) {
        terminal_select_from_list("Test Menu", list, sizeof(list) / sizeof(list[0]), 1);
    }

    return ret;
}
