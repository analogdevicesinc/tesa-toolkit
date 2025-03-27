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

#include "terminal.h"
#include "infoblock.h"

//
#define VERSION "v1.0.0"
//
#define USN_LEN 13

/***** Functions *****/
extern int provision_bootrom(void);
extern int test_menu(void);

// *****************************************************************************
int main(void)
{
    uint8_t usn[16];

    terminal_init();
    terminal_printf("\r\n\r\n");
    terminal_printf("**** MAX32657 Secure Boot ROM Provisioning FW %s ****", VERSION);
    terminal_printf("\r\n");

    terminal_printf("date: '%s'\n\r", __DATE__);
    terminal_printf("time: '%s'\n\r", __TIME__);

    int ret = infoblock_read(INFOBLOCK_USN_OFFSET, usn, USN_LEN);
    if (ret == 0) {
        terminal_hexdump("\n\rUSN:", (char *)usn, USN_LEN);
    } else {
        terminal_printf("\n\rError %d reading USN\r\n", ret);
        return -1;
    }

    //
    provision_bootrom();

    //
    //test_menu();

    while (1) {
        ;
    }
}
