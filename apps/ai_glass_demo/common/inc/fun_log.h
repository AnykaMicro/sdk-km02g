/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd. */
#ifndef _FUN_LOG_H_
#define _FUN_LOG_H_

/**
 * @brief Log levels for application logging.
 */
enum LOG_APP_LEVEL {
	LOG_APP_FATAL = -1, /**< Fatal error */
	LOG_APP_ERROR,      /**< Error condition */
	LOG_APP_WARNING,    /**< Warning condition */
	LOG_APP_INFO,       /**< Informational message */
	LOG_APP_DEBUG,      /**< Debug message */
};

/**
 * @brief Log filter information structure.
 */
typedef struct
{
	int Level;          /**< Log level */
	char File[128];     /**< Filename */
	char Function[64];  /**< Function name */
	char KeyWord[64];   /**< Keyword filter */

	int StartLine;      /**< Start line number */
	int EndLine;        /**< End line number */

	long long StartT;   /**< Start timestamp */
	long long EndT;     /**< End timestamp */
} log_app_filter_info;

/**
 * @brief Callback function type for log printing.
 *
 * @param level Log level.
 * @param str_t Timestamp string.
 * @param lTimeUS Time in microseconds.
 * @param log Log message.
 * @param iLogLen Length of the log message.
 * @return 0 on success.
 */
typedef int (*LOG_PRINT_CB)(int level, char *str_t, long lTimeUS, char* log, int iLogLen);

/**
 * @brief Print application log.
 *
 * @param level Log level.
 * @param file Source file name.
 * @param fun Function name.
 * @param line Line number.
 * @param fmt Format string (printf style).
 * @param ... Variable arguments.
 * @return 0 on success.
 */
int log_print_app(int level, const char *file, const char *fun, int line, const char *fmt, ...)__attribute__((format(printf, 5, 6)));

/**
 * @brief Set log filter.
 *
 * @param pLogFilterInfo Pointer to log filter information structure.
 * @return 0 on success.
 */
int log_print_app_set_filter(log_app_filter_info *pLogFilterInfo);

/**
 * @brief Set log callback function.
 *
 * @param cb Callback function.
 * @return 0 on success.
 */
int log_print_app_set_cb(LOG_PRINT_CB cb);

/**
 * @brief Set current log level.
 *
 * @param level Log level to set.
 * @return 0 on success.
 */
int log_print_app_set_level(int level);


#if 1

#define log_fatal_app(fmt, arg...) \
			log_print_app( LOG_APP_FATAL,__FILE__, __FUNCTION__, __LINE__,fmt"\n",##arg)
#define log_error_app(fmt, arg...) \
			log_print_app(LOG_APP_ERROR, __FILE__,__FUNCTION__, __LINE__,fmt"\n", ##arg)
#define log_warn_app(fmt, arg...) \
			log_print_app( LOG_APP_WARNING,__FILE__, __FUNCTION__, __LINE__,fmt"\n", ##arg)
#define log_info_app(fmt, arg...) \
			log_print_app(LOG_APP_INFO,__FILE__, __FUNCTION__, __LINE__,fmt"\n",##arg)
#define log_debug_app(fmt, arg...) \
			log_print_app( LOG_APP_DEBUG,__FILE__, __FUNCTION__, __LINE__, fmt"\n",##arg)

#else
#define log_error_app
#define log_warn_app
#define log_info_app
#define log_debug_app

#endif

#endif

/* end of file */
