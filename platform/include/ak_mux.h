/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd. */
#ifndef _AK_MPI_MUX_H_
#define _AK_MPI_MUX_H_

#include "ak_common.h"
#include "ak_common_audio.h"
#include "ak_common_video.h"
#include "ak_venc.h"

typedef enum akMUX_ERROR_E
{
    MUX_ERROR_OPEN_LIB_ERROR = (MODULE_ID_MUX << 24) + 0,
    MUX_ERROR_CLOSE_LIB_ERROR,
    MUX_ERROR_GET_LIB_INFO_ERROR,
    MUX_ERROR_START_LIB_ERROR,
    MUX_ERROR_STOP_LIB_ERROR,
    MUX_ERROR_OPEN_TOO_MANY,
    MUX_ERROR_USER_NULL,
    MUX_ERROR_SET_AUDIO_INPUT_ERROR,
    MUX_ERROR_SET_VIDEO_INPUT_ERROR,
    MUX_ERROR_ADD_ERROR,
    MUX_ERROR_ADD_VIDEO_FIRST,
    MUX_ERROR_VALUE_INVAILD,
}MUX_ERROR_E;

typedef enum akMUX_TYPE_E
{
    MUX_TYPE_AVI,
    MUX_TYPE_MP4,// 3GP
    MUX_TYPE_NUM
}MUX_TYPE_E;

typedef enum akMUX_DATA_DEST_E
{
    MUX_DATA_TO_UNKNOWN = 0x00,
    MUX_DATA_TO_LOCAL_FILE,
    MUX_DATA_TO_INTERNET
}MUX_DATA_DEST_E;

typedef enum akMUX_STATUS_E
{
    MUX_STATUS_OPEN,
    MUX_STATUS_STOP,
    MUX_STATUS_DOING,
    MUX_STATUS_SYSERR,
    MUX_STATUS_MEMFULL,
    MUX_STATUS_SYNERR,
    MUX_STATUS_WAITING,
    MUX_STATUS_PAUSE,
    MUX_STATUS_NUM,
}MUX_STATUS_E;


typedef enum akMUX_NODE_E { // Record output file types
    MUX_NODE_RECORD = 0x00,
    MUX_NODE_IDX,
    MUX_NODE_MP4_MOOV,
    MUX_NODE_MP4_STBL,
    MUX_NODE_NUM
}MUX_NODE_E;


typedef struct akMUX_START_PARAM_S
{
    AK_S8 *ps8FilePath;
    AK_S8 *ps8TmpFilePath;
    AK_S32 s32BuffRecord;
    AK_S8 *ps8BuffRecord;
    AK_S32 s32BuffIndex;
    AK_S8 *ps8BuffIndex;
}MUX_START_PARAM_S;


typedef struct akMUX_CONF_S
{
    /* video */
    VENC_OUTPUT_TYPE_E enVencOutputType;
    AK_U16 u16Width;
    AK_U16 u16Height;
    AK_U16 u16Fps;

    /* audio */
    AK_U8 u8CaptureAudio;
    AUDIO_TYPE_E enAudioType;
    AUDIO_DATA_ATTR_S stAudioDataAttr;
    AK_S32 s32FrameLen;

    /* mux */
    MUX_TYPE_E enMuxType;
    MUX_DATA_DEST_E enMuxDataDest;
    AK_U32 u32RecordSec;
}MUX_CONF_S;

typedef struct akMUX_FILEINFO_S // File info returned by record repair API
{
    AK_U64 u64TotalMs ;         // Total duration in ms after repair
}MUX_FILEINFO_S;

/**
 * AK_MPI_MUX_Get_Version - get mux version
 * return: version string
 * notes:
 */
const AK_S8* AK_MPI_MUX_Get_Version(AK_VOID);

/**
 * AK_MPI_MUX_Open - open mux lib
 * @pstMuxParam[IN]: mux input param
 * @s32MuxHandleId[OUT]: mux handle id
 * return: 0 success, other failed
 */
AK_S32 AK_MPI_MUX_Open(const MUX_CONF_S *pstMuxParam, AK_S32 *s32MuxHandleId);

/**
 * AK_MPI_MUX_Start - mux start
 * @s32MuxHandleId[IN]: opened mux handle id
 * @pstStartParam[IN]: save file poAK_S32er
 * return: 0 success, other failed
 */
AK_S32 AK_MPI_MUX_Start(AK_S32 s32MuxHandleId, MUX_START_PARAM_S *pstStartParam);

/**
 * AK_MPI_MUX_Add_Audio - add audio to mux
 * @s32MuxHandleId[IN]: opened mux handle id
 * @audio[IN]: audio stream info
 * return: 0 success, other failed
 */
AK_S32 AK_MPI_MUX_Add_Audio(AK_S32 s32MuxHandleId, const AUDIO_STREAM_S *pstAudioStream);

/**
 * AK_MPI_MUX_Add_Video - add video to mux
 * @s32MuxHandleId[IN]: opened mux handle id
 * @video[IN]: video stream info
 * return: 0 success, other failed
 */
AK_S32 AK_MPI_MUX_Add_Video(AK_S32 s32MuxHandleId, const VIDEO_STREAM_S *pstVideoStream);

/**
 * AK_MPI_MUX_Stop - mux stop
 * @s32MuxHandleId[IN]: opened mux handle id
 * return: 0 success, other failed
 */
AK_S32 AK_MPI_MUX_Stop(AK_S32 s32MuxHandleId);

/**
 * AK_MPI_MUX_Close - close mux module
 * @s32MuxHandleId[IN]: opened mux handle
 * return: 0 success, other failed
 */
AK_S32 AK_MPI_MUX_Close(AK_S32 s32MuxHandleId);

/**
 * AK_MPI_MUX_Print_Runtime_Status - prAK_S32 runtime debug information
 * @s32MuxHandleId[IN]: mux handle id
 * return: 0 success, other failed
 * notes:
 */
AK_S32 AK_MPI_MUX_Print_Runtime_Status(AK_S32 s32MuxHandleId);

/**
 * AK_MPI_MUX_Fix_File - release mux lib resource
 * @pFILERecord[IN]: record file poAK_S32er
 * @pFILEIndex[IN]:  avi index file poAK_S32er
 * @pFILEMoov[IN]:  mp4 moov file poAK_S32er
 * @pFILEStbl[IN]:  mp4 stbl file poAK_S32er
 * @enMuxType[IN]:  MUX_TYPE_AVI | MUX_TYPE_MP4
 * @pstMuxFileinfo[OUT]:  return file info
 * return: 0 success, other failed
 */
AK_S32 AK_MPI_MUX_Fix_File( FILE *pFILERecord, FILE *pFILEIndex, FILE *pFILEMoov,
                            FILE *pFILEStbl, MUX_TYPE_E enMuxType, MUX_FILEINFO_S *pstMuxFileinfo );

/**
 * AK_MPI_MUX_Pause - pause mux audio video to file
 * @s32MuxHandleId[IN]:  mux handle id
 * return: 0 success, other failed
 */
AK_S32 AK_MPI_MUX_Pause(AK_S32 s32MuxHandleId);

/**
 * AK_MPI_MUX_Continue - continue mux audio video to file
 * @s32MuxHandleId[IN]:  mux handle id
 * return: 0 success, other failed
 */
AK_S32 AK_MPI_MUX_Continue(AK_S32 s32MuxHandleId);

/**
 * AK_MPI_MUX_Get_Status - get mux lib status
 * @s32MuxHandleId[IN]: mux handle id
 * @penMuxStatus[OUT]: status result. Can't be NULL.
 * return: 0 success, other failed
 */
AK_S32 AK_MPI_MUX_Get_Status(AK_S32 s32MuxHandleId, MUX_STATUS_E *penMuxStatus );

#endif
