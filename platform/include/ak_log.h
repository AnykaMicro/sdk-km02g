/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd. */
#ifndef _AK_LOG_H_
#define _AK_LOG_H_

#include "ak_types.h"

/**
 * print functions
 * 1. error print, call this when sys-func fail or self-define error occur
 * 2. warning print, warning conditions
 * 3. notice print, normal but significant
 * 4. normal print, normal message or tips
 * 5. info print, use for cycle print like watch-dog feed print
 * 6. debug print, use for debug
 */
typedef enum akLOG_LEVEL_E {
    LOG_LEVEL_RESERVED = 0,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_NOTICE,
    LOG_LEVEL_NORMAL,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG,
}LOG_LEVEL_E;

/**
 * AK_BASE_Print: print function we defined for debugging
 * @module[IN]: module id
 * @s32Level[IN]: print s32Level [0,5]
 * @pcFmt[IN]: format like printf()
 * @...[IN]: variable arguments list
 * return: we return 0 always.
 */
AK_S32 AK_BASE_Print(AK_S32 s32ModuleId, AK_S32 s32Level, \
    const AK_CHAR *pcFmt, ...)__attribute__((format(printf,3,4)));

#define AK_BASE_PrintError(s32ModuleId, pcFmt, arg...) \
    AK_BASE_Print(s32ModuleId, LOG_LEVEL_ERROR, pcFmt, ##arg)
#define AK_BASE_PrintWarning(s32ModuleId, pcFmt, arg...) \
    AK_BASE_Print(s32ModuleId, LOG_LEVEL_WARNING, pcFmt, ##arg)
#define AK_BASE_PrintNotice(s32ModuleId, pcFmt, arg...) \
    AK_BASE_Print(s32ModuleId, LOG_LEVEL_NOTICE, pcFmt, ##arg)
#define AK_BASE_PrintNormal(s32ModuleId, pcFmt, arg...) \
    AK_BASE_Print(s32ModuleId, LOG_LEVEL_NORMAL, pcFmt, ##arg)
#define AK_BASE_PrintInfo(s32ModuleId, pcFmt, arg...) \
    AK_BASE_Print(s32ModuleId, LOG_LEVEL_INFO, pcFmt, ##arg)
#define AK_BASE_PrintDebug(s32ModuleId, pcFmt, arg...) \
    AK_BASE_Print(s32ModuleId, LOG_LEVEL_DEBUG, pcFmt, ##arg)

#define AK_BASE_PrintErrorEx(s32ModuleId, pcFmt, arg...) \
    AK_BASE_Print(s32ModuleId, LOG_LEVEL_ERROR, "[%s:%d] " pcFmt, \
        (AK_CHAR *)__func__, __LINE__, ##arg)
#define AK_BASE_PrintWarningEx(s32ModuleId, pcFmt, arg...) \
    AK_BASE_Print(s32ModuleId, LOG_LEVEL_WARNING, "[%s:%d] " pcFmt, \
        (AK_CHAR *)__func__, __LINE__, ##arg)
#define AK_BASE_PrintNoticeEx(s32ModuleId, pcFmt, arg...) \
    AK_BASE_Print(s32ModuleId, LOG_LEVEL_NOTICE, "[%s:%d] " pcFmt, \
        (AK_CHAR *)__func__, __LINE__, ##arg)
#define AK_BASE_PrintNormalEx(s32ModuleId, pcFmt, arg...) \
    AK_BASE_Print(s32ModuleId, LOG_LEVEL_NORMAL, "[%s:%d] " pcFmt, \
        (AK_CHAR *)__func__, __LINE__, ##arg)
#define AK_BASE_PrintInfoEx(s32ModuleId, pcFmt, arg...) \
    AK_BASE_Print(s32ModuleId, LOG_LEVEL_INFO, "[%s:%d] " pcFmt, \
        (AK_CHAR *)__func__, __LINE__, ##arg)
#define AK_BASE_PrintDebugEx(s32ModuleId, pcFmt, arg...) \
    AK_BASE_Print(s32ModuleId, LOG_LEVEL_DEBUG,  "[%s:%d] " pcFmt, \
        (AK_CHAR *)__func__, __LINE__, ##arg)

/**
 * AK_BASE_PrintGetVersion - get log version
 * return: version string
 * notes:
 */
const AK_CHAR* AK_BASE_PrintGetVersion(AK_VOID);

/**
 * AK_BASE_PrintSetLevel - set current print s32Level
 * @s32ModuleId[IN]: module id
 * @s32Level[IN]: current s32Level
 * return: old s32Level
 */
AK_S32 AK_BASE_PrintSetLevel(AK_S32 s32ModuleId, AK_S32 s32Level);

/**
 * AK_BASE_PrintSetSyslogLevel - set print log s32Level
 * @module[IN] : module id
 * @s32Level[IN]: s32Level to set, value in enum LOG_LEVEL_E
 * return: always return 0
 */
AK_S32 AK_BASE_PrintSetSyslogLevel(AK_S32 s32ModuleId, AK_S32 s32Level);

#endif

/* end of file */
