/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd. */
#ifndef _AK_UUID_H_
#define _AK_UUID_H_

#include "ak_common.h"

typedef enum akUUID_ERROR_TYPE_E
{
    // Invalid argument
    ERROR_UUID_INVALID_ARG  = (MODULE_ID_UUID << 24) + 0,
    // Global ID is zero, cannot read
    ERROR_UUID_GLOBAL_ID_ZERO,
    // Custom ID already has data, cannot write
    ERROR_UUID_UNWRITABLE,
    // Other error (e.g. data mismatch)
    ERROR_UUID_OTHERS ,
    // UUID device not opened
    ERROR_UUID_DEV_NOT_OPENED,
    // Open failed, device does not exist
    ERROR_UUID_DEV_NOT_EXISTED,
    // Device already opened
    ERROR_UUID_DEV_ALREADY_OPENED,
    // Unknown error
    ERROR_UUID_UNKNOW
}UUID_ERROR_TYPE_E;

/**
 * AK_BASE_UuidOpen - open uuid
 * return: return 0 on success  or -1 on failed
 * notes:
 */
AK_S32 AK_BASE_UuidOpen(AK_VOID);

/**
 * AK_BASE_UuidClose - close uuid
 * return: return 0 on success  or -1 on failed
 * notes:
 */
AK_S32 AK_BASE_UuidClose(AK_VOID);

/**
 * AK_BASE_UuidGetGlobalId   - get global_id
 * @pathname[OUT]: buf
 * return: return 0 on success   or -1 on failed
 * notes: fore read from hardware
 */
AK_S32 AK_BASE_UuidGetGlobalId(AK_U8** ppu8Buf);

/*
* AK_BASE_UuidGetGlobalIdLength   - get global_id length
* @pathname[OUT]: length
* return: return 0 on success   or -1 on failed
* notes:
*/
AK_S32 AK_BASE_UuidGetGlobalIdLength(AK_S32* ps32Length);

/*
* AK_BASE_UuidGetVersion   - get uuid version
* return: return version
* notes:
*/
const AK_CHAR* AK_BASE_UuidGetVersion(AK_VOID);

#endif

/* end of file */
