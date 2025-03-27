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

#ifndef _TERMINAL_H_
#define _TERMINAL_H_

/*******************************      INCLUDES    ****************************/

/*******************************      DEFINES     ****************************/
#define KEY_ESC -0x1B
#define KEY_CANCEL -0x1B
#define KEY_ENTER -0x0A

/******************************* Type Definitions ****************************/
typedef struct {
    const char *name;
    int (*callback)(const char *parentName);
} list_t;

/******************************* Public Functions ****************************/
int terminal_init(void);
int terminal_printf(const char *format, ...);
void terminal_hexdump(const char *title, char *buf, unsigned int len);
int terminal_read_num(unsigned int timeout);
int terminal_select_from_list(const char *title, const list_t *items, int nb_items, int nb_col);

#endif // _TERMINAL_H_
