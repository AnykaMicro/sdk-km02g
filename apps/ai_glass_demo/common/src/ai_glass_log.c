/* SPDX-License-Identifier: Apache-2.0 */
/**
 * @file ai_glass_log.c
 * @brief Logging utility implementation for AI Glass application.
 *
 * This file implements a logging framework with multiple severity levels:
 * - INFO: General information messages
 * - DEBUG: Debugging information
 * - ERROR: Error messages
 *
 * The implementation uses a function pointer to route output to the desired
 * output function (defaulting to printf). It includes timestamp formatting
 * and automatic log level prefixing. The log level can be controlled at compile
 * time via the LOG_LEVEL macro.
 *
 * @date 2025-09-25
 * @version 1.0
 * @copyright Copyright (c) 2025 Guangzhou Anyka Microelectronics Co., Ltd.
 */

#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#include "ai_glass_log.h"

// Global output function pointer
static log_output_func_t g_log_output_func = NULL;
// Global current log level (default to INFO)
static int g_current_log_level = LOG_LEVEL_DEBUG;


// Default output function (printf)
static int default_log_output(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int result = vprintf(format, args);
    va_end(args);
    return result;
}

void ai_glass_log_init(void)
{
    if (g_log_output_func == NULL) {
        g_log_output_func = default_log_output;
    }
}

void ai_glass_log_set_output(log_output_func_t func)
{
    g_log_output_func = func;
}


void ai_glass_log_set_level(int level)
{
    g_current_log_level = level;
}


int ai_glass_log_get_level(void)
{
    return g_current_log_level;
}

static char* get_timestamp(char *buffer, size_t size)
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);

    // Format: [HH:MM:SS.mmm]
    snprintf(buffer, size, "[%02d:%02d:%02d.%03ld]",
             ts.tv_sec / 3600 % 24,
             ts.tv_sec / 60 % 60,
             ts.tv_sec % 60,
             ts.tv_nsec / 1000000);

    return buffer;
}

static void log_message(int level, const char *format, va_list args)
{
    // Check if the log level is enabled
    if (level < g_current_log_level) {
        return; // Current level is lower than set level, do not print
    }

    // Get timestamp
    char timestamp[20];
    get_timestamp(timestamp, sizeof(timestamp));

    // Format the log level prefix
    const char *level_str;
    switch (level) {
        case LOG_LEVEL_INFO:
            level_str = "INFO";
            break;
        case LOG_LEVEL_DEBUG:
            level_str = "DEBUG";
            break;
        case LOG_LEVEL_ERROR:
            level_str = "ERROR";
            break;
        default:
            level_str = "UNKNOWN";
            break;
    }

    // Create the full log message
    char log_buffer[512];
    int len = snprintf(log_buffer, sizeof(log_buffer), "%s [%s] ", timestamp, level_str);

    // Append the actual message
    vsnprintf(log_buffer + len, sizeof(log_buffer) - len, format, args);

    // Output the log message
    if (g_log_output_func) {
        g_log_output_func("%s", log_buffer);
    }
}


void ai_glass_log_info(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    log_message(LOG_LEVEL_INFO, format, args);
    va_end(args);
}


void ai_glass_log_debug(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    log_message(LOG_LEVEL_DEBUG, format, args);
    va_end(args);
}

void ai_glass_log_error(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    log_message(LOG_LEVEL_ERROR, format, args);
    va_end(args);
}
