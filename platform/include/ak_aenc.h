/* SPDX-License-Identifier: Apache-2.0 */
/******************************************************************************

  Copyright (C), 2023-2026, Guangzhou Anyka Microelectronics Co., Ltd.

 ******************************************************************************
  File Name         	: ak_aenc.h
  Version        		: V1.0.0
  Author         		: Anyka Linux SDK Team
  Created       		: 2023/8/4
  Description  		    : aenc head file
  Example               :
  History       		:
  1.Date                : 2023/8/4
    Author              : 202107131457
    Modification        : Created file
******************************************************************************/

#ifndef _AK_AUDIO_ENCODE_H_
#define _AK_AUDIO_ENCODE_H_

#include "ak_common_audio.h"
#include "ak_common.h"

/**
 *encode aac type
 */
typedef enum akAENC_AAC_ATTR_E
{
    AENC_AAC_RESERVED = 0,
    AENC_AAC_SAVE_FRAME_HEAD,
    AENC_AAC_CUT_FRAME_HEAD
} AENC_AAC_ATTR_E;

/**
 * aenc error code
 */
typedef enum akAENC_ERROR_TYPE_E
{
    ERROR_AENC_OPEN_LIB_ERROR = (MODULE_ID_AENC << 24) + 0,
    ERROR_AENC_CLOSE_LIB_ERROR,
    ERROR_AENC_OPEN_TOO_MANY,
    ERROR_AENC_NOT_SUPPORT_TYPE,
    ERROR_AENC_INVALID_PARAM,
    ERROR_AENC_USER_NULL,
    ERROR_AENC_LIST_CNT_TOO_MANY,
    ERROR_AENC_RB_ERROR,
    ERROR_AENC_NO_DATA,
    ERROR_AENC_AUDIO_CODEC_ERROR,
    ERROR_AENC_NOT_OPEN
} AENC_ERROR_TYPE_E;


/**
 * aenc aac attr
 */
typedef struct akAENC_AAC_PARAM_S
{
    AENC_AAC_ATTR_E enAacFrmHead;  // Whether encoder returns ADTS header
}AENC_AAC_PARAM_S;

/**
 * aenc encoder pfn,use to resigster encoder
 * note: now not support user's encoder
 */
typedef struct akAENC_ENCODE_S
{
    AK_S32 enType; // T_AUDIO_TYPE
    AK_VOID* (*pfnEncOpen)(AK_VOID *pvOpenArg, AK_VOID *pstAttr);     // value can be  AENC_OpenXXX
    /* the follow pfn don't need to be assigned, default is NULL */
    AK_S32 (*pfnEncProcess)(AK_VOID *pvEncHandle, AK_VOID *pvProcArg);
    AK_S32 (*pfnEncClose)(AK_VOID *pvEncHandle);
    AK_S32 (*pfnEncReset)(AK_VOID *pvEncHandle);
    AK_S32 (*pfnEncFlush)(AK_VOID *pvEncHandle);
} AENC_ENCODE_S;

/**
 * aenc param,use to aenc open
 */
typedef struct akAENC_PARAM_S
{
    AUDIO_SAMPLE_RATE_E enSampleRate;  //samplerate
    AUDIO_SAMPLE_BIT_E enSampleBits;   //samplebit(16bit)
    AUDIO_SOUND_MODE_E enSoundMode;    //soundmode
    AUDIO_TYPE_E enType;               //encode type
    union {
        AENC_AAC_PARAM_S stAacParam;   //aac param
    } unAencAttr;
} AENC_PARAM_S;

/**
 * aenc out info,after open
 */
typedef struct akENC_OUT_INFO
{
    AK_S32 s32SamplesPerFrame;  // per frame sample point before encode
    AK_S32 s32FrameSize;        // frame size after encode
    AK_S32 s32BitRate;          // bitrate of encode
}ENC_OUT_INFO;


/**
 *@brief AK_MPI_AENC_SetLibPrintLevel - set codec lib print level
 *@param u32PrintLevel[IN]    : print level, see ATCLIB_PRINT_LEVEL_E
 *@return 0 success otherwise failed
 *@notes:
 */
AK_S32 AK_MPI_AENC_SetLibPrintLevel(AK_U32 u32PrintLevel);

/**
 *@brief AK_MPI_AENC_GetVersion - get audio encode version
 *@return aenc version string
 *@notes:
 */
 const AK_CHAR* AK_MPI_AENC_GetVersion(AK_VOID);

/**
 *@brief AK_MPI_AENC_Open - open anyka audio encode
 *@param pstParam[IN]: audio input data encode param
 *@param s32AencUserId[IN]: opened encode user id，[0-3]
 *@return 0 success otherwise failed
 *@notes:
 */
AK_S32 AK_MPI_AENC_Open(AK_S32 s32AencUserId, const AENC_PARAM_S *pstParam);

/**
 *@brief AK_MPI_AENC_SendFrame - send pcm data(frame) to encode
 *@param s32AencUserId[IN]: opened encode user id，[0-3]
 *@param pstPcmFrame[IN]: the audio pcm raw data info
 *@param u32TimeOut[IN]: u32TimeOut > 0,block mode;  u32TimeOut = 0, non block mode unit is ms
 *@return 0 success otherwise failed
 *@notes:
 */
AK_S32 AK_MPI_AENC_SendFrame(AK_S32 s32AencUserId, \
    const FRAME_S *pstPcmFrame, AK_U32 u32TimeOut);

/**
 *@brief AK_MPI_AENC_GetStream - get audio encoded data
 *@param s32AencUserId[IN]: opened encode user id，[0-3]
 *@param pstStream[OUT]: audio stream after encode
 *@param u32TimeOut[IN]: u32TimeOut > 0,block mode;  u32TimeOut = 0, non block mode unit is ms
 *@return 0 success otherwise failed
 *@notes:
 */
AK_S32 AK_MPI_AENC_GetStream(AK_S32 s32AencUserId, \
    AUDIO_STREAM_S *pstStream, AK_U32 u32TimeOut);

/**
 *@brief AK_MPI_AENC_SendEndOfFrame -  end of send frame
 *@param s32AencUserId[IN]: opened encode user id，[0-3]
 *@return 0 success otherwise failed
 *@notes:
 */
AK_S32 AK_MPI_AENC_SendEndOfFrame(AK_S32 s32AencUserId);

/**
 *@brief AK_MPI_AENC_ReleaseStream -  release audio data stream
 *@param s32AencUserId[IN]: opened encode user id，[0-3]
 *@param pstStream[IN]: audio stream entry from AK_MPI_AENC_GetStream
 *@return 0 success otherwise failed
 *@notes:
 */
AK_S32 AK_MPI_AENC_ReleaseStream(AK_S32 s32AencUserId,
    AUDIO_STREAM_S *pstStream);

/**
 *@brief AK_MPI_AENC_ClearEncodeBuf -  clear  audio encode bufffer
 *@param s32AencUserId[IN]: opened encode user id，[0-3]
 *@return 0 success otherwise failed
 *@notes:
 */
AK_S32 AK_MPI_AENC_ClearEncodeBuf(AK_S32 s32AencUserId);

/**
 *@brief AK_MPI_AENC_Close - close audio encode
 *@param s32AencUserId[IN]: opened encode user id，[0-3]
 * return: 0 success, other failed
 */
AK_S32 AK_MPI_AENC_Close(AK_S32 s32AencUserId);

/**
 *@brief AK_MPI_AENC_SaveFile - save two file, one file is stream input to aenc, \
 * two file is stream output from aenc
 *@param s32AencUserId[IN]: opened encode user id，[0-3]
 *@param pstSaveFileInfo[IN]: save file infomation
 *@return 0 success otherwise failed
 *@notes:
 */
AK_S32 AK_MPI_AENC_SaveFile(AK_S32 s32AencUserId, AUDIO_SAVE_FILE_INFO_S *pstSaveFileInfo);

/**
 *@brief AK_MPI_AENC_GetEncVersion - get audio code lib version
 *@return audio code lib version
 *@notes:
 */
AK_CHAR *AK_MPI_AENC_GetEncVersion(AK_VOID);

/**
 *@brief AK_MPI_AENC_RegisterEncoder - register encoder function
 *@param pstEncode[IN]: register encoder callback struct
 *@return 0 success otherwise failed
 *@notes:
 */
AK_S32 AK_MPI_AENC_RegisterEncoder(AENC_ENCODE_S * pstEncode);

/**
 *@brief AK_MPI_AENC_GetEncParam - get enc param after open
 *@param s32AencUserId[IN]: opened encode user id，[0-3]
 *@param stEncLenPam[OUT] : get encoder output info
 *@return 0 success otherwise failed
 *@notes:
 */
AK_S32 AK_MPI_AENC_GetEncParam(AK_S32 s32AencUserId, ENC_OUT_INFO *stEncLenPam);


#endif
/* end of file */
