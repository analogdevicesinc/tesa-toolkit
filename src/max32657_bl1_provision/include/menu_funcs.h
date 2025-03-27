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

#ifndef _MENU_FUNCS_H_
#define _MENU_FUNCS_H_

/*******************************      INCLUDES    ****************************/

/*******************************      DEFINES     ****************************/
#define ME30_WARM_BOOT_MAGIC_VALUE 0xb0ccf487

/******************************* Type Definitions ****************************/

/******************************* Public Functions ****************************/
int swd_lock(const char *parentName);
int swd_unlock(const char *parentName);
int swd_set_config_permanently(const char *parentName);
int swd_status(const char *parentName);

int crk_write(const char *parentName);
int crk_dump(const char *parentName);

int secure_boot_toggle_mode(const char *parentName);
int secure_boot_is_enable(const char *parentName);
int secure_boot_enable(const char *parentName);

int dump_device_infoblock(const char *parentName);
int dump_user_infoblock(const char *parentName);
int dump_flash(const char *parentName);
int mass_erase_flash(const char *parentName);
int erase_user_infoblock(const char *parentName);

#endif // _MENU_FUNCS_H_
