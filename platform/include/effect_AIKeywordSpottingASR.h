/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd. */
#ifndef _EFFECT_AI_KEYWORD_SPOTTING_ASR_H
#define _EFFECT_AI_KEYWORD_SPOTTING_ASR_H

#include "ak_types.h"

// get version string
const AK_CHAR *Effect_AIKeywordSpottingASR_GetVersionInfo(AK_VOID);

// followings are to be set into AUDIO_EFFECT_REG_S

// enEffectType shall set to AK_AUDIO_AI_KEYWORD_SPOTTING

AK_VOID *Effect_AIKeywordSpottingASR_Open(AK_VOID *open_arg, AK_VOID *pstEffectAttr, AK_VOID *pParamFactory);
AK_S32   Effect_AIKeywordSpottingASR_Close(AK_VOID *Effect_handle);
AK_S32   Effect_AIKeywordSpottingASR_Process(AK_VOID *Effect_handle, AK_VOID *proc_arg);
AK_S32   Effect_AIKeywordSpottingASR_SetAttr(AK_VOID *Effect_handle, AK_VOID *pstEffectAttr, AK_VOID *pParamFactory);

#endif
