/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd. */
#ifndef _AK_MPI_DEMUX_H_
#define _AK_MPI_DEMUX_H_

#include "ak_common_video.h"
#include "ak_common_audio.h"

typedef enum akDEMUX_ERROR_TYPE_E
{
    DEMUX_ERROR_OPEN_LIB_ERROR = (MODULE_ID_DEMUX << 24) + 0,
    DEMUX_ERROR_CLOSE_LIB_ERROR,
    DEMUX_ERROR_GET_LIB_INFO_ERROR,
    DEMUX_ERROR_START_LIB_ERROR,
    DEMUX_ERROR_STOP_LIB_ERROR,
    DEMUX_ERROR_OPEN_TOO_MANY,
    DEMUX_ERROR_USER_NULL,
    DEMUX_ERROR_LIB_HANDLE_NULL,
    DEMUX_ERROR_AUDIO_TYPE_ERROR,
    DEMUX_ERROR_VIDEO_TYPE_ERROR,
    DEMUX_ERROR_AUDIO_END,
    DEMUX_ERROR_VIDEO_END,
    DEMUX_ERROR_LIB_ERROR,
    DEMUX_ERROR_NO_AUDIO,
    DEMUX_ERROR_NO_VIDEO,
    DEMUX_ERROR_SRC_EOF,
}DEMUX_ERROR_TYPE_E;

typedef enum akDEMUX_DATA_TYPE_E
{
    DEMUX_DATA_UNKNOW = 0x00,
    DEMUX_DATA_VIDEO,
    DEMUX_DATA_AUDIO
}DEMUX_DATA_TYPE_E;

typedef enum akDEMUX_DIRECTION_E
{
    DEMUX_DIRECTION_NEAREST = 0,    //seek to nearest key frame
    DEMUX_DIRECTION_LAST,            //seek to last key frame
    DEMUX_DIRECTION_NEXT            //seek to next key frame
}DEMUX_DIRECTION_E;

/* demux media data source */
typedef enum akDEMUX_DATA_SOURCE_E
{
    DEMUX_DATA_FROM_UNKNOWN = 0x00,
    DEMUX_DATA_FROM_LOCAL_FILE,
    DEMUX_DATA_FROM_INTERNET
}DEMUX_DATA_SOURCE_E;

typedef struct akDEMUX_CONF_S
{
    AK_VOID *pvFromDIR;// file_fd or socket
    DEMUX_DATA_SOURCE_E enDemuxDataSource;
    AK_U64 u64StartMs;
}DEMUX_CONF_S;

typedef struct akDEMUX_STREAM_S
{
    AK_U8 *pu8Data;    // demux pstDemuxStream data
    AK_U32 u32Len;         // demux pstDemuxStream len in bytes
    AK_U64 u64Ts;    // demux timestamp(u64Ms)
    VIDEO_FRAME_TYPE_E enVideoFrameType;    // demux I or P frame
}DEMUX_STREAM_S;

typedef struct akDEMUX_VIDEO_INFO_S
{
    AK_U32 u32Fps;
    AK_S32 enVencOutputType;// enum encode_output_type
    AK_U16 u16Width;
    AK_U16 u16Height;
    AK_DOUBLE dFps;
}DEMUX_VIDEO_INFO_S;

typedef struct akDEMUX_AUDIO_INFO_S
{
    AUDIO_DATA_ATTR_S stAudioDataAttr;
    AUDIO_TYPE_E enAudioType; //encode | decode audio type
}DEMUX_AUDIO_INFO_S;

/**
 * AK_MPI_DEMUX_Get_Version - get demux version
 * return: version string
 * notes:
 */
const AK_S8* AK_MPI_DEMUX_Get_Version(AK_VOID);

/**
 * AK_MPI_DEMUX_Open - open demux lib
 * @pstDemuxConf[IN]: demux input param
 * @s32DemuxHandleId[OUT]: demux handle id
 * return: 0 success, other failed
 */
AK_S32 AK_MPI_DEMUX_Open(const DEMUX_CONF_S *pstDemuxConf, AK_S32 *s32DemuxHandleId);

/**
 * AK_MPI_DEMUX_Get_Total_Time: get media  total time
 * @s32DemuxHandleId[IN]: opened demux handle id
 * @u64TotalTimeMs[OUT]: media total time(u64Ms)
 * return: 0 on succeess, other failed
 */
AK_S32 AK_MPI_DEMUX_Get_Total_Time(AK_S32 s32DemuxHandleId, AK_U64 *u64TotalTimeMs);

/**
 * AK_MPI_DEMUX_Get_Video_Info: get media  video information
 * @s32DemuxHandleId[IN]: opened demux handle id
 * @pstDemuxVideoInfo[OUT]: store video info
 * return: 0 on succeess, other failed
 */
AK_S32 AK_MPI_DEMUX_Get_Video_Info(AK_S32 s32DemuxHandleId, DEMUX_VIDEO_INFO_S *pstDemuxVideoInfo);

/**
 * AK_MPI_DEMUX_Get_Audio_Info: get media  audio information
 * @s32DemuxHandleId[IN]: opened demux handle id
 * @pstDemuxAudioInfo[OUT]: store audio info
 * return: 0 on succeess, other failed
 */
AK_S32 AK_MPI_DEMUX_Get_Audio_Info(AK_S32 s32DemuxHandleId, DEMUX_AUDIO_INFO_S *pstDemuxAudioInfo);

/**
 * AK_MPI_DEMUX_Get_Data - mux start
 * @s32DemuxHandleId[IN]: opened mux handle id
 * @penDemuxDataType[IN]: the penDemuxDataType of data to get
 * @pstDemuxStream[OUT]: the pstDemuxStream of the penDemuxDataType data
 * return: 0 success, other failed
 */
AK_S32 AK_MPI_DEMUX_Get_Data(AK_S32 s32DemuxHandleId, DEMUX_DATA_TYPE_E *penDemuxDataType,
                             DEMUX_STREAM_S *pstDemuxStream);

/**
 * AK_MPI_DEMUX_Get_Video - get video data after demux
 * @s32DemuxHandleId[IN]: opened demux handle
 * @pstDemuxStream[OUT]:  video data pstDemuxStream
 * return: the pstDemuxStream packet after demux, NULL failed
 */
AK_S32 AK_MPI_DEMUX_Get_Video(AK_S32 s32DemuxHandleId, DEMUX_STREAM_S *pstDemuxStream);

/**
 * AK_MPI_DEMUX_Get_Audio - get audio data after demux
 * @s32DemuxHandleId[IN]: opened demux handle
 * @pstDemuxStream[OUT] : audio data pstDemuxStream
 * return: the pstDemuxStream packet after demux, NULL failed
 */
AK_S32 AK_MPI_DEMUX_Get_Audio(AK_S32 s32DemuxHandleId, DEMUX_STREAM_S *pstDemuxStream);

/**
 * AK_MPI_DEMUX_Free_Data - demux free data
 * @s32DemuxHandleId[IN]: opened mux handle id
 * @pstDemuxStream[IN]         : pstDemuxStream pointer to be free
 * return: 0 success, other failed
 */
AK_S32 AK_MPI_DEMUX_Free_Data(AK_S32 s32DemuxHandleId, DEMUX_STREAM_S *pstDemuxStream);

/**
 * AK_MPI_DEMUX_close - close demux lib
 * @s32DemuxHandleId[IN]: opened mux handle
 * return: 0 success, other failed
 */
AK_S32 AK_MPI_DEMUX_Close(AK_S32 s32DemuxHandleId);

/**
 * AK_MPI_DEMUX_Seek_To_Keyframe: seek to the time of video
 * @s32DemuxHandleId[IN]: opened mux handle
 * @u64Ms[IN]: s64Offset time base on file-begin
 * @enDemuxDirection[IN]: setting the direction
 * return: on success, ts with frameget closely keyframe, else return -1;
 */
AK_S32 AK_MPI_DEMUX_Seek_To_Keyframe(AK_S32 s32DemuxHandleId, AK_U64 u64Ms,
                                    DEMUX_DIRECTION_E enDemuxDirection);
#endif
