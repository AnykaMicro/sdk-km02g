/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd. */
#ifndef _EFFECT_RES_H_
#define _EFFECT_RES_H_

#include "ak_types.h"

// get version string
const AK_CHAR *Effect_RES_GetVersionInfo(AK_VOID);

// followings are to be set into AUDIO_EFFECT_REG_S

// enEffectType shall set to AK_AUDIO_RESAMPLE

AK_VOID *Effect_RES_Open(AK_VOID *open_arg, AK_VOID *pstEffectAttr, AK_VOID *pParamFactory);
AK_S32   Effect_RES_Close(AK_VOID *Effect_handle);
AK_S32   Effect_RES_Process(AK_VOID *Effect_handle, AK_VOID *proc_arg);
AK_S32   Effect_RES_SetAttr(AK_VOID *Effect_handle, AK_VOID *pstEffectAttr, AK_VOID *pParamFactory);

#endif
