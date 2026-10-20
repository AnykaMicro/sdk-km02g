/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd. */
#ifndef _AK_ATS_H_
#define _AK_ATS_H_

/**
 *@brief AK_EXT_ATS_GetVersion - Get audio tool version
 *@return version string
 *@notes:
 */
const AK_CHAR* AK_EXT_ATS_GetVersion(AK_VOID);

/**
 *@brief AK_EXT_ATS_Start - Audio tool server start
 *@param port[IN]: Server port id
 *@return 0 success otherwise failed
 *@notes:
 */
AK_S32 AK_EXT_ATS_Start(AK_U32 u32Port);

/**
 *@brief AK_EXT_ATS_Stop - Audio tool server stop
 *@return 0 success otherwise failed
 *@notes:
 */
AK_S32 AK_EXT_ATS_Stop(AK_VOID);


#endif
