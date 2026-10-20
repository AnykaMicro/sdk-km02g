/* SPDX-License-Identifier: Apache-2.0 */
/**
 * @file ai_glass_time.h
 * @brief Timer utility header file for measuring elapsed time.
 *
 * This header file declares functions and data structures for high-resolution time
 * measurement using the monotonic clock (CLOCK_MONOTONIC). It provides a simple interface
 * to start and stop a timer, and calculate the elapsed time in milliseconds.
 *
 * @date 2025-09-25
 * @version 1.0
 * @copyright Copyright (c) 2025 Guangzhou Anyka Microelectronics Co., Ltd.
 */

#ifndef _AI_GLASS_TIME_H__
#define _AI_GLASS_TIME_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "ai_glass_common.h"

/**
 * @brief Timer structure to store start and end time points.
 *
 * This structure holds two timespec values: one for the start time and one for the end time,
 * used to calculate elapsed time between timer_start and timer_stop calls.
 */
typedef struct {
    struct timespec start;  ///< Start time recorded by timer_start
    struct timespec end;    ///< End time recorded by timer_stop
} ai_glass_timer_t;

/**
 * @brief Start the timer by recording the current time.
 *
 * @param timer Pointer to the timer structure to initialize.
 */
void ai_glass_timer_start(ai_glass_timer_t *timer);

/**
 * @brief Stop the timer and return the elapsed time in milliseconds.
 *
 * @param timer Pointer to the timer structure previously started with timer_start.
 * @return Elapsed time in milliseconds (double precision).
 */
double ai_glass_timer_stop(ai_glass_timer_t *timer);

/**
 * @brief Initialize global time-related variables and system clock reference.
 */
void init_global_time(void);

/**
 * @brief Print the elapsed time of the current timer for performance monitoring.
 *
 * @param func_name Name of the function being timed.
 * @param line_num Line number where the timing measurement was taken.
 */
void print_timer_cost(const char* func_name, int line_num);

/**
 * @brief Get and log current system time.
 * @return AK_SUCCESS on success, AK_FAILED otherwise.
 */
int get_system_time(void);

/**
 * @brief Set system date and time.
 * @param datetime Pointer to datetime structure containing year, month, day, hour, minute, second.
 * @return AK_SUCCESS on success, AK_FAILED otherwise.
 */
int set_system_time(ak_glass_datetime_t *datetime);

/**
 * @brief Set system timezone (in minutes offset from UTC).
 * @param timezone Pointer to timezone structure containing the minute offset.
 * @return AK_SUCCESS on success, AK_FAILED otherwise.
 */
int set_system_timezone(ak_glass_timezone_t *timezone);

/**
 * @brief Get the system runtime in milliseconds since boot.
 * @return Current system uptime in milliseconds as a 64-bit integer.
 */
long long get_sys_run_time64();


#ifdef __cplusplus
}
#endif

#endif // _AI_GLASS_TIME_H__
