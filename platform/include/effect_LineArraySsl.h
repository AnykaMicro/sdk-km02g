/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd. */
#ifndef __EFFECT_LINEARRAY_SSL_H__
#define __EFFECT_LINEARRAY_SSL_H__

#include "ak_types.h"

// get version string
const AK_CHAR *Effect_LineArray_SSL_GetVersionInfo(AK_VOID);

// followings are to be set into AUDIO_EFFECT_REG_S

// enEffectType shall set to AK_AUDIO_LINEARRAY_SSL

AK_VOID *Effect_LineArraySSL_Open(AK_VOID *open_arg, AK_VOID *pstEffectAttr, AK_VOID *pParamFactory);
AK_S32   Effect_LineArraySSL_Close(AK_VOID *Effect_handle);
AK_S32   Effect_LineArraySSL_Process(AK_VOID *Effect_handle, AK_VOID *proc_arg);
AK_S32   Effect_LineArraySSL_SetAttr(AK_VOID *Effect_handle, AK_VOID *pstEffectAttr, AK_VOID *pParamFactory);

#endif
