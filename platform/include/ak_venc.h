/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd. */
#ifndef _AK_VENC_H_
#define _AK_VENC_H_
#include <stdbool.h>
#include "ak_common_video.h"
#include "ak_types.h"

/* bitrate contrl mode */
typedef enum akVENC_BITRATE_CTRL_MODE_E {
    VENC_RC_MODE_H264_CBR,
    VENC_RC_MODE_H264_VBR,
    VENC_RC_MODE_H264_CAPPED_VBR,
    VENC_RC_MODE_H264_AVBR,
    VENC_RC_MODE_H264_CONST_QP,
    VENC_RC_MODE_MJPEG,
    VENC_RC_MODE_H265_CBR,
    VENC_RC_MODE_H265_VBR,
    VENC_RC_MODE_H265_CAPPED_VBR,
    VENC_RC_MODE_H265_AVBR,
    VENC_RC_MODE_H265_CONST_QP,
    VENC_RC_MODE_TYPE_TOTAL
}VENC_BITRATE_CTRL_MODE_E;

/* output type */
typedef enum akVENC_OUTPUT_TYPE_E {
    H264_ENC_TYPE,
    MJPEG_ENC_TYPE,
    HEVC_ENC_TYPE
}VENC_OUTPUT_TYPE_E;

/* profile mode */
typedef enum akVENC_PROFILE_MODE_E {
    VENC_PROFILE_MAIN,
    VENC_PROFILE_HIGH,
    VENC_PROFILE_BASE,
    VENC_PROFILE_C_BASE,
    VENC_PROFILE_HEVC_MAIN,
    VENC_PROFILE_HEVC_MAIN_STILL,
    VENC_PROFILE_HEVC_MAIN_INTRA,
    VENC_PROFILE_JPEG
}VENC_PROFILE_MODE_E;

/* Chroma mode */
typedef enum akVENC_CHROMA_MODE_E {
    VENC_CHROMA_MONO,
    VENC_CHROMA_YUV420
}VENC_CHROMA_MODE_E;

/* GDR mode */
typedef enum akVENC_GDR_MODE_E {
    VENC_GDR_OFF,
    VENC_GDR_VERTICAL,
    VENC_GDR_HORIZONTAL
}VENC_GDR_MODE_E;

/* encode level */
typedef enum akVENC_ENCODE_LEVEL_E {
    VENC_HEVC_AVC_LEVEL_1   = 10,
    VENC_AVC_LEVEL_1_1      = 11, //only for avc
    VENC_AVC_LEVEL_1_2      = 12, //only for avc
    VENC_AVC_LEVEL_1_3      = 13, //only for avc
    VENC_HEVC_AVC_LEVEL_2   = 20,
    VENC_HEVC_AVC_LEVEL_2_1 = 21,
    VENC_AVC_LEVEL_2_2      = 22, //only for avc
    VENC_HEVC_AVC_LEVEL_3   = 30,
    VENC_HEVC_AVC_LEVEL_3_1 = 31,
    VENC_AVC_LEVEL_3_2      = 32, //only for avc
    VENC_HEVC_AVC_LEVEL_4   = 40,
    VENC_HEVC_AVC_LEVEL_4_1 = 41,
    VENC_AVC_LEVEL_4_2      = 42, //only for avc
    VENC_HEVC_AVC_LEVEL_5   = 50,
    VENC_HEVC_AVC_LEVEL_5_1 = 51,
    VENC_HEVC_AVC_LEVEL_5_2 = 52,
    VENC_HEVC_AVC_LEVEL_6   = 60,
    VENC_HEVC_AVC_LEVEL_6_1 = 61,
    VENC_HEVC_AVC_LEVEL_6_2 = 62
}VENC_ENCODE_LEVEL_E;

/* cu block size */
typedef enum akVENC_CU_BLOCK_SIZE_E {
    VENC_CUSize32x32,   /*!< CU size 32x32, just for HEVC */
    VENC_CUSize16x16,   /*!< CU size 16x16, just for HEVC */
    VENC_CUSize8x8      /*!< CU size 8x8, just for HEVC */
}VENC_CU_BLOCK_SIZE_E;

/* smart mode */
typedef enum akVENC_SMART_MODE_E {
    VENC_SMART_OFF,
    VENC_SMART_LTR,
    VENC_SMART_GOP_CHANGE,
    VENC_SMART_SKIP_REF,
    VENC_SMART_LTRC,
    VENC_SMART_SKIP_REF_LTRC
}VENC_SMART_MODE_E;

/* frame type */
typedef enum akVENC_FRAME_TYPE_E {
    VENC_FRAME_TYPE_P,
    VENC_FRAME_TYPE_I,
    VENC_FRAME_TYPE_PI,
}VENC_FRAME_TYPE_E;

/* spec param type */
typedef enum akVENC_SPEC_PARAM_TYPE_E {
    VENC_SPEC_PARAM_START_TYPE,
    VENC_MAX_REORDER_PICS_NUM,
    VENC_SPEC_PARAM_END_TYPE
}VENC_SPEC_PARAM_TYPE_E;

/* H264 attr */
typedef struct akVENC_H264_ATTR_S
{
    AK_U16	u16Framerate;
    AK_U16	u16GopLen;
    VENC_GDR_MODE_E	enGdrMode;
    VENC_PROFILE_MODE_E	enProfileMode;
    VENC_ENCODE_LEVEL_E enEncLevel;
}VENC_H264_ATTR_S;

/* H265 attr */
typedef struct akVENC_H265_ATTR_S
{
    AK_U16	u16Framerate;
    AK_U16	u16GopLen;
    VENC_GDR_MODE_E	enGdrMode;
    VENC_PROFILE_MODE_E	enProfileMode;
    VENC_ENCODE_LEVEL_E enEncLevel;
}VENC_H265_ATTR_S;

/* venc attr */
typedef struct akVENC_ATTR_S
{
    AK_U16 u16Width;
    AK_U16 u16Height;
    VENC_OUTPUT_TYPE_E enOutputType;
    AK_U32 u32MaxBufSize;
    AK_U32 u32MaxDMASize;
    VENC_CHROMA_MODE_E enChromaMode; //color formats
    AK_U32 u32Priority;
    union
    {
        VENC_H264_ATTR_S stH264Attr;
        VENC_H265_ATTR_S stH265Attr;
    };
}VENC_ATTR_S;

/* H264 cbr param */
typedef struct akVENC_RC_H264_CBR_S
{
    AK_U16 u16TargetBitrate;
}VENC_RC_H264_CBR_S;

/* H264 vbr param */
typedef struct akVENC_RC_H264_VBR_S
{
    AK_U16 u16MaxBitrate;
    AK_U16 u16Quality;
}VENC_RC_H264_VBR_S;


typedef struct akVENC_RC_H264_CAPPED_VBR_S
{
    AK_U16 u16MinQP;
    AK_U16 u16MaxQP;
    AK_U16 u16MaxBitRate;    // kbps
    AK_U16 u16ChangePos;//[0:100]
    AK_DOUBLE d64MinPSNR;
    AK_DOUBLE d64MaxPSNR;
}VENC_RC_H264_CAPPED_VBR_S;

typedef struct akVENC_RC_H265_CAPPED_VBR_S
{
    AK_U16 u16MinQP;
    AK_U16 u16MaxQP;
    AK_U16 u16MaxBitRate;    // kbps
    AK_U16 u16ChangePos;//[0:100]
    AK_DOUBLE d64MinPSNR;
    AK_DOUBLE d64MaxPSNR;
}VENC_RC_H265_CAPPED_VBR_S;

/* H264 avbr param */
typedef struct akVENC_RC_H264_AVBR_S
{
    AK_U16 u16MinQP;
    AK_U16 u16MaxQP;
    AK_U16 u16MaxBitrate;
    AK_U16 u16ChangePos;
}VENC_RC_H264_AVBR_S;

/* H264 const  qp param */
typedef struct akVENC_RC_H264_CONST_QP_S
{
    AK_U16 u16QP;
}VENC_RC_H264_CONST_QP_S;

/* MJPEG param */
typedef struct akVENC_RC_MJPEG_S
{
    AK_U16 u16Quality;
}VENC_RC_MJPEG_S;

/* H265 cbr param */
typedef struct akVENC_RC_H265_CBR_S
{
    AK_U16 u16TargetBitrate;
}VENC_RC_H265_CBR_S;

/* H265 vbr param */
typedef struct akVENC_RC_H265_VBR_S
{
    AK_U16 u16MaxBitrate;
    AK_U16 u16Quality;
}VENC_RC_H265_VBR_S;

/* H265 avbr param */
typedef struct akVENC_RC_H265_AVBR_S
{
    AK_U16 u16MinQP;
    AK_U16 u16MaxQP;
    AK_U16 u16MaxBitrate;
    AK_U16 u16ChangePos;
}VENC_RC_H265_AVBR_S;

/* H265 const qp param */
typedef struct akVENC_RC_H265_CONST_QP_S
{
    AK_U16 u16QP;
}VENC_RC_H265_CONST_QP_S;

/* smart mode param */
typedef struct akVENC_RC_AVC_HEVC_SMART_S
{
    VENC_SMART_MODE_E enSmartMode;
    AK_U16 u16SmartGopLen;
    AK_U16 u16SmartQuality;
    AK_U16 u16StaticValue;
}VENC_RC_AVC_HEVC_SMART_S;

/* venc rc attr */
typedef struct akVENC_RC_ATTRS_S
{
    VENC_BITRATE_CTRL_MODE_E enRcMode;
    union
    {
    VENC_RC_H264_CBR_S stH264Cbr;
    VENC_RC_H264_VBR_S stH264Vbr;
    VENC_RC_H264_CAPPED_VBR_S stH264CappedVbr;
    VENC_RC_H264_AVBR_S stH264Avbr;
    VENC_RC_H264_CONST_QP_S stH264ConstQp;
    VENC_RC_MJPEG_S stMjpegRc;
    VENC_RC_H265_CBR_S stH265Cbr;
    VENC_RC_H265_VBR_S stH265Vbr;
    VENC_RC_H265_CAPPED_VBR_S stH265CappedVbr;
    VENC_RC_H265_AVBR_S stH265Avbr;
    VENC_RC_H265_CONST_QP_S stH265ConstQp;
    };
}VENC_RC_ATTRS_S;

/* H264/265 encode advance param */
typedef struct akVENC_RC_AVC_HEVC_ADVANCE_S
{
    VENC_CU_BLOCK_SIZE_E enCuBlock;
    AK_U16 u16MinQP[3];
    AK_U16 u16MaxQP[3];
    AK_S16 s16IPDelta;
    AK_U32 u32MaxPictureISize;
    AK_U32 u32MaxPicturePSize;
    AK_U32 u32MaxPictureBSize;
    AK_BOOL bEnableSRD;
    AK_U8 u8SRDThreshold;
    AK_BOOL bEnableMMA;
}VENC_RC_AVC_HEVC_ADVANCE_S;

/* venc ext attr */
typedef struct akVENC_EXT_ATTRS_S
{
    VENC_RC_AVC_HEVC_ADVANCE_S stAdvance;
    VENC_RC_AVC_HEVC_SMART_S stSmart;
}VENC_EXT_ATTRS_S;

/* venc chn attr */
typedef struct akVENC_CHN_ATTRS_S
{
    VENC_ATTR_S stVencAttrs;
    VENC_RC_ATTRS_S stRcAttrs;
    VENC_EXT_ATTRS_S stExtAttrs;
}VENC_CHN_ATTRS_S;

typedef struct RECT_S {
    AK_U32 u32Width;
    AK_U32 u32Height;
    AK_U32 u32X;
    AK_U32 u32Y;
}RECT_S;

/* crop param */
typedef struct akVENC_CROP_CFG_S
{
    AK_BOOL bEnable;
    RECT_S stRect;
}VENC_CROP_CFG_S;


/* venc encode chn stat */
typedef struct akVENC_STAT_S
{
    AK_U32 u32Fps;
    AK_U32 u32GopLen;
    AK_U64 u32Kbps;
    AK_U64 u64MaxPictureSize;
}VENC_STAT_S;

/* spec param */
typedef struct akVENC_HEVC_SPEC_PARAM_S
{
    AK_U8 u8MaxNumReorderPics;
}VENC_HEVC_SPEC_PARAM_S;

/* spec param */
typedef struct akVENC_SPEC_PARAM_S
{
    VENC_SPEC_PARAM_TYPE_E enSpecType;
    union{
        VENC_HEVC_SPEC_PARAM_S stHevcSpecReorder;
    };
}VENC_SPEC_PARAM_S;

typedef struct akROI_SVP_INFO_S
{
    AK_U16 u16RoiNum;
    AK_U16 u16RoiLeft[10];
    AK_U16 u16RoiTop[10];
    AK_U32 u32RoiWidth[10];
    AK_U32 u32RoiHeigh[10];
    AK_S32 s32RoiQuality[10];
}ROI_SVP_INFO_S;

typedef enum akVENC_ERROR_TYPE_E
{
    ERROR_VENC_INIT_LIB_ERROR = (MODULE_ID_VENC << 24) + 0,
    ERROR_VENC_OPEN_LIB_ERROR,
    ERROR_VENC_OPEN_TOO_MANY,
    ERROR_VENC_INVALID_PARAM,
    ERROR_VENC_USER_NULL,
    ERROR_VENC_PARAM_CANNOT_DYNAMIC_SET,
    ERROR_VENC_ENCODE_FAILED,
    ERROR_VENC_DEV_OPEN_FAILED,
    ERROR_VENC_CREATE_CHN_FAILED,
    ERROR_VENC_DELETE_CHN_FAILED,
    ERROR_VENC_QUERY_DMA_INFO_FAILED,
    ERROR_VENC_SET_DMA_BLOCK_FAILED,
    ERROR_VENC_DEL_DMA_BLOCK_FAILED,
    ERROR_VENC_GET_STREAM_FAILED,
    ERROR_VENC_RELEASE_STREAM_FAILED,
    ERROR_VENC_CLEAN_STREAM_FAILED,
    ERROR_VENC_SET_JPEG_SLICE_FAILED,
    ERROR_VENC_DMA_NO_ENOUGH,
    ERROR_VENC_SET_ATTR_ERROR,
}VENC_ERROR_TYPE_E;

/* h.264 / h.265 encode control define */
typedef enum akBITRATE_CTRL_MODE_E {
    BR_MODE_CBR,
    BR_MODE_VBR,
    BR_MODE_CAPPED_VBR,
    BR_MODE_CONST_QP,
    BR_MODE_LOW_LATENCY,
    BR_MODE_AVBR,
    BR_MODE_TYPE_TOTAL
}BITRATE_CTRL_MODE_E;

typedef enum akPROFILE_MODE_E {
    PROFILE_MAIN,
    PROFILE_HIGH,
    PROFILE_BASE,
    PROFILE_C_BASE,
    PROFILE_HEVC_MAIN,
    PROFILE_HEVC_MAIN_STILL,
    PROFILE_HEVC_MAIN_INTRA,
    PROFILE_JPEG
}PROFILE_MODE_E;

enum venc_function
{
    stream_isize_adjustment = 1,
    bit_ratecontrol_slide_rule = 2,
    stream_state = 4,
    bit_ratecontrol_learning = 8,
    quality_Amendatory = 16,
    skip_mode = 32,
    calculte_QPtableRef = 64,
    frame_Resize = 128,
    application_of_NotifySceneChange = 256,
    gop_icontrol = 512,
};

typedef struct  akQP_TABLE_PARAM_S
{
    AK_U32 u32NumLCUs;
    AK_U32 u32NumQPPerLCU;
    AK_U32 u32NumBytesPerLCU;
}QP_TABLE_PARAM_S;

typedef struct CUR_QP_PARAM_S
{
    AK_S32 s32CurQp;
}CUR_QP_PARAM;


typedef struct akRATE_CTRL_PARAM_S
{
    AK_U16 u16SliceQP;
}RATE_CTRL_PARAM_S;

typedef struct akVENC_RC_STATUS_S
{
    AK_U32 u32SumCplx; // Sum of LCU complexity
    VIDEO_FRAME_TYPE_E eSliceType; // slice type of next frame
}VENC_RC_STATUS_S;

typedef enum akRC_TYPE_E
{
    INTERNAL = 0x00,  // Use internal Rate Ctrl of encoder lib.
    EXTERNAL = 0x01,  // Use external Rate Ctrl of user.
}RC_TYPE_E;


// User-space EXIF tag enum (avoid conflict with driver)
typedef enum {
    USER_EXIF_TAG_EXPOSURE_TIME = 0,    // Maps to driver EXIF_TAG_EXPOSURE_TIME
    USER_EXIF_TAG_ISO_SPEED_RATINGS = 1, // Maps to driver EXIF_TAG_ISO_SPEED_RATINGS
    USER_EXIF_TAG_MAX
} USER_EXIF_TAG_TYPE;


typedef struct {
    AK_U32    numerator;
    AK_U32    denominator;
} USER_ExifRational;

// EXIF value union
typedef union {
    USER_ExifRational rational_value;
    AK_U16 short_value;
} EXIF_VALUE;

// EXIF info structure
typedef struct {
    USER_EXIF_TAG_TYPE user_tag;
    EXIF_VALUE value;
} EXIF_INFO;


#define AK_VENC_MAX_DMA_BLOCK_NUM	20
typedef struct ak_venc_req_dma_block{
    unsigned int block_num;
    unsigned int block_size[AK_VENC_MAX_DMA_BLOCK_NUM];
}AK_VENC_REQ_DMA_BLOCK;

/**
 * AK_MPI_VENC_Open - open encoder and set encode param
 * @pstChnAttrs[IN]: encode param
 * @s32ChnId[IN]: s32ChnId
 * return: AK_SUCCESS success , others error code.
 */
AK_S32 AK_MPI_VENC_Open(AK_S32 s32ChnId, const VENC_CHN_ATTRS_S *pstChnAttrs);

/**
 * AK_MPI_VENC_Close - close video encode
 * @s32ChnId[IN]: s32ChnId from AK_MPI_VENC_Open
 * return: 0 success , others error code.
 */
AK_S32 AK_MPI_VENC_Close(AK_S32 s32ChnId);

/**
 * AK_MPI_VENC_EncodeFrame - encode single frame
 * @s32ChnId[IN]: s32ChnId from AK_MPI_VENC_Open
 * @pu8Frame[IN]: frame which you want to encode
 * @u32FrameLen[IN]: lenght of frame
 * @pvMdinfo[IN]: md info array
 * @pstStream[OUT]: encode output buffer address
 * return: 0 success , others error code.
 */
AK_S32 AK_MPI_VENC_EncodeFrame(AK_S32 s32ChnId, const AK_U8 *pu8Frame,AK_U32 u32FrameLen,
                                AK_VOID *pvMdinfo, VIDEO_STREAM_S *pstStream);

/**
 * AK_MPI_VENC_ReleaseStream - release stream resource
 * @s32ChnId[IN]: s32ChnId from AK_MPI_VENC_Open
 * @pstStream[IN]: stream return by AK_MPI_VENC_EncodeFrame()
 * return: 0 success , others error code.
 * notes:
 */
AK_S32 AK_MPI_VENC_ReleaseStream(AK_S32 s32ChnId, VIDEO_STREAM_S *pstStream);

/**
 * AK_MPI_VENC_GetAttr - get venc params
 * @s32ChnId[IN]: s32ChnId from AK_MPI_VENC_Open
 * @pstChnAttrs[OUT]: params
 * return: 0 success , others error code.
 * notes:
 */
AK_S32 AK_MPI_VENC_GetAttr(AK_S32 s32ChnId, VENC_CHN_ATTRS_S *pstChnAttrs);

/**
 * AK_MPI_VENC_SetAttr - set venc params with extend param
 * @s32ChnId[IN]: s32ChnId from AK_MPI_VENC_Open
 * @pstChnAttrs[IN]: 		 video encoder param  to set
 * return: 0 success , others error code.
 * notes:
 */
AK_S32 AK_MPI_VENC_SetAttr(AK_S32 s32ChnId, const VENC_CHN_ATTRS_S *pstChnAttrs);

/**
 * AK_MPI_VENC_RequestIdr - request I frame
 * @s32ChnId[IN]: s32ChnId from AK_MPI_VENC_Open
 * return: 0 success , others error code.
 * notes:
 */
AK_S32 AK_MPI_VENC_RequestIdr(AK_S32 s32ChnId);

/**
 * AK_MPI_VENC_GetStat - on stream-encode, get encode rate stat info
 * @s32ChnId[IN]: s32ChnId from AK_MPI_VENC_Open
 * @pstStat[OUT]: stream rate stat info
 * return: 0 success, others error code.
 * notes:
 */
AK_S32 AK_MPI_VENC_GetStat(AK_S32 s32ChnId, VENC_STAT_S *pstStat);

/**
 * AK_MPI_VENC_SetFps - set the venc chn fps
 * @s32ChnId[IN]: s32ChnId from AK_MPI_VENC_Open
 * @u32Fps[IN]: frame rate
 * return: 0 success, others error code.
 * notes: if venc chn not work on kernel-kernel slice frame mode,
 *        that func couldn't work if called.
 */
AK_S32 AK_MPI_VENC_SetFps(AK_S32 s32ChnId, AK_U32 u32Fps);

/**
 * AK_MPI_VENC_SetQp - set the venc qp value
 * @s32ChnId[IN]: s32ChnId from AK_MPI_VENC_Open
 * @u32Minqp[IN]: stream rate stat info
 * @u32Maxqp[IN]: stream rate stat info
 * return: 0 success, others error code.
 * notes: if venc opened in const qp mode, that func couldn't work if called.
 */
AK_S32 AK_MPI_VENC_SetQp(AK_S32 s32ChnId, AK_U32 u32Minqp, AK_U32 u32Maxqp);

/**
 * AK_MPI_VENC_ResetQp - reset the venc qp value
 * @s32ChnId[IN]: s32ChnId from AK_MPI_VENC_Open
 * return: 0 success, others error code.
 * notes: if venc opened in const qp mode, that func couldn't work if called.
 *        And that func will make the qp back to the value when opening.
 */
AK_S32 AK_MPI_VENC_ResetQp(AK_S32 s32ChnId);

/**
 * AK_MPI_VENC_SetRoiMdinfo - set the ROI param to night mode NR
 *                          (ROI reduces noise in night vision image)
 * @s32ChnId[IN]: s32ChnId from AK_MPI_VENC_Open
 * @u32Threshold[IN]: threshold of mdinfo value
 * @u32Quality[IN]:  max qp delta (Q0 format)
 * @u32RecoverStep[IN]: recover step of qp delta (Q2)
 * @u32WaitIntra[IN]: wait num of frames to enable Intra, 0-disable, 1-enable
 * @u32WaitSkip[IN]: wait num of frames to enable skip, 0-disable, 1-enable
 * return: 0 success, others error code.
 */
AK_S32 AK_MPI_VENC_SetRoiMdinfo(AK_S32 s32ChnId, AK_U32 u32Threshold, AK_S32 u32Quality, AK_S32 u32RecoverStep,
                                            AK_U32 u32WaitIntra, AK_U32 u32WaitSkip);

/**
 * AK_MPI_VENC_SetROISvpInfo - set the ROI SVP param
 * @s32ChnId[IN]: s32ChnId from AK_MPI_VENC_Open
 * @*pstRoiParam[in]		: roi svp param
 * return: 0 success , others error code.
 */
AK_S32 AK_MPI_VENC_SetRoiSvpInfo(AK_S32 s32ChnId, ROI_SVP_INFO_S *pstRoiParam);

/**
 * AK_MPI_VENC_GetIframeParam - get the venc iframe size
 * @s32ChnId[IN]: s32ChnId from AK_MPI_VENC_Open
 * @pu32Minqp[OUT]: Iframe stat info
 * @pu32Maxqp[OUT]: Iframe stat info
 * @u32MinFrameSize[OUT]: min frame KByte size
 * @pu32MaxFrameSize[OUT]: max frame KByte size
 * return: 0 success, others error code.
 */
AK_S32 AK_MPI_VENC_GetIframeParam(AK_S32 s32ChnId, AK_U32 *pu32Minqp, AK_U32 *pu32Maxqp,
                                    AK_U32 *u32MinFrameSize, AK_U32 *pu32MaxFrameSize);

/**
 * AK_MPI_VENC_SetIframeParam - set the venc iframe size
 * @s32ChnId[IN]: s32ChnId from AK_MPI_VENC_Open
 * @u32Minqp[IN]: Iframe stat info
 * @u32Maxqp[IN]: Iframe stat info
 * @u32MinFrameSize[IN]: min frame KByte size
 * @u32MaxFrameSize[IN]: max frame KByte size
 * return: 0 success, others error code.
 */
AK_S32 AK_MPI_VENC_SetIframeParam(AK_S32 s32ChnId, AK_U32 u32Minqp, AK_U32 u32Maxqp,
                                        AK_U32 u32MinFrameSize, AK_U32 u32MaxFrameSize);

/**
 * AK_MPI_VENC_StriclylimitFrameSize - Limit the size of frame strictly
 * @s32ChnId[IN]: s32ChnId from AK_MPI_VENC_Open
 * @u32Maxsize[IN]: max frame size(unit is kbytes),
 *              0 <= maxsize(KB) * 1024 < Stream_Buffer Size,
 *              when maxsize==0, will disable frame size limit
 * return: 0 success, others error code.
 */
AK_S32 AK_MPI_VENC_StriclylimitFrameSize(AK_S32 s32ChnId, AK_U32 u32MaxSize);

/**
 * AK_MPI_VENC_SetCrop - set the venc crop
 * @s32ChnId[IN]: s32ChnId from AK_MPI_VENC_Open
 * @pstCrop[IN]: venc crop
 * return: 0 success, others error code.
 * notes:
 */
AK_S32 AK_MPI_VENC_SetCrop(AK_S32 s32ChnId, const VENC_CROP_CFG_S *pstCrop);

/**
 * AK_MPI_VENC_GetCrop - get the venc crop
 * @s32ChnId[IN]: s32ChnId from AK_MPI_VENC_Open
 * @pstCrop[OUT]: venc crop
 * return: 0 success, others error code.
 * notes:
 */
AK_S32 AK_MPI_VENC_GetCrop(AK_S32 s32ChnId, VENC_CROP_CFG_S *pstCrop);

/**
 * AK_MPI_VENC_SetJpegSlice - set the jpeg slice encode config
 * @s32ChnId[IN]: s32ChnId from AK_MPI_VENC_Open
 * @u32PicHeight[IN]:total frame height
 * @u32FirstSliceHeight[IN]: first Slice Height
 * return: 0 success, others error code.
 */
AK_S32 AK_MPI_VENC_SetJpegSlice(AK_S32 s32ChnId, AK_U32 u32PicHeight, AK_U32 u32FirstSliceHeight);

/**
 * AK_MPI_VENC_JpegSliceEncode - jpeg slice encode
 * @s32ChnId[IN]: s32ChnId from AK_MPI_VENC_Open
 * @pu8frame[IN]: slice frame which you want to encode
 * @u32FrameLen[IN]: lenght of slice frame
 * @u32SliceIdx[IN]: index of slice number
 * @u32SliceHeight[IN]: height of slice frame
 * @pstStream[OUT]: encode output buffer address
 * return: 0 success , others error code.
 */
AK_S32  AK_MPI_VENC_JpegSliceEncode(AK_S32 s32ChnId, const AK_U8 *pu8Frame,
        AK_U32 u32FrameLen, AK_U32 u32SliceIdx, AK_U32 u32SliceHeight,VIDEO_STREAM_S *pstStream);

/**
 * AK_MPI_VENC_SetFunction - set the venc chn function capability
 * @s32ChnId[IN]: s32ChnId from AK_MPI_VENC_Open
 * @u32Function[IN]: function value bit
 * return: 0 success, others error code.
 * notes: diffrent mode work function to adjust some encoder capability
 */
AK_S32 AK_MPI_VENC_SetFunction(AK_S32 s32ChnId,AK_U32 u32Function);

/**
 * AK_MPI_VENC_GetFunction - get the venc chn function capability
 * @s32ChnId[IN]: s32ChnId from AK_MPI_VENC_Open
 * @pu32Function[IN]: pfunction value bit point
 * return: 0 success, others error code.
 * notes: diffrent mode work function to adjust some encoder capability
 */
AK_S32 AK_MPI_VENC_GetFunction(AK_S32 s32ChnId,AK_U32 *pu32Function);

/**
 * AK_MPI_VENC_GetVersion - get venc version
 * return: version string
 */
const AK_CHAR* AK_MPI_VENC_GetVersion(void);

/**
 * AK_MPI_VENC_GetEncLibVersion - get venc encode lib version
 * return: version string
 */
const AK_CHAR* AK_MPI_VENC_GetEncLibVersion(void);

/**
 * AK_MPI_VENC_SetRcType - set rc type;
 * @s32ChnId[IN]: s32ChnId from AK_MPI_VENC_Open
 * @eRcType[IN]: rc type;
 * return: 0 success, others error code.
 */
AK_S32 AK_MPI_VENC_SetRcType(AK_S32 s32ChnId, RC_TYPE_E eRcType);
/**
 * AK_MPI_VENC_SetRcParam - set rc param;
 * @s32ChnId[IN]: s32ChnId from AK_MPI_VENC_Open
 * @pstRcParam[IN]: rc param;
 * return: 0 success, others error code.
 */
AK_S32 AK_MPI_VENC_SetRcParam(AK_S32 s32ChnId, RATE_CTRL_PARAM_S *pstRcParam);
/**
 * AK_MPI_VENC_GetRcStat - get rc stat;
 * @s32ChnId[IN]: s32ChnId from AK_MPI_VENC_Open
 * @pstRcStatus[OUT]: rc stat;
 * return: 0 success, others error code.
 */
AK_S32 AK_MPI_VENC_GetRcStat(AK_S32 s32ChnId, VENC_RC_STATUS_S *pstRcStatus);
/**
 * AK_MPI_VENC_SetStreamBufSize - set encoder inner stream buf size;
 * @s32ChnId[IN]: s32ChnId from AK_MPI_VENC_Open
 * @u32Size[IN]: stream buf size;
 * return: 0 success, others error code.
 */
AK_S32 AK_MPI_VENC_SetStreamBufSize(AK_S32 s32ChnId, AK_U32 u32Size);
/**
 * AK_MPI_VENC_SetQpTable - set qp table;
 * @s32ChnId[IN]: handle id return by AK_MPI_VENC_OPEN
 * @pu8QpTable[IN]: QP Table;
 * @bRelativeQp[IN]: releative qp;
 * return: 0 success, others error code.
 */
AK_S32 AK_MPI_VENC_SetQpTable(AK_S32 s32ChnId, AK_U8 * pu8QpTable, AK_S32 s32QpTableSize,
                                            bool bRelativeQp, bool bAutoQp, bool bQpTableEnable);
/**
 * AK_MPI_VENC_GetQpTableParam - get qp table;
 * @s32ChnId[IN]: handle id return by AK_MPI_VENC_OPEN
 * @pstQpTableParam[OUT]: QP Table Param;
 * return: 0 success, others error code.
 */
AK_S32 AK_MPI_VENC_GetQpTableParam(AK_S32 s32ChnId, QP_TABLE_PARAM_S * pstQpTableParam);
/**
 * AK_MPI_VENC_GetCurQp - get current qp
 * @s32ChnId[IN]: handle id return by AK_MPI_VENC_OPEN
 * @pstCurQpParam[OUT]:  qp param
 * return: 0 success , others error code.
 * notes:
 */
AK_S32 AK_MPI_VENC_GetCurQp(AK_S32 s32ChnId, CUR_QP_PARAM *pstCurQpParam);
/**
 *@brief AK_MPI_VENC_GetStream get encode stream.
 *@param s32ChnId[IN] : venc chn id.
 *@param pstStream[OUT]: venc frame.
 *@return 0 success, others failed.
 *@note
*/
AK_S32 AK_MPI_VENC_GetStream(AK_S32 s32ChnId, VIDEO_STREAM_S *pstStream);
/**
 * AK_MPI_VENC_SetExifEnable - set exif enable;
 * @s32ChnId[IN]: handle id return by AK_MPI_VENC_OPEN
 * @bExifEnable[IN]: EXIF Enable;
 * return: 0 success, others error code.
 */
AK_S32 AK_MPI_VENC_SetExifEnable(AK_S32 s32ChnId, bool bExifEnable);
/**
 * AK_MPI_VENC_SetExifInfo - set jpeg exif information;
 * @s32ChnId[IN]: channel handle id returned by AK_MPI_VENC_CreateCHN
 * @pstExifInfo[IN]: pointer to EXIF information structure
 * return: 0 success, others error code.
 */
AK_S32 AK_MPI_VENC_SetExifInfo(AK_S32 s32ChnId, EXIF_INFO* pstExifInfo);

#endif

/* end of file */
