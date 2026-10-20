/* SPDX-License-Identifier: Apache-2.0 */
/**
 * @file print_color_simple.h
 * @brief Simple Color Printing Header File
 *
 * This file defines macros and enumerations for printing colored text
 * to the console, including foreground, background, and text styles.
 *
 * @copyright Copyright (c) 2025 Guangzhou Anyka Microelectronics Co., Ltd.
 */

#ifndef _PRINT_COLOR_SIMPLE_H_
#define _PRINT_COLOR_SIMPLE_H_

/** @brief Reset color code */
#define COLOR_END                "\033[0m"

/**
 * @brief Color Mode Enumeration
 * Defines text styles such as bold, underline, blink, etc.
 */
enum COLOR_MODE {                                                               /**< Color Mode */
    CM_NORMAL     = 0,                                                          /**< Default Mode */
    CM_BOLD       = 1,                                                          /**< Highlight/Bold */
    CM_UNDERLINED = 4,                                                          /**< Underline */
    CM_BLINK      = 5,                                                          /**< Blink */
    CM_NEGATIVE   = 7,                                                          /**< Inverse/Negative Color */
};

/**
 * @brief Background Color Enumeration
 * Defines background colors for console output.
 */
enum COLOR_BACKGROUND {                                                         /**< Background Color */
    CB_NONE   = 0,                                                              /**< Custom use: no background color */
    CB_BLACK  = 40,                                                             /**< Black background */
    CB_RED    = 41,                                                             /**< Red background */
    CB_GREEN  = 42,                                                             /**< Green background */
    CB_YELLOW = 43,                                                             /**< Yellow background */
    CB_BLUE   = 44,                                                             /**< Blue background */
    CB_PURPLE = 45,                                                             /**< Purple background */
    CB_CYAN   = 46,                                                             /**< Cyan background */
    CB_WHITE  = 47,                                                             /**< White background */
};

/**
 * @brief Font Color Enumeration
 * Defines foreground (text) colors for console output.
 */
enum COLOR_FONT {                                                               /**< Font Color */
    CF_BLACK  = 30,                                                             /**< Black text */
    CF_RED    = 31,                                                             /**< Red text */
    CF_GREEN  = 32,                                                             /**< Green text */
    CF_YELLOW = 33,                                                             /**< Yellow text */
    CF_BLUE   = 34,                                                             /**< Blue text */
    CF_PURPLE = 35,                                                             /**< Purple text */
    CF_CYAN   = 36,                                                             /**< Cyan text */
    CF_WHITE  = 37,                                                             /**< White text */
};

#ifndef BOOL_TYPE
#define BOOL_TYPE
/**
 * @brief Custom Boolean Enumeration
 */
enum BOOL_VALUE {                                                               /**< Custom BOOLEAN value */
    BOOL_FAILURE = -1,                                                          /**< Failure (-1) */
    BOOL_SUCCESS = 0 ,                                                          /**< Success (0) */
    BOOL_FALSE = 0,                                                             /**< False (0) */
    BOOL_TRUE  = 1,                                                             /**< True (1) */
};

/**
 * @brief Alternative Boolean Enumeration
 */
enum B_VALUE {                                                                  /**< Custom BOOLEAN value */
    B_FAILURE = -1,                                                             /**< Failure (-1) */
    B_SUCCESS = 0 ,                                                             /**< Success (0) */
    B_FALSE = 0,                                                                /**< False (0) */
    B_TRUE  = 1,                                                                /**< True (1) */
};
#endif

/** @brief Macro to get the filename from a path */
#define __FILENAME__ ( __builtin_strrchr( __FILE__, '/' ) ? __builtin_strrchr( __FILE__, '/' ) + 1 : __FILE__ )

#ifdef _FUNC_GETTID_
/**
 * @brief Debug Print Macro with Thread ID
 *
 * Prints formatted output with color, process ID, thread ID, filename, line number, and function name.
 *
 * @param CM Color Mode (Attribute)
 * @param CB Background Color
 * @param CF Foreground Color
 * @param FMT Format string
 * @param ... Variable arguments
 */
#define PRINTD( CM, CB, CF, FMT, ... ) \
    do { \
        switch( CB ) { \
            case CB_NONE: \
                printf( "\033[%d;%dm[%d:%d]%s:%d:%s( ) " FMT COLOR_END, \
                        CM, CF, getpid(), gettid(), __FILE__, __LINE__, __func__, ##__VA_ARGS__ ); \
                        break;\
            default: \
                printf( "\033[%d;%d;%dm[%d:%d]%s:%d:%s( ) " FMT COLOR_END, \
                        CM, CB, CF, getpid(), gettid(), __FILE__, __LINE__, __func__, ##__VA_ARGS__ ); \
        } \
    } while (0);
#else
/**
 * @brief Debug Print Macro with Syscall ID
 *
 * Prints formatted output with color, process ID, syscall ID, filename, line number, and function name.
 *
 * @param CM Color Mode (Attribute)
 * @param CB Background Color
 * @param CF Foreground Color
 * @param FMT Format string
 * @param ... Variable arguments
 */
#define PRINTD( CM, CB, CF, FMT, ... ) \
    do { \
        switch( CB ) { \
            case CB_NONE: \
                printf( "\033[%d;%dm[%d:%ld]%s:%d:%s( ) " FMT COLOR_END, \
                        CM, CF, getpid(), syscall( 224 ), __FILE__, __LINE__, __func__, ##__VA_ARGS__ ); \
                        break;\
            default: \
                printf( "\033[%d;%d;%dm[%d:%ld]%s:%d:%s( ) " FMT COLOR_END, \
                        CM, CB, CF, getpid(), syscall( 224 ), __FILE__, __LINE__, __func__, ##__VA_ARGS__ ); \
        } \
    } while (0);
#endif

/**
 * @brief Simple Color Print Macro
 *
 * Prints formatted output with color, without extra metadata.
 *
 * @param CM Color Mode (Attribute)
 * @param CB Background Color
 * @param CF Foreground Color
 * @param FMT Format string
 * @param ... Variable arguments
 */
#define PRINTV( CM, CB, CF, FMT, ... ) \
    do { \
        switch( CB ) { \
            case CB_NONE: \
                printf( "\033[%d;%dm" FMT COLOR_END, CM, CF, ##__VA_ARGS__ ); \
                        break;\
            default: \
                printf( "\033[%d;%d;%dm" FMT COLOR_END, CM, CB, CF, ##__VA_ARGS__ ); \
        } \
    } while (0);

/**
 * @brief Log Print Macro
 *
 * Prints formatted output with color, filename, line number, and function name (no PIDs).
 *
 * @param CM Color Mode (Attribute)
 * @param CB Background Color
 * @param CF Foreground Color
 * @param FMT Format string
 * @param ... Variable arguments
 */
#define PRINTL( CM, CB, CF, FMT, ... ) \
    do { \
        switch( CB ) { \
            case CB_NONE: \
                printf( "\033[%d;%dm%s:%d:%s( ) " FMT COLOR_END, \
                        CM, CF, __FILE__, __LINE__, __func__, ##__VA_ARGS__ ); \
                        break;\
            default: \
                printf( "\033[%d;%d;%dm%s:%d:%s( ) " FMT COLOR_END, \
                        CM, CB, CF, __FILE__, __LINE__, __func__, ##__VA_ARGS__ ); \
        } \
    } while (0);

#ifndef FREE_POINT
#define FREE_POINT( POINT )  \
if( POINT != NULL ) {\
    free( POINT ) ;\
    POINT = NULL ;\
}
#endif

#endif
