/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd. */
/**
 * @file ai_glass_time.c
 * @brief Timer utility functions for measuring elapsed time.
 *
 * This file provides simple timer functions to measure elapsed time in milliseconds
 * using the monotonic clock (CLOCK_MONOTONIC). It includes functions to start and stop
 * a timer, and calculate the time difference between them.
 *
 * @date 2025-09-25
 * @version 1.0
 */
#include "fun_log.h"
#include "ai_glass_time.h"

ai_glass_timer_t timer_cost;

void ai_glass_timer_start(ai_glass_timer_t *timer)
{
    clock_gettime(CLOCK_MONOTONIC, &timer->start);
}

double ai_glass_timer_stop(ai_glass_timer_t *timer)
{
    clock_gettime(CLOCK_MONOTONIC, &timer->end);

    // Calculate time difference (seconds + nanoseconds)
    double elapsed = (timer->end.tv_sec - timer->start.tv_sec) * 1000.0;  // Convert to milliseconds
    elapsed += (timer->end.tv_nsec - timer->start.tv_nsec) / 1000000.0;  // Nanoseconds to milliseconds

    return elapsed;
}

void init_global_time(void)
{
    ai_glass_timer_start(&timer_cost);
}

void print_timer_cost(const char* func_name, int line_num)
{
    double cost = ai_glass_timer_stop(&timer_cost);
    log_info_app("%s,line=%d,cost time_take_cost %.3f ms\n", func_name, line_num, cost);
}


int get_system_time(void)
{
    int ret = 0;
    DATE_S stDate = {0};

    ret = AK_BASE_GetLocalDate(&stDate);
    if (ret == AK_SUCCESS)
    {
        log_info_app("get_system_time:timezone:%d time= %u/%u/%u %u:%u:%u\n",stDate.s32Timezone,stDate.s32Year,stDate.s32Month,stDate.s32Day,stDate.s32Hour,stDate.s32Minute,stDate.s32Second);
    }else
    {
        return AK_FAILED;
    }
    return AK_SUCCESS;
}

int set_system_time(ak_glass_datetime_t *datetime)
{
    // Parameter checking
    if (datetime == NULL)
    {
        log_error_app("Error: datetime parameter is NULL\n");
        return AK_FAILED;
    }

    // Validate the validity of date and time parameters
    if (datetime->year < 1970 || datetime->year > 2100 ||
        datetime->month < 1 || datetime->month > 12 ||
        datetime->day < 1 || datetime->day > 31 ||
        datetime->hour > 23 ||
        datetime->min > 59 ||
        datetime->sec > 59)
    {
        log_error_app("Error: Invalid datetime parameters\n");
        return AK_FAILED;
    }

    int ret = 0;
    DATE_S stDate = {0};

    stDate.s32Year = datetime->year;
    stDate.s32Month = datetime->month;
    stDate.s32Day = datetime->day;
    stDate.s32Hour = datetime->hour;
    stDate.s32Minute = datetime->min;
    stDate.s32Second = datetime->sec;

    // Use AK_BASE_SetSystemTime to set time only, without modifying timezone
    ret = AK_BASE_SetSystemTime(&stDate);
    if (ret != AK_SUCCESS){
        log_error_app("AK_BASE_SetSystemTime Failed! ret:%d\n", ret);
        return AK_FAILED;
    }

    return AK_SUCCESS;
}

int set_system_timezone(ak_glass_timezone_t *timezone)
{
    // Parameter checking
    if (timezone == NULL)
    {
        log_error_app("Error: timezone parameter is NULL\n");
        return AK_FAILED;
    }

    // Validate the validity of timezone parameters (minutes range: -720 to 720, i.e., -12 to +12 hours)
    if (timezone->minutes > 720 || timezone->minutes < -720)
    {
        log_error_app("Error: Invalid timezone parameters\n");
        return AK_FAILED;
    }

    int ret = 0;
    AK_S32 s32Timezone = timezone->minutes / 60;

    // Use AK_BASE_SetTimezone to set timezone only, without modifying system time
    ret = AK_BASE_SetTimezone(s32Timezone);
    if (ret != AK_SUCCESS)
    {
        log_error_app("AK_BASE_SetTimezone Failed! ret:%d\n", ret);
        return AK_FAILED;
    }

    return AK_SUCCESS;
}

long long get_sys_run_time64()
{
	struct timespec sttimespec;
	clock_gettime(CLOCK_MONOTONIC,&sttimespec);
	long long llt = (long long)(sttimespec.tv_sec) * 1000 + (long long)sttimespec.tv_nsec/1000000;

	return llt;
}
