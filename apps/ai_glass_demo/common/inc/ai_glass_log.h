/* SPDX-License-Identifier: Apache-2.0 */
/**
 * @file ai_glass_log.h
 * @brief Logging utility header file for AI Glass application.
 *
 * This header file defines a logging framework with multiple severity levels:
 * - INFO: General information messages
 * - DEBUG: Debugging information
 * - ERROR: Error messages
 *
 * The logging functions are designed to be used as replacements for printf,
 * with automatic timestamp and log level prefixing. The actual output is routed
 * through a configurable function pointer (ak_ai_glass_printf).
 *
 * @date 2025-09-25
 * @version 1.0
 * @copyright Copyright (c) 2025 Guangzhou Anyka Microelectronics Co., Ltd.
 */

#ifndef _AI_GLASS_LOG_H__
#define _AI_GLASS_LOG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdarg.h>

// Log level definitions
#define LOG_LEVEL_INFO    0  ///< Informational messages
#define LOG_LEVEL_DEBUG   1  ///< Debugging messages
#define LOG_LEVEL_ERROR   2  ///< Error messages


// Function pointer type for output
typedef int (*log_output_func_t)(const char *format, ...);

/**
 * @brief Set the output function for logging.
 *
 * @param func Pointer to the function that will handle log output.
 */
void ai_glass_log_set_output(log_output_func_t func);

/**
 * @brief Initialize the logging module.
 */
void ai_glass_log_init(void);

/**
 * @brief Log an informational message.
 *
 * @param format Format string (like printf)
 * @param ... Variable arguments
 */
void ai_glass_log_info(const char *format, ...);
/**
 * @brief Set the current log level.
 *
 * @param level The new log level (LOG_LEVEL_INFO, LOG_LEVEL_DEBUG, LOG_LEVEL_ERROR)
 */
void ai_glass_log_set_level(int level);

/**
 * @brief Get the current log level.
 *
 * @return Current log level
 */
int ai_glass_log_get_level(void);

/**
 * @brief Log a debug message.
 *
 * @param format Format string (like printf)
 * @param ... Variable arguments
 */
void ai_glass_log_debug(const char *format, ...);

/**
 * @brief Log an error message.
 *
 * @param format Format string (like printf)
 * @param ... Variable arguments
 */
void ai_glass_log_error(const char *format, ...);



#ifdef __cplusplus
}
#endif

#endif // _AI_GLASS_LOG_H__
