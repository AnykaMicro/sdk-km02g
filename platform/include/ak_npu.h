/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd. */
#ifndef _AK_NPU_H_
#define _AK_NPU_H_

#include "ak_common.h"

typedef enum akNPU_ERROR_TYPE_E {
    ERROR_NPU_ALREADY_INIT = ERROR_TYPE_NO_LEFT_SPACE + 2,
    ERROR_NPU_ALREADY_DEINIT,
}NPU_ERROR_TYPE_E;

/**
 * AK_MPI_NPU_GetVersion: Get npu module version info
 * return: npu module version info
 */
const AK_CHAR* AK_MPI_NPU_GetVersion(AK_VOID);

/**
 * AK_MPI_NPU_Init - Init npu module
 * return: 0 success, other failed
 * notes:
 */
AK_S32 AK_MPI_NPU_Init(AK_VOID);

/**
 * AK_MPI_NPU_Deinit - Deinit npu module
 * return: 0 success, other failed
 * notes:
 */
AK_S32 AK_MPI_NPU_Deinit(AK_VOID);

/* AK_MPI_NPU_ResetHw	-	reset SVP HW
 * return AK_SUCCESS is successful, error code otherwise.
 */
AK_S32 AK_MPI_NPU_ResetHw(AK_VOID);

#endif
