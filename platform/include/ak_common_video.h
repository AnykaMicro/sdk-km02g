/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd. */
#ifndef _AK_COMMON_VIDEO_H_
#define _AK_COMMON_VIDEO_H_

#include "ak_types.h"

/* video encode frame define */
typedef enum akVIDEO_FRAME_TYPE_E {
    FRAME_TYPE_P,
    FRAME_TYPE_I,
    FRAME_TYPE_PI,
}VIDEO_FRAME_TYPE_E;


typedef struct akVIDEO_STREAM_S {
    AK_U8 *pu8Data;	//stream data
    AK_U32 u32Len; 		//stream len in bytes
    AK_U64 u64Ts;	//timestamp(ms)
    AK_U64 u64SeqNo;	//current stream sequence no according to frame
    VIDEO_FRAME_TYPE_E enFrameType;	//I or P frame
}VIDEO_STREAM_S;

typedef struct akRES_S{
    AK_S32 u32Width;
    AK_S32 u32Height;
}RES_S;

typedef struct akCROP_INFO_S
{
    AK_S32 u32PointX;
    AK_S32 u32PointY;
    AK_S32 u32Width;
    AK_S32 u32Height;
}CROP_INFO_S;

typedef enum akPIXEL_FORMAT_E {
    PIXEL_FORMAT_YUV420SP,    //NV12
    PIXEL_FORMAT_YUV420P,     //I420
    PIXEL_FORMAT_RGB,
    PIXEL_FORMAT_RAW,
    PIXEL_FORMAT_RGB_LINEINTL, //RGB line-interlace format
    PIXEL_FORMAT_RGB_HWC,
    PIXEL_FORMAT_RGB_CHW,
    PIXEL_FORMAT_TOTAL
}PIXEL_FORMAT_E;

#endif

/* end of file */
