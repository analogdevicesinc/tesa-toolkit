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

#ifndef __PACKED_STRUCT
#define __PACKED_STRUCT struct __attribute__((packed, aligned(1)))
#endif /* __PACKED_STRUCT */

typedef __PACKED_STRUCT
{
    uint8_t huk[32];
    uint8_t iak[32];
    uint32_t iak_len;
    uint32_t iak_type;
    uint8_t iak_id[32];

    uint8_t boot_seed[32];
    uint32_t lcs;
    uint8_t implementation_id[32];
    uint8_t cert_ref[32];
    uint8_t verification_service_url[32];
    uint8_t profile_definition[32];

    uint8_t bl2_rotpk_0[100];
    uint8_t bl2_rotpk_1[100];
    uint8_t bl2_rotpk_2[100];
    uint8_t bl2_rotpk_3[100];

    uint8_t bl2_nv_counter_0[64];
    uint8_t bl2_nv_counter_1[64];
    uint8_t bl2_nv_counter_2[64];
    uint8_t bl2_nv_counter_3[64];

    uint8_t ns_nv_counter_0[64];
    uint8_t ns_nv_counter_1[64];
    uint8_t ns_nv_counter_2[64];

    uint8_t entropy_seed[64];
    uint8_t secure_debug_pk[32];
}
max32657_otp_nv_counters_region_t;

//******************************************************************************
static int get_bl2_provision_info(max32657_otp_nv_counters_region_t *counters)
{
    int addr = MXC_INFO_MEM_BASE + 0x3000;
    unsigned int len = sizeof(max32657_otp_nv_counters_region_t);
    unsigned char *ptr = (unsigned char *)counters;

    MXC_FLC_UnlockInfoBlock(MXC_INFO_MEM_BASE);
    memcpy(ptr, (uint8_t *)addr, len);
    MXC_FLC_LockInfoBlock(MXC_INFO_MEM_BASE);

    // XOR with 0xff
    for (unsigned int i = 0; i < len; i++) {
        *ptr = *ptr ^ 0xff;
        ptr++;
    }

    return 0;
}

int dump_bl2_params(const char *parentName)
{
    max32657_otp_nv_counters_region_t bl2_info;

    get_bl2_provision_info(&bl2_info);

    terminal_hexdump("\n\rHUK", (char *)bl2_info.huk, sizeof(bl2_info.huk));
    terminal_hexdump("\n\rIAK", (char *)bl2_info.iak, sizeof(bl2_info.iak));
    terminal_printf("\n\rIAK Len : 0x%08X\n\r", bl2_info.iak_len);
    terminal_printf("\n\rIAK Type: 0x%08X\n\r", bl2_info.iak_type);
    terminal_hexdump("\n\rIAK ID", (char *)bl2_info.iak_id, sizeof(bl2_info.iak_id));

    terminal_hexdump("\n\rBOOT SEED", (char *)bl2_info.boot_seed, sizeof(bl2_info.boot_seed));
    terminal_printf("\n\rLCS: 0x%08X\n\r", bl2_info.lcs);
    terminal_hexdump("\n\rImplementation ID", (char *)bl2_info.implementation_id,
                     sizeof(bl2_info.implementation_id));
    terminal_hexdump("\n\rCert Ref", (char *)bl2_info.cert_ref, sizeof(bl2_info.cert_ref));
    terminal_hexdump("\n\rVerification Service URL", (char *)bl2_info.verification_service_url,
                     sizeof(bl2_info.verification_service_url));
    terminal_hexdump("\n\rProfile Definition", (char *)bl2_info.profile_definition,
                     sizeof(bl2_info.profile_definition));

    terminal_hexdump("\n\rbl2_rotpk_0", (char *)bl2_info.bl2_rotpk_0, sizeof(bl2_info.bl2_rotpk_0));
    terminal_hexdump("\n\rbl2_rotpk_1", (char *)bl2_info.bl2_rotpk_1, sizeof(bl2_info.bl2_rotpk_1));
    terminal_hexdump("\n\rbl2_rotpk_2", (char *)bl2_info.bl2_rotpk_2, sizeof(bl2_info.bl2_rotpk_2));
    terminal_hexdump("\n\rbl2_rotpk_3", (char *)bl2_info.bl2_rotpk_3, sizeof(bl2_info.bl2_rotpk_3));

    terminal_hexdump("\n\rbl2_nv_counter_0", (char *)bl2_info.bl2_nv_counter_0,
                     sizeof(bl2_info.bl2_nv_counter_0));
    terminal_hexdump("\n\rbl2_nv_counter_1", (char *)bl2_info.bl2_nv_counter_1,
                     sizeof(bl2_info.bl2_nv_counter_1));
    terminal_hexdump("\n\rbl2_nv_counter_2", (char *)bl2_info.bl2_nv_counter_2,
                     sizeof(bl2_info.bl2_nv_counter_2));
    terminal_hexdump("\n\rbl2_nv_counter_3", (char *)bl2_info.bl2_nv_counter_3,
                     sizeof(bl2_info.bl2_nv_counter_3));

    terminal_hexdump("\n\rns_nv_counter_0", (char *)bl2_info.ns_nv_counter_0,
                     sizeof(bl2_info.ns_nv_counter_0));
    terminal_hexdump("\n\rns_nv_counter_1", (char *)bl2_info.ns_nv_counter_1,
                     sizeof(bl2_info.ns_nv_counter_1));
    terminal_hexdump("\n\rns_nv_counter_2", (char *)bl2_info.ns_nv_counter_2,
                     sizeof(bl2_info.ns_nv_counter_2));

    terminal_hexdump("\n\rentropy_seed", (char *)bl2_info.entropy_seed,
                     sizeof(bl2_info.entropy_seed));
    terminal_hexdump("\n\rsecure_debug_pk", (char *)bl2_info.secure_debug_pk,
                     sizeof(bl2_info.secure_debug_pk));

    terminal_printf("\r\n");

    return 0;
}
