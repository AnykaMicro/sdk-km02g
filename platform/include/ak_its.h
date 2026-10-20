/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd. */
#ifndef _AK_ITS_H_
#define _AK_ITS_H_

#include "ak_types.h"

/**
 * AK_EXT_ITS_GetVersion - get ITS version
 * return: version string
 * notes:
 */
const AK_CHAR* AK_EXT_ITS_GetVersion(AK_VOID);

/**
 * AK_EXT_ITS_Start - start anyka ITS
 * @u32Port[IN]:port
 * notes: If you wan't use ISP tool to debug, do not start this server.
 */
AK_VOID AK_EXT_ITS_Start(AK_U32 u32Port);

/**
 * ak_its_stop - stop anyka ISP tool server
 * notes:
 */
AK_VOID AK_EXT_ITS_Stop(AK_VOID);


#endif
/* end of file*/
