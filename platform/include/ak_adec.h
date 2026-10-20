/* SPDX-License-Identifier: Apache-2.0 */
/******************************************************************************

  Copyright (C), 2023-2026, Guangzhou Anyka Microelectronics Co., Ltd.

 ******************************************************************************
  File Name         	: ak_adec.h
  Version        		: V1.0.0
  Author         		: Anyka Linux SDK Team
  Created       		: 2023/8/4
  Description  		    : adec head file
  Example               :
  History       		:
  1.Date                : 2023/8/4
    Author              : 202107131457
    Modification        : Created file
******************************************************************************/

#ifndef _AK_AUDIO_DECODE_H_
#define _AK_AUDIO_DECODE_H_

#include "ak_common_audio.h"

/**
 * adec aac attr
 */
typedef struct akADEC_AAC_PARAM_S
{
    AK_U16 u16AacStreamFormat; // 0: auto detect; 1: adts; 2: adif; 3: raw
}ADEC_AAC_PARAM_S;

/**
 * adec decoder pfn,use to resigster deocder
 * note: now not support user's decoder
 */
typedef struct akADEC_DECODE_S
{
    AUDIO_TYPE_E enType;
    AK_VOID *(*pfnDecOpen)(AK_VOID *pvOpenArg, AK_VOID *pstAttr);          // value can be ADEC_OpenXXX
    /* the follow pfn don't need to be assigned, default is NULL */
    AK_S32 (*pfnDecProcess)(AK_VOID *pvAdecHandle, AK_VOID *pvProcArg);
    AK_S32 (*pfnDecClose)(AK_VOID *pvAdecHandle);
    AK_S32 (*pfnDecReset)(AK_VOID *pvAdecHandle);
    AK_S32 (*pfnDecBufferClear)(AK_VOID *pvAdecHandle);
    AK_S32 (*pfnDecBufFill)(AK_VOID *pvAdecHandle, AK_VOID *pvFillArg);
    AK_S32 (*pfnDecBufGetFreeLen)(AK_VOID *pvAdecHandle);
    AK_S32 (*pfnDecGetInputDataSize)(AK_VOID *pvAdecHandle);
    AK_S32 (*pfnDecSetInbufMinLen)(AK_VOID *pvAdecHandle, AK_U32 u32Len);
    AK_S32 (*pfnDecSetBufMode)(AK_VOID *pvAdecHandle, AK_S32 s32BufMode);
}ADEC_DECODE_S;

/**
 * adec error code
 */
typedef enum akADEC_ERROR_TYPE_E
{
    ERROR_ADEC_OPEN_LIB_ERROR = (MODULE_ID_ADEC << 24) + 0,
    ERROR_ADEC_CLOSE_LIB_ERROR,
    ERROR_ADEC_OPEN_TOO_MANY,
    ERROR_ADEC_USER_NULL,
    ERROR_ADEC_RB_ERROR,
    ERROR_ADEC_ATC_LIB_ERROR,
    ERROR_ADEC_SEND_STREAM_ERROR,
    ERROR_ADEC_GET_FRAME_NO_DATA,
    ERROR_ADEC_SEND_STREAM_NO_SPACE,
    ERROR_ADEC_STREAM_END,
    ERROR_ADEC_SEND_STREAM_NO_START
}ADEC_ERROR_TYPE_E;

/**
 * adec param,use to adec open
 */
typedef struct akADEC_PARAM_S
{
    AUDIO_TYPE_E enType;       //encode/decode type
    AUDIO_SAMPLE_RATE_E enSampleRate;
    AUDIO_SAMPLE_BIT_E enSampleBits;
    AUDIO_SOUND_MODE_E enSoundMode;
    union {
        ADEC_AAC_PARAM_S stAacParam;
    } unAdecAttr;
} ADEC_PARAM_S;

/**
 * adec buffer status,use to get deocder buffer param
 */
typedef struct akDEC_BUF_STATUS_S
{
    AK_S32 s32BufTotalLen;
    AK_S32 s32BufRemainLen;
} DEC_BUF_STATUS_S;


/**
 *@brief AK_MPI_ADEC_SetLibPrintLevel - set decodec lib print level
 *@param u32PrintLevel[IN]    : print level, see ATCLIB_PRINT_LEVEL_E
 *@return 0 success otherwise failed
 *@notes:
 */
AK_S32 AK_MPI_ADEC_SetLibPrintLevel(AK_U32 u32PrintLevel);

/**
 *@brief AK_MPI_ADEC_GetVersion - get audio decode version
 *@return: version string
 *@notes:
 */
const AK_CHAR *AK_MPI_ADEC_GetVersion(AK_VOID );

/**
 *@brief AK_MPI_ADEC_GetEncVersion - get audio code lib version
 *@return: audio code lib version
 *@notes:
 */
AK_CHAR *AK_MPI_ADEC_GetEncVersion(AK_VOID);


/**
 *@brief AK_MPI_ADEC_Open - open audio decode
 *@param pstParam[IN]: adec open params
 *@param s32AdecUserId[OUT]: adec user id number
 *@return 0 success otherwise failed
 *@notes:
 */
AK_S32 AK_MPI_ADEC_Open(AK_S32 s32AdecUserId, const ADEC_PARAM_S *pstParam);

/**
 *@brief AK_MPI_ADEC_SetOutputPcmLen - set output pcm data length
 *@param s32AdecUserId[IN]: adec user id number
 *@param u32OutPcmLen[IN]: set output pcm data length
 *@return 0 success otherwise failed
 *@notes:
 */
AK_S32 AK_MPI_ADEC_SetOutputPcmLen(AK_S32 s32AdecUserId, AK_U32 u32OutPcmLen);

/**
 *@brief AK_MPI_ADEC_SendStream - send decode audio stream
 *@param s32AdecUserId[IN]: adec user id number
 *@param pu8Data[IN]: data add to decode buffer
 *@param u32Len[IN]: add to decode buffer data len
 *@param u32TimeOut[IN]: =[1-60000] timeout ; =0 without timeout unit is ms
 *@return 0 success otherwise failed
 *@notes:
 */
 AK_S32 AK_MPI_ADEC_SendStream(AK_S32 s32AdecUserId, const AK_U8 *pu8Data, const AK_U32 u32Len, AK_U32 u32TimeOut);

 /**
 *@brief AK_MPI_ADEC_GetFrame -get pcm data after decode
 *@param s32AdecUserId[IN]: adec user id number
 *@param pstPcmFrame[OUT]: audio decode frame out buffer
 *@param u32TimeOut[IN]: =[1-60000] is timeout; =0 without timeout unit is ms
 *@return 0 success otherwise failed
 *@notes:
 */
 AK_S32 AK_MPI_ADEC_GetFrame(AK_S32 s32AdecUserId, FRAME_S *pstPcmFrame, AK_U32 u32TimeOut);

 /**
 *@brief AK_MPI_ADEC_ReleaseFrame -release frame data
 *@param s32AdecUserId[IN]: adec user id number
 *@param pstPcmFrame[IN]: audio decode frame out buffer
 *@return 0 success otherwise failed
 *@notes:
 */
 AK_S32 AK_MPI_ADEC_ReleaseFrame(AK_S32 s32AdecUserId, FRAME_S *pstPcmFrame);

 /**
 *@brief AK_MPI_ADEC_SendEndofStream - notice send stream end
 *@param s32AdecUserId[IN]: adec user id number
 *@return 0 success otherwise failed
 *@note: when play data send end,use this function
 */
AK_S32 AK_MPI_ADEC_SendEndofStream(AK_S32 s32AdecUserId);

/**
 *@brief AK_MPI_ADEC_ClearDecodeBuf - clear decoder buffer
 *@param s32AdecUserId[IN]: adec user id number
 *@return 0 success otherwise failed
 *@notes:
 */
AK_S32 AK_MPI_ADEC_ClearDecodeBuf(AK_S32 s32AdecUserId);

/**
 *@brief AK_MPI_ADEC_Close - close audio decoder
 *@param s32AdecUserId[IN]: adec user id number
 *@return 0 success otherwise failed
 *@notes:
 */
AK_S32 AK_MPI_ADEC_Close(AK_S32 s32AdecUserId);

/**
 *@brief AK_MPI_ADEC_SaveFile - save 4 files, one file is stream input to adec,
 *two file is pcm output from adec, three is stream before decode,
 *four is pcm after decode
 *@param s32AdecUserId[IN]: adec user id number
 *@param pstSaveFileInfo[IN]: save file infomation
 *@return 0 success otherwise failed
 *@notes:
 */
AK_S32 AK_MPI_ADEC_SaveFile(AK_S32 s32AdecUserId, \
    AUDIO_SAVE_FILE_INFO_S *pstSaveFileInfo);

/**
 *@brief AK_MPI_ADEC_GetBufStatus - audio output get device buffer status
 *@param s32AdecUserId[IN]: adec opened user id
 *@param pstBufStatus[OUT]: get decode buffer total length and
 * decode buffer remain length
 *@return 0 success otherwise failed
 *@notes:
 */
AK_S32 AK_MPI_ADEC_GetBufStatus(AK_S32 s32AdecUserId,\
    DEC_BUF_STATUS_S *pstBufStatus);

/**
 *@brief AK_MPI_ADEC_RegisterDecoder - register decoder function
 *@param pstEncode[IN]: register decoder callback struct
 *@return 0 success otherwise failed
 *@notes:
 */
AK_S32 AK_MPI_ADEC_RegisterDecoder(ADEC_DECODE_S *pstDecode);



#endif
/* end of file */
