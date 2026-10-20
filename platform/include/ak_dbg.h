/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd. */
#ifndef _AK_DBG_H_
#define _AK_DBG_H_

#include "ak_types.h"

#define DBG_SUPPORT

typedef enum akDBG_TYPE_E{
    DBG_TYPE_PARAM,
    DBG_TYPE_STAT,
}DBG_TYPE_E;

typedef enum akDBG_ACT_E{
    DBG_ACT_ADD_STR ,
    DBG_ACT_ADD_INT ,
    DBG_ACT_ADD_DOUBLE ,
    DBG_ACT_ADD_OBJ ,
    DBG_ACT_DROP ,
    DBG_ACT_READ ,
}DBG_ACT_E;

/**
 * AK_BASE_DebugInfo - write debug information such as "title=string"
 * @enModuleIdParam[IN]: module id
 * @enDbgTypeParam[IN]:  debug type
 * @str[IN]: such as "title=string"
 * return: 0 success, other failed
 * notes:
 */
AK_S32 AK_BASE_DebugInfo( MODULE_ID_E enModuleIdParam, \
    DBG_TYPE_E enDbgTypeParam, AK_CHAR *pcData );

/**
 * AK_BASE_DbgGetVersion: Get dbg module version info
 * return: dbg module version info
 */
const AK_CHAR* AK_BASE_DbgGetVersion(AK_VOID);

/**
 * AK_BASE_SetDebugStrInfo - write debug information such as "title=string"
 * @enModuleIdParam[IN]: module id
 * @enDbgTypeParam[IN]:  debug type
 * @pcTitle[IN]:  debug information title
 * @pcStr[IN]:  debug information string
 * return: 0 success, other failed
 * notes:
 */
AK_S32 AK_BASE_SetDebugStrInfo(MODULE_ID_E enModuleIdParam, \
    DBG_TYPE_E enDbgTypeParam, AK_CHAR *pcTitle, AK_CHAR *pcStr);

/**
 * AK_BASE_SetDebugNumInfo - write debug information such as "title=number"
 * @enModuleIdParam[IN]: module id
 * @enDbgTypeParam[IN]:  debug type
 * @pcTitle[IN]:  debug information title
 * @s32Num[IN]:  debug information number
 * return: 0 success, other failed
 * notes:
 */
AK_S32 AK_BASE_SetDebugNumInfo(MODULE_ID_E enModuleIdParam, \
    DBG_TYPE_E enDbgTypeParam, AK_CHAR *pcTitle, AK_S32 s32Num);

/**
 * AK_BASE_DebugJson - multipurpose interface with extensible parameters of dbg module.
 * @enModuleIdParam[IN]: module id
 * @enDbgTypeParam[IN]:  debug type
 * @action[IN]: enum dbg_action type
 * @other parameters [IN|OUT]
 * return: 0 success, other failed
 * notes:
 */
AK_S32 AK_BASE_DebugJson( MODULE_ID_E enModuleIdParam, \
    DBG_TYPE_E enDbgTypeParam, ... );

#endif

/* end of file */
