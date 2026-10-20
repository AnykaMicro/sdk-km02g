/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd. */
#ifndef _AK_VIDEO_H
#define _AK_VIDEO_H

#include "ak_common.h"
#include "ak_venc.h"

typedef enum akEXT_VIDEO_ERROR_TYPE_E {
    ERROR_EXT_VIDEO_WRONG_SRC_MODULE = (MODULE_ID_APP_VIDEO << 24) + 0,
    ERROR_EXT_VIDEO_WRONG_MODULE_ID,
    ERROR_EXT_VIDEO_INCORRECT_BIND_RELATION,
    ERROR_EXT_VIDEO_DST_CHN_MUTLEPLE_BINDED,
    ERROR_EXT_VIDEO_DST_CHN_NOT_BINDED,
    ERROR_EXT_VIDEO_SRC_HHN_NOT_BINDED,
    ERROR_EXT_VIDEO_SRC_CHN_OUTOF_RANGE,
    ERROR_EXT_VIDEO_SRC_DST_CHN_PARAM_NOT_MATCH,
    ERROR_EXT_VIDEO_GENERATE_GOBJ_FAILED,
    ERROR_EXT_VIDEO_GENERATE_OBJ_FAILED,
    ERROR_EXT_VIDEO_ENCODRE_HAS_NO_DATA,
    ERROR_EXT_VIDEO_FRAME_COUNT_OUTOF_LIMITATION,
    ERROR_EXT_VIDEO_DST_CHN_NOT_DISABLED,
} EXT_VIDEO_ERROR_TYPE_E;

typedef enum akBIND_MODE_E
{
    USER_MODE = 0x0,
    AUTO_MODE,
} BIND_MODE_E;

typedef struct akEXT_CHN_S
{
    MODULE_ID_E         enModId;            /* module id  */
    AK_S32              s32DevId;
    /* object id, chn id for vpss, encoder id for venc,
    decoder id for vdec*/
    AK_S32              s32ChnId;
} EXT_CHN_S;

typedef struct akEXT_BIND_PARAM_S
{
    AK_S32                      s32BPFrameRate;             /* frame rate control */
    AK_S32                      s32BPFrameDepth;            /* frame list max length*/
    BIND_MODE_E                 s32BPMode;                   /* bind mode */
} EXT_BIND_PARAM_S;

/* define the callback to handle the attr chn data */
typedef AK_VOID (*EXT_VIDEO_DATA_CB_S)(EXT_CHN_S *pstChn, AK_VOID *pvInput, AK_VOID *pvOutput);

/**
 * AK_EXT_VIDEO_GetVersion - get version
 * return: version string
 */
const AK_CHAR* AK_EXT_VIDEO_GetVersion(AK_VOID);

/* AK_EXT_VIDEO_SetDstChnActive - set stat to dst chn
 * @Chn[IN]         :   dst info of bind obj
 * @s32Enable[IN]      :   activate status of Chn
 * return AK_SUCCESS is successful,  error code otherwise.
 * */
AK_S32 AK_EXT_VIDEO_SetDstChnActive(const EXT_CHN_S *pstChn, AK_S32 s32Enable);

/* AK_EXT_VIDEO_VencGetStream --  get the venc stream from video lib
 * @Chn[IN]     :   dst info of bind obj
 * @stream[OUT] :   output the stream
 * return AK_SUCCESS is successful,  error code otherwise.
 */
AK_S32 AK_EXT_VIDEO_VencGetStream(const EXT_CHN_S *pstChn, VIDEO_STREAM_S *pstVideoStream);

/* AK_EXT_VIDEO_UnbindChn  --  chn bind function for video
 * @pstSrcChn[IN]   :   src info of bind obj
 * @pstDstChn[IN]  :   dest info of bind obj
 * return AK_SUCCESS is successful,  error code otherwise.
 */
AK_S32 AK_EXT_VIDEO_UnbindChn(const EXT_CHN_S *pstSrcChn,
                                const EXT_CHN_S *pstDstChn);

/* AK_EXT_VIDEO_BindChn    --  chn bind function for video
 * @pstSrcChn[IN]   :   src info of bind obj
 * @pstDstChn[IN]  :   dest info of bind obj
 * @param[IN]       :   param of bind obj
 * return AK_SUCCESS is successful,  error code otherwise.
 */
AK_S32 AK_EXT_VIDEO_BindChn(const EXT_CHN_S *pstSrcChn,
                const EXT_CHN_S *pstDstChn, const EXT_BIND_PARAM_S *pstBparam);

/* AK_EXT_VIDEO_SetCb  --  set the callback to handle the chn video data
 * @Chn[IN]     :   dest src obj
 * @fn_cb[IN]   :   func to handle the data
 * @output[IN]      :   param of output for use
 * return AK_SUCCESS is successful,  error code otherwise.
 */
AK_S32 AK_EXT_VIDEO_SetCb(const EXT_CHN_S *pstChn, EXT_VIDEO_DATA_CB_S funcVideoDataCb, AK_VOID *pvOutput);

/* AK_EXT_VIDEO_UnsetCb    --  unset the callback func
 * @Chn[IN]     :   dest src obj
 * return AK_SUCCESS is successful,  error code otherwise.
 */
AK_S32 AK_EXT_VIDEO_UnsetCb(const EXT_CHN_S *pstChn);

/* AK_EXT_VIDEO_SetChnAttr    --  set the dst chn attr
 * @pstDstChn[IN]      :   dest chn
 * @param[IN]           :   dest param to set
 * return AK_SUCCESS is successful,  error code otherwise.
 * NOTE : This api is only to modify the frame rate output from vi.
 *        for the venc wanted, call the  corresponding venc interface
 */
AK_S32 AK_EXT_VIDEO_SetChnAttr(const EXT_CHN_S *pstDstChn,
                               const EXT_BIND_PARAM_S *pstBparam);

#endif

/* end of file */
