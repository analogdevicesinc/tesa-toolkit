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
#ifndef _SWD_LOCK_H_
#define _SWD_LOCK_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup    swd_lock    Routines for locking/unlocking the Debug port
 * @brief       Routines to assist with locking and unlocking the SWD debug port
 * @{
 */

/**
 * @defgroup    swd_lock_defines    Defines for SWD lock/unlock and status
 * @brief       Defines and structures for SWD lock/unlock
 * @details     Structure for returning status of debug lock/unlock.
 * @{
 */

/**
 * @brief    Structure type for status of debug lock/unlock.
 */
typedef struct {
    unsigned int locks; /**< number of locks left */
    unsigned int unlocks; /**< number of unlocks left */
    unsigned int locked; /**< 1 if part is locked, 0 if unlocked */
    unsigned int
        permanent; /**< 1 if lock information is unchangeable, 0 if we can still modify it. */
} debug_status_t;

/**@} end of group swd_lock_defines */

/**
 * @brief debug_lock    Locks the debug port.
 * @return      error_code    error if unable to lock the debug port
 * @retval      E_NO_ERROR    Locking was successful
 * @retval      E_BAD_STATE    Unable to lock the debug port
 */
int debug_lock(void);

/**
 * @brief debug_unlock  Unlocks the debug port.
 * @return      error_code    error if unable to unlock the debug port
 * @retval      E_NO_ERROR    Unlocking was successful
 * @retval      E_BAD_STATE    Unable to unlock the debug port
 */
int debug_unlock(void);

/**
 * @brief debug_status  Returns the status of the locking mechanism for the debug port.
 * @param[out]  ptr     Structure indicating the status of the lock
 * @details     There are 4 locations that start out unprogrammed.
                Any one of those 4 locations can be programmed with the correct value to lock debug.
                A location with the proper lock value can be further modified (such as writing to 0x00) to remove the lock.
				Once a specific lock location has been unlocked, that location cannot be used again.
                Once all 4 locations have been set to lock and subsequently unlocked/cleared, we can no longer lock the debug port again.
                Permanent bit is the bit in the infoblock line that when cleared prevents any further writes to that line.
 * @return      lock/unlock status  1=locked, 0=unlocked
 * @retval      1       Locked
 * @retval      0       Unlocked
 */
unsigned int debug_status(debug_status_t *ptr);

/**
 * @brief debug_set_config_permanently   Makes the current locked/unlocked debug status permanent.
 * @note     Once the permanent bit is set, you can no longer change the debug lock/unlock
 * @return      error_code    error if unable to set the permanent bit
 * @retval      E_NO_ERROR    Permanent bit is set.
 */
int debug_set_config_permanently(void);

/**@} end of group swd_lock */

#ifdef __cplusplus
}
#endif

#endif /* _SWD_LOCK_H_ */
