/* SPDX-License-Identifier: Apache-2.0 */
/******************************************************************************

  Copyright (C), 2023-2026, Guangzhou Anyka Microelectronics Co., Ltd.

 ******************************************************************************
  File Name         	: ak_ao.h
  Version        		: V1.0.0
  Author         		: Anyka Linux SDK Team
  Created       		: 2023/9/1
  Description  		    : ao head file
  Example               :
  History       		:
  1.Date                : 2023/9/1
    Author              : 202107131457
    Modification        : Created file
******************************************************************************/

#ifndef _AK_AO_H_
#define _AK_AO_H_

#include "ak_common_audio.h"
#include "ak_common.h"

#define AUDIO_AO_MAX_CHN_NUM    (1)
#define DAC_DEV_MIN_GAIN        (-24)       /* analog gain */
#define DAC_DEV_MAX_GAIN        (0)       /* analog gain */

/**
 * ao mode error code
 */
typedef enum akAO_ERROR_TYPE_E
{
    ERROR_AO_OPEN_LIB_ERROR = (MODULE_ID_AO << 24) + 0,
    ERROR_AO_INIT_LIB_ERROR,
    ERROR_AO_CLOSE_LIB_ERROR,
    ERROR_AO_DEV_ID_NOT_SUPPORT,
    ERROR_AO_CHN_ID_NOT_SUPPORT,
    ERROR_AO_DEV_INFO_NULL,
    ERROR_AO_CHN_INFO_NULL,
    ERROR_AO_DEV_ALREADY_OPEN,
    ERROR_AO_CHN_ALREADY_OPEN,
    ERROR_AO_DEV_HANDLE_NULL,
    ERROR_AO_DEV_INVALID_FD,
    ERROR_AO_DEV_PLAYING,
    ERROR_AO_SET_PARAM_IS_THE_SAME,
    ERROR_AO_FRAME_SIZE_TOO_LONG,
    ERROR_AO_SET_MUTE_ERROR,
    ERROR_AO_SET_DAC_GAIN_ERROR,
    ERROR_AO_NOTICE_END_ERROR,
    ERROR_AO_DRIVER_FAILED,
    ERROR_AO_EQ_ERROR,
    ERROR_AO_CLOSE_FILTER_LIB_ERROR,
    ERROR_AO_GAIN_ERROR,
    ERROR_AO_MUTE_IS_ON,
    ERROR_AO_VQE_ERROR,
    ERROR_AO_THREAD_CREATE_FAILED,
    ERROR_AO_PUB_ATTR_ERROR,
    ERROR_AO_DEV_CHN_NOT_MATCH,
    ERROR_AO_DEV_CLOSE_ERROR,
    ERROR_AO_CHN_RUNNING,
    ERROR_AO_CHN_HAS_BEEN_ENABLED,
    ERROR_AO_CONF_FILE_ERROR,
    ERROR_AO_OPEN_LOOPBACK,
    ERROR_AO_INIT_SEM,
    ERROR_AO_FILL_VQE_ERROR,
    ERROR_AO_CHN_NOT_ENABLE,
    ERROR_AO_STRING_TOO_BIG,
    ERROR_AO_DEV_NOT_OPEN,
    ERROR_AO_NOT_SET_PUBATTR
}AO_ERROR_TYPE_E;

/* set dac conf */
typedef struct akAUDIO_DAC_CONFIG_S
{
    AK_S32 sReserved;
} AUDIO_DAC_CONFIG_S;

/**
 * audio param of ao, use to enable dev
 */
typedef struct akAUDIO_OUT_PARAM_S
{
    AUDIO_SAMPLE_RATE_E enSampleRate;       //sample rate
    AUDIO_SAMPLE_BIT_E  enSampleBits;       //samplt bits now only support 16bit
    AUDIO_SOUND_MODE_E  enSoundMode;        //soundmode :mono stereo queue
    AK_U32 u32SrcCnt;                       //source  count
    AK_U32 u32ChnCnt;                       //channel count
    union {
        AUDIO_DAC_CONFIG_S stDacConfig;     //dac config of ao
    }unSrcConfig;
    AK_U32 u32Reserved;
} AUDIO_OUT_PARAM_S;


/**
 *@brief AK_MPI_AO_GetVersion - get audio out version
 *@return: version string
 *@notes:
 */
const AK_CHAR* AK_MPI_AO_GetVersion(AK_VOID);

/**
 *@brief AK_MPI_AO_GetFilterVersion - get filter version of ao
 *@return: version string
 *@notes:
 */
const AK_CHAR *AK_MPI_AO_GetFilterVersion(AK_VOID);

/**
 *@brief AK_MPI_AO_GetVqeVersion - get filter version
 *@return: version string
 *@notes:
 */
const AK_CHAR *AK_MPI_AO_GetVqeVersion(AK_VOID);

/**
 *@brief AK_MPI_AO_SetPubAttr  : set pub attr of ao
 *@param s32AoDevId[IN]   : dev id
 *@param pstParam[IN]  : ao dev param
 *return 0 success, others failed.
 *@note: malloc ao dev info
 */
AK_S32 AK_MPI_AO_SetPubAttr(AK_S32 s32AoDevId, AUDIO_OUT_PARAM_S *pstParam);


/**
 *@brief AK_MPI_AO_GetPubAttr  : get pub attr of ao
 *@param s32AoDevId[IN]   : dev id
 *@param pstParam[OUT]  : ao dev param
 *return 0 success, others failed.
 *@note:
 */
AK_S32 AK_MPI_AO_GetPubAttr(AK_S32 s32AoDevId, AUDIO_OUT_PARAM_S *pstParam);


/**
 *@brief AK_MPI_AO_Enable - enable device
 *@param s32AoDevId[IN]: audio out opened dev id
 *@return: 0 success, otherwise failed
 *@notes:
 */
AK_S32 AK_MPI_AO_Enable(AK_S32 s32AoDevId);

/**
 *@brief AK_MPI_AO_Disable - disable device
 *@param s32AoDevId[IN]: audio out opened dev id
 *@return: 0 success, otherwise failed
 *@notes:
 */
AK_S32 AK_MPI_AO_Disable(AK_S32 s32AoDevId);

/**
 *@brief AK_MPI_AO_EnableChn - enable ao chn
 *@param s32AoDevId[IN]: dev id
 *@param s32AoChnId[IN]: chn id
 *@param s32AoChnId[IN]: chn id
 *@return: 0 success, otherwise failed
 *@notes:
 */
AK_S32 AK_MPI_AO_EnableChn(AK_S32 s32AoDevId, AK_S32 s32AoChnId);

/**
 *@brief AK_MPI_AO_DisableChn - disable ao chn
 *@param s32AoDevId[IN]: dev id
 *@param s32AoChnId[IN]: chn id
 *@return: 0 success, otherwise failed
 *@notes:
 */
AK_S32 AK_MPI_AO_DisableChn(AK_S32 s32AoDevId, AK_S32 s32AoChnId);

/**
 *@brief AK_MPI_AO_SetGain - set dev gain
 *@param s32AoDevId[IN]: dev id
 *@param s32AoChnId[IN]: chn id
 *@param s32Gain[IN]: gain value
 *@return: 0 success, otherwise failed
 *@notes:
 */
AK_S32 AK_MPI_AO_SetGain(AK_S32 s32AoDevId, AK_S32 s32AoChnId, AK_S32 s32Gain);

/**
 *@brief AK_MPI_AO_SetGain - set dev gain
 *@param s32AoDevId[IN]: dev id
 *@param s32AoChnId[IN]: chn id
 *@param ps32Gain[OUT]: gain value
 *@return: 0 success, otherwise failed
 *@notes:
 */
AK_S32 AK_MPI_AO_GetGain(AK_S32 s32AoDevId, AK_S32 s32AoChnId, AK_S32 *ps32Gain);

/**
 *@brief AK_MPI_AO_SetMute - set ao mute to device
 *@param s32AoDevId[IN] : ao dev id
 *@param s32AoChnId[IN] : ao chn id
 *@param s32Enable[IN]  :enable to set mute 0:disable ,1:enable
 *@param pchReserve[IN] :reserve param
 *@return: 0 success, other failed
 *@notes:
 */
AK_S32 AK_MPI_AO_SetMute(AK_S32 s32AoDevId, AK_S32 s32AoChnId, AK_S32 s32Enable, \
    AK_CHAR *pchReserve);

/**
 *@brief AK_MPI_AO_SetMute - get ao mute to device
 *@param s32AoDevId[IN] : ao dev id
 *@param s32AoChnId[IN] : ao chn id
 *@param ps32Enable[OUT]: mute status, 0:disable ,1:enable
 *@param pchReserve[IN]:reserve param
 *@return: 0 success, other failed
 *@notes:
 */
AK_S32 AK_MPI_AO_GetMute(AK_S32 s32AoDevId, AK_S32 s32AoChnId, AK_S32 *ps32Enable, \
    AK_CHAR *pchReserve);

/**
 *@brief AK_MPI_AO_SetDigitalGain - set ao volume by aslc
 *@param s32AoDevId[IN]: dev id
 *@param s32AoChnId[IN]: chn id
 *@param s32DGain[IN]: aslc volume value, [-90, 20], 0 means no use aslc
 *@return: 0 success, otherwise failed
 *@notes:after the AK_MPI_AO_EnableChn
 */
AK_S32 AK_MPI_AO_SetDigitalGain(AK_S32 s32AoDevId, AK_S32 s32AoChnId, AK_S32 s32DGain);

/**
 *@brief AK_MPI_AO_GetDigitalGain - get ao volume by aslc
 *@param s32AoDevId[IN]: dev id
 *@param s32AoChnId[IN]: chn id
 *@param ps32DGain[OUT]: aslc volume value, [-90, 20], 0 means no use aslc
 *@return: 0 success, otherwise failed
 *@notes:
 */
AK_S32 AK_MPI_AO_GetDigitalGain(AK_S32 s32AoDevId, AK_S32 s32AoChnId, AK_S32 *ps32DGain);

/**
 *@brief AK_MPI_AO_EnableVqe - enable different vqe type
 *@param s32AoDevId[IN]: dev id
 *@param s32AoChnId[IN]: chn id
 *@param enVqeType[IN]: vqe type(nr agc eq hs aslc)
 *@return: 0 success, otherwise failed
 *@notes:
 */
AK_S32 AK_MPI_AO_EnableVqe(AK_S32 s32AoDevId, AK_S32 s32AoChnId, AUDIO_VQE_TYPE_E enVqeType);

/**
 *@brief AK_MPI_AO_DisableVqe - disable different vqe type
 *@param s32AoDevId[IN]: dev id
 *@param s32AoChnId[IN]: chn id
 *@param enVqeType[IN]: vqe type(nr agc eq hs aslc)
 *@return: 0 success, otherwise failed
 *@notes:
 */
AK_S32 AK_MPI_AO_DisableVqe(AK_S32 s32AoDevId, AK_S32 s32AoChnId, AUDIO_VQE_TYPE_E enVqeType);

/**
 *@brief AK_MPI_AO_SetVqeAttr - set vqe attr in different type
 *@param s32AoDevId[IN]   : dev id
 *@param s32AoChnId[IN]   : chn id
 *@param enVqeType[IN] : vqe type(nr agc eq hs aslc)
 *@param pstVqeAttr[IN]:vqe attr
 *@return: 0 success, otherwise failed
 *@notes:
 */
AK_S32 AK_MPI_AO_SetVqeAttr(AK_S32 s32AoDevId, AK_S32 s32AoChnId, AUDIO_VQE_TYPE_E enVqeType, \
    AUDIO_VQE_ATTR_S *pstVqeAttr);

/**
 *@brief AK_MPI_AO_GetVqeAttr - get vqe attr in different type
 *@param s32AoDevId[IN]   : dev id
 *@param s32AoChnId[IN]   : chn id
 *@param enVqeType[IN] : vqe type(nr agc eq hs aslc)
 *@param pstVqeAttr[OUT]:vqe attr
 *@return: 0 success, otherwise failed
 *@notes:
 */
AK_S32 AK_MPI_AO_GetVqeAttr(AK_S32 s32AoDevId, AK_S32 s32AoChnId, AUDIO_VQE_TYPE_E enVqeType, \
    AUDIO_VQE_ATTR_S *pstVqeAttr);


/**
 *@brief AK_MPI_AO_SendFrame - get vqe attr in different type
 *@param s32AoDevId[IN]        : dev id
 *@param s32AoChnId[IN]        : chn id
 *@param pu8SendPcmData[IN] : pointer data of send pcm
 *@param s32SendPcmLen[IN]  : len of send pcm unit:byte
 *@param ps32PlayPcmLen[OUT]: actually send to device length,unit:byte
 *@param s32TimeOut[IN]: s32TimeOut > 0,block mode;  s32TimeOut = 0, non block mode , unit is ms
 *@return: 0 success, otherwise failed
 *@notes:this function have timeout mode
 */
AK_S32 AK_MPI_AO_SendFrame(AK_S32 s32AoDevId, AK_S32 s32AoChnId,\
                AK_U8 *pu8SendPcmData, AK_S32 s32SendPcmLen,  \
                AK_S32 *ps32PlayPcmLen,  AK_S32 s32TimeOut);

/**
 *@brief AK_MPI_AO_Flush - push the remaining data out
 *@param s32AoDevId[IN] 	   : dev id
 *@param s32AoChnId[IN] 	   : chn id
 *@return: 0 success, otherwise failed
 *@notes:
 */
AK_S32 AK_MPI_AO_Flush(AK_S32 s32AoDevId, AK_S32 s32AoChnId);


/**
 *@brief AK_MPI_AO_SaveFile - save input file
 *@param s32AoDevId[IN] 	   : dev id
 *@param s32AoChnId[IN] 	   : chn id
 *@param pstSaveFileInfo[IN]: save file info
 *@return: 0 success, otherwise failed
 *@notes:
 */
AK_S32 AK_MPI_AO_SaveFile(AK_S32 s32AoDevId, AK_S32 s32AoChnId, \
        AUDIO_SAVE_FILE_INFO_S *pstSaveFileInfo);

/**
 *@brief AK_MPI_AO_GetBufStatus - get buf(vqe dev) status
 *@param s32AoDevId[IN] 	   : dev id
 *@param s32AoChnId[IN] 	   : chn id
 *@param pstBufStatus[OUT]	: get buf status
 *@return: 0 success, otherwise failed
 *@notes:
 */
AK_S32 AK_MPI_AO_GetBufStatus(AK_S32 s32AoDevId, AK_S32 s32AoChnId, DEV_BUF_STATUS_S *pstBufStatus);

/**
 *@brief AK_MPI_AO_SetFilterPrintLevel  : set print level of filter lib
 *@param u32PrintLevel[IN]    : print level, see ATCLIB_PRINT_LEVEL_E
 *@return 0 success
 */
AK_S32 AK_MPI_AO_SetFilterPrintLevel(AK_U32 u32PrintLevel);

/**
 *@brief AK_MPI_AO_SetDrvBufPeriods - set driver buffer periods
 *@param s32AoDevId[IN]        : dev id
 *@param enDrvBufType[IN]   : driver buffer type (ao ai loopback)
 *@param u32Periods[IN]    : periods's value
 *@return: 0 success,otherwise failed
 *@notes:
 */
AK_S32 AK_MPI_AO_SetDrvBufPeriods(AK_S32 s32AoDevId, AUDIO_DRV_BUF_TYPE_E enDrvBufType,\
                                AK_U32 u32Periods);

/**
 *@brief AK_MPI_AO_GetDrvBufPeriods - get driver buffer periods
 *@param s32AoDevId[IN]        : dev id
 *@param enDrvBufType[IN]   : driver buffer type (ao ai loopback)
 *@param u32Periods[OUT]    : periods's value
 *@return: 0 success,otherwise failed
 *@notes:
 */
AK_S32 AK_MPI_AO_GetDrvBufPeriods (AK_S32 s32AoDevId,
                                   AUDIO_DRV_BUF_TYPE_E enDrvBufType,
                                   AK_U32 *pu32Periods);

/**
*@brief AK_MPI_AO_SetThreadPriority - set priority of ao inner thread
*@param s32AoDevId[IN]		  : dev id
*@param enThreadType[IN]   : thread type
*@param s32Priority[IN]	  : priority's value
*@return: 0 success,otherwise failed
*@notes:
*/
AK_S32 AK_MPI_AO_SetThreadPriority(AK_S32 s32AoDevId, AUDIO_THREAD_E enThreadType,\
                            AK_S32 s32Priority);

/**
 *@brief AK_MPI_AO_GetThreadPriority - get priority of ao inner thread or loopback inner threads
 *@param s32AoDevId[IN]        : dev id
 *@param enThreadType[IN]   : thread type
 *@param ps32Priority[OUT]   : priority's value
 *@return: 0 success,otherwise failed
 *@notes:
 */
AK_S32 AK_MPI_AO_GetThreadPriority(AK_S32 s32AoDevId, AUDIO_THREAD_E enThreadType,\
                            AK_S32 *ps32Priority);

/**
 *@brief AK_MPI_AO_LoadConf  : load ao config (vqe etc)
 *@param s32AoDevId[IN] 		: ai dev id
 *@param s32AoChnId[IN] 		: ai chn id
 *@param pchConfFile[IN] 	  :  config file
 *@param pchSection[IN]	   :  which section
 *@return 0 success otherwise failed
 *@notes:
 */
AK_S32 AK_MPI_AO_LoadConf(AK_S32 s32AoDevId, AK_S32 s32AoChnId,
                          AK_CHAR *pchConfFile, AK_CHAR *pchSection);

/**
 *@brief AK_MPI_AO_LoadCmdLine  : load ao cmd line (vqe etc)
 *@param s32AoDevId[IN]         : ao dev id
 *@param s32AoChnId[IN]         : ao chn id
 *@param pchCmdLineStr[IN]       :  cmd line str
 *@param s32StrSize[IN]     :  str size
 *@return 0 success otherwise failed
 *@notes:
 */
AK_S32 AK_MPI_AO_LoadCmdLine(AK_S32 s32AoDevId, AK_S32 s32AoChnId, AK_CHAR *pchCmdLineStr, AK_S32 s32StrSize);

/**
 *@brief AK_MPI_AO_Dump   : Enable dump
 *@param s32AoDevId[IN]: Audio device handle id
 *@param s32AoChnId[IN]: Audio channle handle id
 *@param s32Enable[IN]    : Flag for enable dump, 0: disable, 1: enable
 *@return 0 success otherwise failed
 *@notes:
 */
AK_S32 AK_MPI_AO_Dump(AK_S32 s32AoDevId, AK_S32 s32AoChnId, AK_S32 s32Enable);

/**
 *@brief AK_MPI_AO_SetDumpCb - Set dump callback function
 *@param s32AoDevId[IN]: Audio device handle id
 *@param s32AoChnId[IN]: Audio channle handle id
 *@param cbDumpFun[IN]: Dump file callback function, for save the dump files
 *@return: 0 success, other failed
 *@notes:
 */
AK_S32 AK_MPI_AO_SetDumpCb(AK_S32 s32AoDevId, AK_S32 s32AoChnId, SET_DUMP_CALLBACK cbDumpFun);

/**
 *@brief AK_MPI_AO_ReadConf - Read params config
 *@param s32AoDevId[IN]         : ao dev id
 *@param s32AoChnId[IN]         : ao chn id
 *@param pchConfStrBuf[IN/OUT]: Config buffer
 *@param s32ConfStrBufLen[IN]: Config buffer length
 *@param ps32ConfStrLen[OUT]: Config string length
 *@return: 0 success, other failed
 *@notes:
 */
AK_S32 AK_MPI_AO_ReadConf(AK_S32 s32AoDevId, AK_S32 s32AoChnId,\
AK_CHAR *pchConfStrBuf, AK_S32 s32ConfStrBufLen, AK_S32 *ps32ConfStrLen);
#endif
/* end of file */
