/* SPDX-License-Identifier: Apache-2.0 */
/******************************************************************************

  Copyright (C), 2023-2026, Guangzhou Anyka Microelectronics Co., Ltd.

 ******************************************************************************
  File Name         	: ak_ai.h
  Version        		: V1.0.0
  Author         		: Anyka Linux SDK Team
  Created       		: 2023/9/15
  Description  		    : ai head file
  Example               :
  History       		:
  1.Date                : 2023/9/15
    Author              : 202107131457
    Modification        : Created file
******************************************************************************/

#ifndef _AK_AI_H_
#define _AK_AI_H_


#include "ak_common.h"
#include "ak_common_audio.h"

#define ADC_DEV_MAX_GAIN_MIC        (41)
#define ADC_DEV_MAX_GAIN_LINEIN     (1)
/**
 * ai mode error code
 */
typedef enum akAI_ERROR_TYPE_E
{
    ERROR_AI_OPEN_LIB_ERROR = (MODULE_ID_AI << 24) + 0,
    ERROR_AI_NR_NOT_OPEN,
    ERROR_AI_RESET_RB_ERROR,    // reset ring buffer error
    ERROR_AI_DEV_ID_NOT_SUPPORT,
    ERROR_AI_CHN_ID_NOT_SUPPORT,
    ERROR_AI_DEV_INFO_NULL,
    ERROR_AI_CHN_INFO_NULL,
    ERROR_AI_GAIN_ERROR,
    ERROR_AI_NOT_RUNNING,
    ERROR_AI_DEV_READ_ERROR,
    ERROR_AI_FRAME_SIZE_NOT_SET,
    ERROR_AI_DEV_ALREADY_OPEN,
    ERROR_AI_DRIVER_FAILED,
    ERROR_AI_MUTE_IS_ON,
    ERROR_AI_ASLC_ERROR,
    ERROR_AI_EQ_ERROR,
    ERROR_AI_NO_DATA,
    ERROR_AI_RB_NOT_ENOUGH_DATA,
    ERROR_AI_RB_READ_NOT_ENOUGH,
    ERROR_AI_DEV_INVALID_FD,
    ERROR_AI_CLOSE_FILTER_LIB_ERROR,
    ERROR_AI_THREAD_CREATE_FAILED,
    ERROR_AI_VQE_ERROR,
    ERROR_AI_PUB_ATTR_ERROR,
    ERROR_AI_CHN_HAS_BEEN_ENABLED,
    ERROR_AI_CHN_RUNNING,
    ERROR_AI_NOT_GET_FRAME,
    ERROR_AI_CONF_FILE_ERROR,
    ERROR_AI_CHN_NOT_ENABLE,
    ERROR_AI_STRING_TOO_BIG,
    ERROR_AI_DEV_NOT_OPEN,
    ERROR_AI_NOT_SET_PUBATTR
}AI_ERROR_TYPE_E;


/**
 * adc type of dev
 */
typedef enum akAUDIO_ADC_SOURCE_E
{
    AUDIO_ADC_SOURCE_MIC,        //mic
    AUDIO_ADC_SOURCE_LINEIN      //linein
} AUDIO_ADC_SOURCE_E;

/**
 * pdm config of dev
 */
typedef struct akAUDIO_PDM_CONFIG_S
{
    AK_S32 sReserved;
} AUDIO_PDM_CONFIG_S;

/**
 * adc config of dev
 */
typedef struct akAUDIO_ADC_CONFIG_S
{
    AUDIO_ADC_SOURCE_E enSourceType;           //adc type of dev
    AK_S32 sReserved;
} AUDIO_ADC_CONFIG_S;

/*audio param of ai*/

/**
 * audio param of ai, use to enable dev
 */
typedef struct akAUDIO_IN_PARAM_S
{
    AUDIO_SAMPLE_RATE_E     enSampleRate;       //sample rate
    AUDIO_SAMPLE_BIT_E      enSampleBits;       //sample bit not only support 16bit
    AUDIO_SOUND_MODE_E      enSoundMode;        //soundmode /mono/stero/queue
    AK_U32			        u32SrcCnt;          //source  count
    AK_U32			        u32ChnCnt;          //channel count
    union {
        AUDIO_ADC_CONFIG_S  stAdcConfig;        //adc config
        AUDIO_PDM_CONFIG_S  stPdmConfig;        //pdm config
    } unSrcConfig;
    AK_U32			        u32Reserved;
} AUDIO_IN_PARAM_S;


/**
 *@brief AK_MPI_AIGetVersion  : get ai module version
 *@return version
 *@notes:
 */
const AK_CHAR* AK_MPI_AI_GetVersion(AK_VOID);

/**
 *@brief AK_MPI_AIGetFilterVersion - get filter version of ai
 *@return: version string
 *@notes:
 */
const AK_CHAR *AK_MPI_AI_GetFilterVersion(AK_VOID);

/**
 *@brief AK_MPI_AI_GetVqeVersion - get vqe version
 *@return: version string
 *@notes:
 */
const AK_CHAR *AK_MPI_AI_GetVqeVersion(AK_VOID);

/**
 *@brief AK_MPI_AISetPubAttr  : set pub attr
 *@param s32AiDevId[IN]        : ai dev id
 *@param pstParam[IN]       : ai param
 *@return 0 success otherwise failed
 *@notes:
 */
AK_S32 AK_MPI_AI_SetPubAttr(AK_S32 s32AiDevId, AUDIO_IN_PARAM_S *pstParam);

/**
 *@brief AK_MPI_AIGetPubAttr  : get pub attr
 *@param s32AiDevId[IN]        : ai dev id
 *@param pstParam[OUT]       : ai param
 *@return 0 success otherwise failed
 *@notes:
 */
AK_S32 AK_MPI_AI_GetPubAttr(AK_S32 s32AiDevId, AUDIO_IN_PARAM_S *pstParam);

/**
 *@brief AK_MPI_AIEnable    : enable ai driver ,open dev node
 *@param s32AiDevId[IN]        : ai dev id
 *@return 0 success otherwise failed
 *@notes:
 */
AK_S32 AK_MPI_AI_Enable(AK_S32 s32AiDevId);

/**
 *@brief AK_MPI_AIDisable    : disable ai driver ,close AK_S32 node
 *@param s32AiDevId[IN]        : ai dev id
 *@return 0 success otherwise failed
 *@notes:
 */
AK_S32 AK_MPI_AI_Disable(AK_S32 s32AiDevId);

/**
 *@brief AK_MPI_AIEnableChn    : enable ai chn
 *@param s32AiDevId[IN]        : ai dev id
 *@param s32AiChnId[IN]        : ai chn id
 *@return 0 success otherwise failed
 *@notes:
 */
AK_S32 AK_MPI_AI_EnableChn(AK_S32 s32AiDevId, AK_S32 s32AiChnId);

/**
 *@brief AK_MPI_AIDisableChn    : diable ai chn
 *@param s32AiDevId[IN]        : ai dev id
 *@param s32AiChnId[IN]        : ai chn id
 *@return 0 success otherwise failed
 *@notes:
 */
AK_S32 AK_MPI_AI_DisableChn(AK_S32 s32AiDevId, AK_S32 s32AiChnId);

/**
 *@brief AK_MPI_AIEnableAecRefFrame    : enable to get aec frame
 *@param s32AiDevId[IN]        : ai dev id
 *@param s32AiChnId[IN]        : ai chn id
 *@param s32AoDevId[IN]        : ao dev id
 *@param s32AoChnId[IN]        : ao chn id
 *@return 0 success otherwise failed
 *@notes:
 */
AK_S32 AK_MPI_AI_EnableAecRefFrame(AK_S32 s32AiDevId, AK_S32 s32AiChnId,
                                   AK_S32 s32AoDevId, AK_S32 s32AoChnId);


/**
 *@brief AK_MPI_AISetGain   : set dev gain
 *@param s32AiDevId[IN]		  : ai dev id
 *@param s32AiChnId[IN]		  : ai chn id
 *@param s32Gain[IN]		  : dev gain
 *@return 0 success otherwise failed
 *@notes:
*/
AK_S32 AK_MPI_AI_SetGain(AK_S32 s32AiDevId, AK_S32 s32AiChnId, AK_S32 s32Gain);

/**
 *@brief AK_MPI_AIGetGain   : get dev gain
 *@param s32AiDevId[IN]        : ai dev id
 *@param s32AiChnId[IN]        : ai chn id
 *@param s32Gain[OUT]       : dev gain
 *@return 0 success otherwise failed
 *@notes:
 */
AK_S32 AK_MPI_AI_GetGain(AK_S32 s32AiDevId, AK_S32 s32AiChnId, AK_S32 *ps32Gain);

/**
 *@brief AK_MPI_AI_SetMute - set ai mute to device
 *@param s32AiDevId[IN]         : ai dev id
 *@param s32AiChnId[IN]         : ai chn id
 *@param s32Enable[IN]: enable to set mute 0:disable ,1:enable
 *@param pchReserve[IN]:reserve param
 *@return: 0 success, other failed
 *@notes:
 */
AK_S32 AK_MPI_AI_SetMute(AK_S32 s32AiDevId, AK_S32 s32AiChnId, AK_S32 s32Enable, AK_CHAR *pchReserve);

/**
 *@brief AK_MPI_AI_GetMute - get ai mute in device
 *@param s32AiDevId[IN]         : ai dev id
 *@param s32AiChnId[IN]         : ai chn id
 *@param ps32Enable[OUT]: enable to set mute 0:disable ,1:enable
 *@param pchReserve[OUT]:reserve param
 *@return: 0 success, other failed
 *@notes:
 */
AK_S32 AK_MPI_AI_GetMute(AK_S32 s32AiDevId, AK_S32 s32AiChnId, AK_S32 *ps32Enable, AK_CHAR *pchReserve);

/**
 *@brief AK_MPI_AISetDigitalGain  : set digital gain
 *@param s32AiDevId[IN]        : ai dev id
 *@param s32AiChnId[IN]        : ai chn id
 *@param s32DGain[IN]       : digital gain
 *@return 0 success otherwise failed
 *@notes:
 */
AK_S32 AK_MPI_AI_SetDigitalGain(AK_S32 s32AiDevId, AK_S32 s32AiChnId, AK_S32 s32DGain);

/**
 *@brief AK_MPI_AIGetDigitalGain   : get digital gain
 *@param s32AiDevId[IN]        : ai dev id
 *@param s32AiChnId[IN]        : ai chn id
 *@param s32DGain[OUT]      : digital gain
 *@return 0 success otherwise failed
 *@notes:
 */
AK_S32 AK_MPI_AI_GetDigitalGain(AK_S32 s32AiDevId, AK_S32 s32AiChnId, AK_S32 *ps32DGain);

/**
 *@brief AK_MPI_AIEnableVqe   : enable vqe (nr agc aec eq aslc)
 *@param s32AiDevId[IN]         : ai dev id
 *@param s32AiChnId[IN]         : ai chn id
 *@param enVqeType[IN]    :  vqe type
 *@return 0 success otherwise failed
 *@notes:
 */
AK_S32 AK_MPI_AI_EnableVqe(AK_S32 s32AiDevId, AK_S32 s32AiChnId, AUDIO_VQE_TYPE_E enVqeType);


/**
 *@brief AK_MPI_AIDisableVqe   : disable vqe (nr agc aec eq aslc)
 *@param s32AiDevId[IN]         : ai dev id
 *@param s32AiChnId[IN]         : ai chn id
 *@param enVqeType[IN]    :  vqe type
 *@return 0 success otherwise failed
 *@notes:
 */
AK_S32 AK_MPI_AI_DisableVqe(AK_S32 s32AiDevId, AK_S32 s32AiChnId, AUDIO_VQE_TYPE_E enVqeType);

/**
 *@brief AK_MPI_AISetVqeAttr   : set vqe attr(nr agc aec eq aslc)
 *@param s32AiDevId[IN]         : ai dev id
 *@param s32AiChnId[IN]         : ai chn id
 *@param enVqeType[IN]    :  vqe type
 *@param pstVqeAttr[IN]    :  vqe attr
 *@return 0 success otherwise failed
 *@notes:
 */
AK_S32 AK_MPI_AI_SetVqeAttr(AK_S32 s32AiDevId, AK_S32 s32AiChnId, AUDIO_VQE_TYPE_E enVqeType, \
    AUDIO_VQE_ATTR_S *pstVqeAttr);

/**
 *@brief AK_MPI_AIGetVqeAttr  : get vqe attr(nr agc aec eq aslc)
 *@param s32AiDevId[IN]         : ai dev id
 *@param s32AiChnId[IN]         : ai chn id
 *@param enVqeType[IN]       :  vqe type
 *@param pstVqeAttr[OUT]     :  vqe attr
 *@return 0 success otherwise failed
 *@notes:
 */
AK_S32 AK_MPI_AI_GetVqeAttr(AK_S32 s32AiDevId, AK_S32 s32AiChnId, AUDIO_VQE_TYPE_E enVqeType, \
    AUDIO_VQE_ATTR_S *pstVqeAttr);

/**
 *@brief AK_MPI_AIGetFrame   : get pcm frame
 *@param s32AiDevId[IN]        : ai dev id
 *@param s32AiChnId[IN]        : ai chn id
 *@param pstFrame[OUT]      : pcm frame struct
 *@param s32TimeOut[IN]: s32TimeOut > 0,block mode;  s32TimeOut = 0, non block mode unit is ms
 *@return 0 success otherwise failed
 *@notes:
 */
AK_S32 AK_MPI_AI_GetFrame(AK_S32 s32AiDevId, AK_S32 s32AiChnId, FRAME_S *pstFrame, AK_S32 s32TimeOut, \
    AK_S32 sReserved);

/**
 *@brief AK_MPI_AIReleaseFrame   : release frame
 *@param s32AiDevId[IN]        : ai dev id
 *@param s32AiChnId[IN]        : ai chn id
 *@param pstFrame[OUT]      : pcm frame
 *@return 0 success otherwise failed
 *@notes:
 */
AK_S32 AK_MPI_AI_ReleaseFrame(AK_S32 s32AiDevId, AK_S32 s32AiChnId, FRAME_S *pstFrame, AK_S32 sReserved);

/**
 *@brief AK_MPI_AIDumpFile   : enable dump file
 *@param s32AiDevId[IN]         : ai dev id
 *@param s32AiChnId[IN]         : ai chn id
 *@param s32Enable[IN]    : inner thread type(ai ao loopback)
 *@return 0 success otherwise failed
 *@notes:
 */
AK_S32 AK_MPI_AI_Dump(AK_S32 s32AiDevId, AK_S32 s32AiChnId, AK_S32 s32Enable);

/**
 *@brief AK_MPI_AISaveFile   : set ai file info
 *@param s32AiDevId[IN]         : ai dev id
 *@param s32AiChnId[IN]         : ai chn id
 *@param pstSaveFileInfo[IN]    : save file info
 *@return 0 success otherwise failed
 *@notes:
 */
AK_S32 AK_MPI_AI_SaveFile(AK_S32 s32AiDevId, AK_S32 s32AiChnId, AUDIO_SAVE_FILE_INFO_S *pstSaveFileInfo);

/**
 *@brief AK_MPI_AISetFilterPrintLevel  : set print level of filter lib
 *@param u32PrintLevel[IN]    : print level, see ATCLIB_PRINT_LEVEL_E
 *@return 0 success
 *@notes:
 */
AK_S32 AK_MPI_AI_SetFilterPrintLevel(AK_U32 u32PrintLevel);

/**
 *@brief AK_MPI_AISetDrvBufPeriods   : set driver buffer periods
 *@param s32AiDevId[IN]        : ai dev id
 *@param enDrvBufType[IN]   : driver type (ai loopback ao)
 *@param u32Periods[IN]       : driver buffer periods
 *@return 0 success otherwise failed
 *@notes:
 */
AK_S32 AK_MPI_AI_SetDrvBufPeriods(AK_S32 s32AiDevId, AUDIO_DRV_BUF_TYPE_E enDrvBufType, AK_U32 u32Periods);

/**
 *@brief AK_MPI_AIGetDrvBufPeriods   : get driver buffer periods
 *@param s32AiDevId[IN]        : ai dev id
 *@param enDrvBufType[IN]   : driver type (ai loopback ao)
 *@param pu32Periods[OUT]    : driver buffer periods
 *@return 0 success otherwise failed
 *@notes:
 */
AK_S32 AK_MPI_AI_GetDrvBufPeriods(AK_S32 s32AiDevId, AUDIO_DRV_BUF_TYPE_E enDrvBufType, AK_U32 *pu32Periods);

/**
 *@brief AK_MPI_AISetThreadPriority   : set inner thread priority value
 *@param s32AiDevId[IN]        : ai dev id
 *@param enThreadType       :  inner thread type(ai ao loopback)
 *@param s32Priority[IN]    : thread priority
 *@return 0 success otherwise failed
 *@notes:
 */
AK_S32 AK_MPI_AI_SetThreadPriority(AK_S32 s32AiDevId, AUDIO_THREAD_E enThreadType, AK_S32 s32Priority);

/**
 *@brief AK_MPI_AIGetThreadPriority   : get inner thread priority value of ai
 *@param s32AiDevId[IN]        : ai dev id
 *@param enThreadType[IN]    :  inner thread type(ai ao loopback)
 *@param ps32Priority[OUT]    : thread priority
 *@return 0 success otherwise failed
 *@notes:
 */
AK_S32 AK_MPI_AI_GetThreadPriority(AK_S32 s32AiDevId, AUDIO_THREAD_E enThreadType, AK_S32 *ps32Priority);

/**
 *@brief AK_MPI_AILoadConf   : load ai config (vqe etc)
 *@param s32AiDevId[IN]         : ai dev id
 *@param s32AiChnId[IN]         : ai chn id
 *@param pchConfFile[IN]     :  config file
 *@param pchSection[IN]      :  which section
 *@return 0 success otherwise failed
 *@notes:
 */
AK_S32 AK_MPI_AI_LoadConf(AK_S32    s32AiDevId,
                          AK_S32    s32AiChnId,
                          AK_CHAR   *pchConfFile,
                          AK_CHAR   *pchSection);

/**
 *@brief AK_MPI_AILoadCmdLine  : load ai cmd line (vqe etc)
 *@param s32AiDevId[IN]         : ai dev id
 *@param s32AiChnId[IN]         : ai chn id
 *@param pchCmdLineStr[IN]       :  cmd line str
 *@param s32StrSize[IN]     :  str size
 *@return 0 success otherwise failed
 *@notes:
 */
AK_S32 AK_MPI_AI_LoadCmdLine(AK_S32 s32AiDevId, AK_S32 s32AiChnId, AK_CHAR *pchCmdLineStr, AK_S32 s32StrSize);

/**
 *@brief AK_MPI_AISetFrmLen - set audio frame length, unit: byte
 *@param s32AiDevId[IN]         : ai dev id
 *@param s32AiChnId[IN]         : ai chn id
 *@param s32FrmLen[IN]: audio frame length
 *@return: 0 success, otherwise failed
 *@notes:
 */
AK_S32 AK_MPI_AI_SetFrmLen(AK_S32 s32AiDevId, AK_S32 s32AiChnId, AK_S32 s32FrmLen);

/**
 *@brief AK_MPI_AIGetFrmLen - get audio frame len, unit: ms
 *@param s32AiDevId[IN]         : ai dev id
 *@param s32AiChnId[IN]         : ai chn id
 *@param ps32FrmLen[OUT]: audio frame length
 *@return: audio frame interval, unit: ms, >0 success, otherwise failed
 *@notes:
 */
AK_S32 AK_MPI_AI_GetFrmLen(AK_S32 s32AiDevId, AK_S32 s32AiChnId, AK_S32 *ps32FrmLen);

/**
 *@brief AK_MPI_AISetDumpCb - Set dump file callback function
 *@param s32AiDevId[IN]         : ai dev id
 *@param s32AiChnId[IN]         : ai chn id
 *@param cbDumpFun[IN]: Dump file callback function, for save the dump files
 *@return: 0 success, other failed
 *@notes:
 */
AK_S32 AK_MPI_AI_SetDumpCb(AK_S32 s32AiDevId, AK_S32 s32AiChnId, SET_DUMP_CALLBACK cbDumpFun);

/**
 *@brief AK_MPI_AI_ReadConf - Read params config
 *@param s32AiDevId[IN]         : ai dev id
 *@param s32AiChnId[IN]         : ai chn id
 *@param pchConfStrBuf[IN/OUT]: Config buffer
 *@param s32ConfStrBufLen[IN]: Config buffer length
 *@param ps32ConfStrLen[OUT]: Config string length
 *@return: 0 success, other failed
 *@notes:
 */
AK_S32 AK_MPI_AI_ReadConf(AK_S32 s32AiDevId, AK_S32 s32AiChnId,\
    AK_CHAR *pchConfStrBuf, AK_S32 s32ConfStrBufLen, AK_S32 *ps32ConfStrLen);


#endif
/*end of file */
