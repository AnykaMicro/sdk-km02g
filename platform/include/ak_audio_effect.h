/* SPDX-License-Identifier: Apache-2.0 */
/******************************************************************************

  Copyright (C), 2023-2026, Guangzhou Anyka Microelectronics Co., Ltd.

 ******************************************************************************
  File Name         	: ak_audio_effect.h
  Version        		: V1.0.0
  Author         		: Anyka Linux SDK Team
  Created       		: 2023/9/15
  Description  		    : audio effect head file
  Example               :
  History       		:
  1.Date                : 2023/8/15
    Author              : 202107131457
    Modification        : Created file
******************************************************************************/

#ifndef _AK_AUDIO_EFFECT_H_
#define _AK_AUDIO_EFFECT_H_

#include "ak_common.h"
#include "ak_common_audio.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

/**
 * AUDIO_EFFECT USER MAX NUM
 */
#define AUDIO_EFFECT_USER_MAX_NUM       (10)

/**
 * filter algorithm support MAX NUM
 */
#define FILTER_ALGORITHM_MAX_MIC_NUM    (4)
/**
 * AUDIO_EFFECT ERROR CODE
 */

/* resample attr */
typedef struct akAUDIO_RES_ATTR_S {
    AUDIO_SAMPLE_RATE_E     enOutSampleRate;
} AUDIO_RES_ATTR_S;

/* vad attr*/
typedef struct akAUDIO_VAD_ATTR_S {
    AK_S32          s32Reserve1;//s32FrameLen;     // Notice: no use
    AK_U32          u32Probe;
    AK_S32          s32VadThresh; // threshold of signal amplitude. use the form of AK32Q15(x.xx) to set it
    AK_S32          s32VadLevel;  // sensitivity level.[0, 4],smaller is more sensible
    AK_S32          s32Reserve2;
} AUDIO_VAD_ATTR_S;

/* cry attr*/
typedef struct akAUDIO_CRY_ATTR_S {
    AK_S32          s32Reserve1;//s32FrameLen;    // Notice: no use
    AK_S32          s32CryThresh;
    AK_S32          s32CryLevel;
    AK_S32          s32Reserve2;
} AUDIO_CRY_ATTR_S;

typedef struct akAUDIO_AI_CRY_ATTR_S {
    AK_U16          u16FrameSize;    // samples. 0: use default
    AK_U16          u16FrameShift;   // samples. 0: use default
    AK_U16          u16FftSize;      // samples. 0: use default
    AK_U16          u16NBands;       // 0: use default
    AK_U16          u16NFrames;      // 0: use default
    AK_U32          u32LowFreq;      // 0: use default
    AK_U32          u32HighFreq;     // 0: use default
    AK_S8           *ps8ModelName;   // 0: use default mobilenetv1_aicrydet_opt
    AK_S32          s32Reserve;      //
    AK_U32          u32Probe;        // probe flag
    AK_S32          s32Reserve1;
    AK_S32          s32Level;        //cry detect level [0-5], 0: use default level 0
} AUDIO_AI_CRY_ATTR_S;

typedef struct akMIC_POSITION_S {
    AK_S32          x; // m. use the form of AK32Q15(x.xx)
    AK_S32          y; // m. use the form of AK32Q15(x.xx)
    AK_S32          z; // m. use the form of AK32Q15(x.xx)
} MIC_POSITION_S;

typedef struct akARRAY_GEOMETRY_S {
    AK_U32                  u32NumMic; // number of mic
    MIC_POSITION_S          astElement[FILTER_ALGORITHM_MAX_MIC_NUM];
} ARRAY_GEOMETRY_S;

typedef struct akAUDIO_SSL_ATTR_S {
    AK_U16                  u16FrameSize;   // frame size in bytes
    AK_U16                  u16Reserve1;//u16SoundSpeed;  // speed of sound // Notice: no use
    AK_U32                  u32Reserve2;//u32SrcNum;      // number of sound sources  // Notice: no use
    ARRAY_GEOMETRY_S        stArrayGeometry;
    AK_S32                  s32Reserve3;
} AUDIO_SSL_ATTR_S;

typedef struct akAUDIO_LSSL_ATTR_S {
    AK_U16                  u16FrameSize;   // frame size in bytes
    AK_U16                  u16Reserve;//u16SoundSpeed;  // speed of sound   // Notice: no use
    AK_U16                  u16NumMic;      // number of mic
    AK_U16                  u16MicSpace;
    AK_S32                  s32Thresh;
    AK_S32                  s32Level;
    AK_S32                  s32Reserve;
} AUDIO_LSSL_ATTR_S;

typedef struct akAUDIO_AI_KWS_ATTR_S {
    AK_U32                  u32AiKwsEna;     // enable AI keyword spotting
    AK_U16                  u16frameShift;   // Frame shift in samples. 0: use default
    AK_U16                  u16frameSize;   // Frame size in samples. 0: use default
    AK_U16                  u16FftSize;     // FFT size in samples. 0: use default
    AK_U16                  u16Frames;
    AK_U16                  u16Bands;
    AK_U16                  s16Reserve;
    AK_S8                   *ps8ModelName;  // AI keyword model name. 0: use default "model_aikws"
    AK_S32                  s32Reserve;
} AUDIO_AI_KWS_ATTR_S;

typedef struct akAUDIO_AI_KWSASR_ATTR_S {
    AK_U32                  u32AiKwsAsrEna;     // enable AI keyword spotting ASR
    AK_U16                  u16frameShift;   // Frame shift in samples. 0: use default
    AK_U16                  u16frameSize;   // Frame size in samples. 0: use default
    AK_U16                  u16FftSize;     // FFT size in samples. 0: use default
    AK_U16                  u16Frames;
    AK_U16                  u16Bands;
    AK_U16                  s16Reserve;
    AK_S8                   *ps8ModelName;  // AI keyword model name. 0: use default "model_aikws"
    AK_S32                  s32Reserve;
} AUDIO_AI_KWSASR_ATTR_S;

/**
 * AUDIO_EFFECT struct
 */
typedef struct akAUDIO_EFFECT_ATTR_S {
    union {
        AUDIO_RES_ATTR_S        stResAttr;
        AUDIO_ASLC_ATTR_S       stAslcAttr;
        AUDIO_EQ_ATTR_S         stEqAttr;
        AUDIO_VAD_ATTR_S        stVadAttr;
        AUDIO_CRY_ATTR_S        stCryAttr;
        AUDIO_AI_CRY_ATTR_S     stAiCryAttr;
        AUDIO_SSL_ATTR_S        stSslAttr;
        AUDIO_LSSL_ATTR_S       stLsslAttr;
        AUDIO_AI_KWS_ATTR_S     stAiKwsAttr;
        AUDIO_AI_KWSASR_ATTR_S  stAiKwsAsrAttr;
    } unEffectAttr;
} AUDIO_EFFECT_ATTR_S;

typedef enum akAUDIO_EFFECT_ERROR_TYPE_E {
    ERROR_AUDIO_EFFECT_OPEN_LIB_ERROR = (MODULE_ID_AUDIO_EFFECT << 24) + 0,
    ERROR_AUDIO_EFFECT_LIB_ERROR,
    ERROR_AUDIO_EFFECT_OUTBUF_LEN_ERROR,
    ERROR_AUDIO_EFFECT_NO_MATCH_VAD_TYPE,
    ERROR_AUDIO_EFFECT_RESULT_NO_RETURN,
} AUDIO_EFFECT_ERROR_TYPE_E;

/**
 * AUDIO_EFFECT support type list
 */
typedef enum akAUDIO_EFFECT_TYPE_E {
    AK_AUDIO_RESAMPLE       = 0,
    AK_AUDIO_VAD            = 1,
    AK_AUDIO_CRY            = 2,
    AK_AUDIO_ASLC           = 3,
    AK_AUDIO_EQ             = 4,
    AK_AUDIO_AI_CRY         = 5,
    AK_AUDIO_SSL            = 6,
    AK_AUDIO_LSSL           = 7,
    AK_AUDIO_AI_KWS         = 8,// AI Keyword Spotting
    AK_AUDIO_AI_KWSASR      = 9,// AI Keyword Spotting ASR
    AK_AUDIO_TYPE_NUM,
} AUDIO_EFFECT_TYPE_E;

/* data process result output*/
typedef struct akAUDIO_EFFECT_DATA_PROCESS_RESULT_S {
    AK_U8           *pu8OutBuf;
    AK_U32          u32OutDataLen;
    AK_S32          s32Reserve;
} AUDIO_EFFECT_DATA_PROCESS_RESULT_S;

/* vad result output*/
typedef struct akAUDIO_VAD_RESULT_S {
    AK_U64          u64TimeOffset;
    AK_S32          s32ProbedThresh;
} AUDIO_VAD_RESULT_S;

/* cry result output*/
typedef struct akAUDIO_CRY_RESULT_S {
    AK_U64          u64TimeOffset;
    AK_S32          s32Reserve;
} AUDIO_CRY_RESULT_S;

/* AI cry result output*/
typedef struct akAUDIO_AICRY_RESULT_S {
    AK_U64          u64TimeOffset;
    AK_S16          s16ProbValue;
} AUDIO_AICRY_RESULT_S;

typedef struct akAUDIO_AIKWS_RESULT_S {
    AK_U64          u64TimeOffset;
    AK_S16          s16ProbValue;
} AUDIO_AIKWS_RESULT_S;

typedef struct akAUDIO_AIKWSASR_RESULT_S {
    AK_U64          u64TimeOffset;
    AK_S16          s16ProbValue;
    AK_S16          s16KeyWordResult;
} AUDIO_AIKWSASR_RESULT_S;

// The convention used:
// azimuth: zero is to the right from the camera's perspective, with positive
//          angles in radians counter-clockwise.
// elevation: zero is horizontal, with positive angles in radians upwards.
// radius: distance from the camera in meters.
typedef struct akSOUND_SPHERICAL_DIRECTION {
    AK_S32      azimuth;   // Azimuth (rad) = this * pi/2. Use AK32Q15(x.xx) form
    AK_S32      elevation; // Elevation (rad) = this * pi/2. Use AK32Q15(x.xx) form
    AK_S32      radius;    // Distance (m). Use AK32Q15(x.xx) form
} SOUND_SPHERICAL_DIRECTION;

/* SSL result output*/
typedef struct akAUDIO_SSL_RESULT_S {
    AK_U64                      u64TimeOffset;
    AK_U32                      u32SrcNum;      // number of sound sources
    SOUND_SPHERICAL_DIRECTION   *pastSndSrcDir;
    AK_S32                      s32Reserve;
} AUDIO_SSL_RESULT_S;

/* LSSL result output*/
typedef struct akAUDIO_LSSL_RESULT_S {
    AK_U64                      u64TimeOffset;
    AK_U32                      u32SrcNum;      // number of sound sources
    SOUND_SPHERICAL_DIRECTION   *pastSndSrcDir;
    AK_S32                      s32Reserve;
} AUDIO_LSSL_RESULT_S;

typedef struct akSLList SLList;
struct akSLList { // Singly Linked List
    void            *data;
    SLList          *next;
};

typedef struct akAUDIO_VAD_RESULT_LIST_S {
    AUDIO_VAD_RESULT_S      *stResult;
    SLList                  *next;
} AUDIO_VAD_RESULT_LIST_S;

typedef struct akAUDIO_CRY_RESULT_LIST_S {
    AUDIO_CRY_RESULT_S      *stResult;
    SLList                  *next;
} AUDIO_CRY_RESULT_LIST_S;

typedef struct akAUDIO_AICRY_RESULT_LIST_S {
    AUDIO_AICRY_RESULT_S    *stResult;
    SLList                  *next;
} AUDIO_AICRY_RESULT_LIST_S;

typedef struct akAUDIO_SSL_RESULT_LIST_S {
    AUDIO_SSL_RESULT_S      *stResult;
    SLList                  *next;
} AUDIO_SSL_RESULT_LIST_S;

typedef struct akAUDIO_LSSL_RESULT_LIST_S {
    AUDIO_LSSL_RESULT_S      *stResult;
    SLList                  *next;
} AUDIO_LSSL_RESULT_LIST_S;

typedef struct akAUDIO_AIKWS_RESULT_LIST_S {
    AUDIO_AIKWS_RESULT_S    *stResult;
    SLList                  *next;
} AUDIO_AIKWS_RESULT_LIST_S;

typedef struct akAUDIO_AIKWSASR_RESULT_LIST_S {
    AUDIO_AIKWSASR_RESULT_S    *stResult;
    SLList                     *next;
} AUDIO_AIKWSASR_RESULT_LIST_S;

typedef union akRESULT_S {
    AUDIO_EFFECT_DATA_PROCESS_RESULT_S      stDataProcessResult;// RES / ASLC / EQ
    void                                    *pstDetResult;      // detect result list
} RESULT_S;

/**
 * AUDIO_EFFECT Result struct
 */
typedef struct akAUDIO_EFFECT_RESULT_S {
    AUDIO_EFFECT_TYPE_E                         u32EffectType;
    /* sizeof(AUDIO_EFFECT_DATA_PROCESS_RESULT_S) / sizeof(AUDIO_XXX_RESULT_S) */
    AK_U32                                      u32UnionBufSize;
    RESULT_S                                    unEffectResult;
} AUDIO_EFFECT_RESULT_S;


/**
 * AUDIO_EFFECT_REG struct
 */
typedef struct akAUDIO_EFFECT_REG_S {
    AUDIO_EFFECT_TYPE_E     enEffectType;

    AK_VOID* (*EffectOpen)(
        AK_VOID             *pstOpenArg,
        AK_VOID             *pstEffectAttr,
        AK_VOID             *pParamFactory
    );

    AK_S32 (*EffectClose)(AK_VOID *pstEffectHandle);

    AK_S32 (*EffectProcess)(
        AK_VOID             *pstEffectHandle,
        AK_VOID             *pstProcArg
    );

    AK_S32 (*EffectSetAttr)(
        AK_VOID             *pstEffectHandle,
        AK_VOID             *pstEffectAttr,
        AK_VOID             *pParamFactory
    );

    AK_S32 (*EffectGetAttr)(
        AK_VOID             *pstEffectHandle,
        AK_VOID             *pstEffectAttr);
} AUDIO_EFFECT_REG_S;

/**
 * @brief AK_MPI_AUDIO_EFFECT_GetVersion:get version string of AUDIO_EFFECT
 * @return: NULL failed.non 0: version string
 * @note can be called at any time
 */
const AK_CHAR* AK_MPI_AUDIO_EFFECT_GetVersion(AK_VOID);

/**
 * @brief AK_MPI_AUDIO_EFFECT_GetFilterVersion:get version string of Filter
 * @return: NULL failed.non 0: version string
 * @note:
 */
const AK_CHAR* AK_MPI_AUDIO_EFFECT_GetFilterVersion(AK_VOID);

/**
 * @brief AK_MPI_AUDIO_EFFECT_SetLibPrintLevel:set print level of the Filter
 * @param  u32PrintLevel[IN]:print level，for details ,please refer to ak_common_audio.h in ATCLIB_PRINT_LEVEL_E
 * @return: 0 success, otherwise failed
 * @note:
 */
AK_S32 AK_MPI_AUDIO_EFFECT_SetLibPrintLevel(AK_U32 u32PrintLevel);

/**
 * @brief AK_MPI_AUDIO_EFFECT_Register:regsister the funcitons of AUDIO_EFFECT.
 * @param :pstAudioEffect[IN] AUDIO_EFFECT_REG struct.
 * @return: 0 success, otherwise failed
 * @code
 * // regsister the funcitons of AUDIO_EFFECT
 * AUDIO_EFFECT_REG_S stAudioEffect = {
 *      AK_AUDIO_RESAMPLE,
 *      Effect_RES_Open,
 *      Effect_RES_Close,
 *      Effect_RES_Process,
 *      Effect_RES_SetAttr
 * };
 * AK_S32 s32Ret = AK_MPI_AUDIO_EFFECT_Register(*stAudioEffect);
 * if (s32Ret != 0) {
 *      AK_BASE_PrintErrorEx(MODULE_ID_AUDIO_EFFECT, "AK_MPI_AUDIO_EFFECT_Register error\n");
 *      return s32Ret;
 * }
 * @endcode
 * @note after called AK_MPI_AUDIO_EFFECT_Register, can call other funcions:
 * AK_MPI_AUDIO_EFFECT_Init()，
 * AK_MPI_AUDIO_EFFECT_DeInit()，
 * AK_MPI_AUDIO_EFFECT_Process()，
 */
AK_S32 AK_MPI_AUDIO_EFFECT_Register(AUDIO_EFFECT_REG_S *pstAudioEffect);

/**
 * @brief AK_MPI_AUDIO_EFFECT_Init:init AUDIO_EFFECT
 * @param  s32EffectUserId[IN] :user id.
 * @param  AUDIO_DATA_ATTR_S[IN] : audio base attr: enSampleRate . enSampleBits . enSoundMode .
 * @param  enEffectType[IN] : AUDIO_EFFECT TYPE
 * @param  pchFile[IN] : AUDIO_EFFECT config file path.
 * @param  pchSection[IN] : AUDIO_EFFECT config file section.
 * @return 0 success otherwise failed
 * @code
 * // init AUDIO_EFFECT
 * AK_S32 s32EffectUserIdRes = 0;
 * AUDIO_SAMPLE_RATE_E enSampleRate = AK_AUDIO_SAMPLE_RATE_8000;
 * AUDIO_SAMPLE_BIT_E enSampleBits = AK_AUDIO_SMPLE_BIT_16;
 * AUDIO_SOUND_MODE_E enSoundMode =AUDIO_SOUND_MODE_MONO;
 * AUDIO_EFFECT_TYPE_E enEffectTypeRes = AK_AUDIO_RESAMPLE;
 * AK_CHAR  pchFile = "/etc/config/audio_config.ini";
 * AK_CHAR  pchSection = "res_1";
 * AUDIO_EFFECT_RESULT_S stEffectResult = {0};
 *
 * AK_S32 s32Ret = AK_MPI_AUDIO_EFFECT_Init(s32EffectUserIdRes,
 *                              &stAudioDataAttr,
 *                              enEffectTypeRes,
 *                              pchFile,
 *                              pchSection );
 * if (s32Ret != 0) {
 *      AK_BASE_PrintErrorEx(MODULE_ID_AUDIO_EFFECT, "AK_MPI_AUDIO_EFFECT_Init error\n");
 *      return s32Ret;
 * }
 *
 * while (-- cnt > 0) {
 *       AK_MPI_AUDIO_EFFECT_Process( s32EffectUserIdRes,
 *                                   u8InBuf, u32InDataLen,
 *                                   &stEffectResult);
 * }
 *
 * s32Ret = AK_MPI_AUDIO_EFFECT_DeInit(s32EffectUserIdRes);
 * if (s32Ret != 0) {
 *      AK_BASE_PrintErrorEx(MODULE_ID_AUDIO_EFFECT, "AK_MPI_AUDIO_EFFECT_DeInit error\n");
 *      return s32Ret;
 * }
 * @endcode
 * @note
 */
AK_S32 AK_MPI_AUDIO_EFFECT_Init(AK_S32                  s32EffectUserId,
                                AUDIO_DATA_ATTR_S       *pstAudioDataAttr,
                                AUDIO_EFFECT_TYPE_E     enEffectType,
                                AK_CHAR                 *pchFile,
                                AK_CHAR                 *pchSection);

/**
 * @brief AK_MPI_AUDIO_EFFECT_DeInit:deinit AUDIO_EFFECT.
 * @param  s32EffectUserId[IN] : user id.
 * @return 0 success otherwise failed
 * @note 1.The AUDIO_EFFECT of the corresponding user id can be released \
 * only after the corresponding user id is initialized;
 * now AI_CRY model only support 16K sample rate.
 */
AK_S32 AK_MPI_AUDIO_EFFECT_DeInit(AK_S32 s32EffectUserId);

/**
 * @brief  AK_MPI_AUDIO_EFFECT_Process:perform sound processing or audio detection
 * @param s32EffectUserId[IN]: user id
 * @param pu8InBuf[IN]: pointer of input buf
 * @param u32InDataLen[IN]: length of input data
 * @param pstEffectResult[OUT]: pointer of output data
 * @return 0 success otherwise failed
 * @note
 */
AK_S32 AK_MPI_AUDIO_EFFECT_Process(AK_S32                  s32EffectUserId,
                                   AK_U8                   *pu8InBuf,
                                   AK_U32                  u32InDataLen,
                                   AUDIO_EFFECT_RESULT_S   *pstEffectResult);

/**
 * @brief  AK_MPI_AUDIO_EFFECT_LoadConf: set audio effect attr by load conf
 * @param s32EffectUserId[IN]: user id
 * @param enEffectType[IN]: audio effect type
 * @param  pchFile[IN] : AUDIO_EFFECT config file path.
 * @param  pchSection[IN] : AUDIO_EFFECT config file section.
 * @return 0 success otherwise failed
 * @code
 * // init AUDIO_EFFECT
 * AK_S32 s32EffectUserIdAslc = 0;
 * AUDIO_SAMPLE_RATE_E enSampleRate = AK_AUDIO_SAMPLE_RATE_8000;
 * AUDIO_SAMPLE_BIT_E enSampleBits = AK_AUDIO_SMPLE_BIT_16;
 * AUDIO_SOUND_MODE_E enSoundMode =AUDIO_SOUND_MODE_MONO;
 * AUDIO_EFFECT_TYPE_E enEffectTypeAslc = AK_AUDIO_ASLC;
 * AK_CHAR  pchFile = "/etc/config/audio_config.ini";
 * AK_CHAR  pchSection = "aslc_1";
 * AK_CHAR  pchSection2 = "aslc_2";
 * AUDIO_EFFECT_RESULT_S stEffectResult = {0};
 * EFFECT_ASLC_ATTR_S      stAslcAttr = {xxx};
 * AK_S32 s32Ret = AK_MPI_AUDIO_EFFECT_Init(s32EffectUserIdAslc,
 *                              &stAudioDataAttr,
 *                              enEffectTypeAslc,
 *                              pchFile,
 *                              pchSection );
 * if (s32Ret != 0) {
 *      AK_BASE_PrintErrorEx(MODULE_ID_AUDIO_EFFECT, "AK_MPI_AUDIO_EFFECT_Init error\n");
 *      return s32Ret;
 * }
 *
 * while(-- cnt > 0) {
 *       AK_MPI_AUDIO_EFFECT_Process( s32EffectUserIdAslc,
 *                                   u8InBuf, u32InDataLen,
 *                                   &stEffectResult);
 *
 *       AK_MPI_AUDIO_EFFECT_LoadConf( s32EffectUserIdAslc,
 *                                      AK_AUDIO_ASLC,
 *                                      pchFile,
 *                                      pchSection2 );
 * }
 *
 * s32Ret = AK_MPI_AUDIO_EFFECT_DeInit(s32EffectUserIdAslc);
 * if (s32Ret != 0) {
 *      AK_BASE_PrintErrorEx(MODULE_ID_AUDIO_EFFECT, "AK_MPI_AUDIO_EFFECT_DeInit error\n");
 *      return s32Ret;
 * }
 * @endcode
 * @note
 */
AK_S32 AK_MPI_AUDIO_EFFECT_LoadConf(AK_S32                  s32EffectUserId,
                                    AUDIO_EFFECT_TYPE_E     enEffectType,
                                    AK_CHAR                 *pchFile,
                                    AK_CHAR                 *pchSection);

/**
 * @brief  AK_MPI_AUDIO_EFFECT_Reset: reset the buf of LSSL
 * @param s32EffectUserId[IN]: user id
 * @return 0 success otherwise failed
 * @code
 * // init AUDIO_EFFECT
 * AK_S32 s32EffectUserIdLssl = 0;
 * AUDIO_SAMPLE_RATE_E enSampleRate = AK_AUDIO_SAMPLE_RATE_8000;
 * AUDIO_SAMPLE_BIT_E enSampleBits = AK_AUDIO_SMPLE_BIT_16;
 * AUDIO_SOUND_MODE_E enSoundMode =AUDIO_SOUND_MODE_MONO;
 * AUDIO_EFFECT_TYPE_E enEffectTypeLssl = AK_AUDIO_LSSL;
 * AK_CHAR  pchFile = "/etc/config/audio_config.ini";
 * AK_CHAR  pchSection = "lssl_1";
 * AUDIO_EFFECT_RESULT_S    stEffectResult = {0};
 * AK_S32 s32Ret = AK_MPI_AUDIO_EFFECT_Init(s32EffectUserIdLssl,
 *                              &stAudioDataAttr,
 *                              enEffectTypeLssl,
 *                              pchFile,
 *                              pchSection );
 * if (s32Ret != 0) {
 *      AK_BASE_PrintErrorEx(MODULE_ID_AUDIO_EFFECT, "AK_MPI_AUDIO_EFFECT_Init error\n");
 *      return s32Ret;
 * }
 *
 * while (-- cnt > 0) {
 *       AK_MPI_AUDIO_EFFECT_Process( s32EffectUserIdLssl,
 *                                   u8InBuf, u32InDataLen,
 *                                   &stEffectResult);
 *
 *       AK_MPI_AUDIO_EFFECT_Reset( s32EffectUserIdLssl );
 * }
 *
 * s32Ret = AK_MPI_AUDIO_EFFECT_DeInit(s32EffectUserIdLssl);
 * if (s32Ret != 0) {
 *      AK_BASE_PrintErrorEx(MODULE_ID_AUDIO_EFFECT, "AK_MPI_AUDIO_EFFECT_DeInit error\n");
 *      return s32Ret;
 * }
 * @endcode
 * @note
 */
AK_S32 AK_MPI_AUDIO_EFFECT_Reset(AK_S32 s32EffectUserId);

/**
 * set attr
 *
 * @param[IN] s32EffectUserId:    audio effect user id
 * @param[IN] enEffectType:       audio effect type
 * @param[IN] pstEffectAttr:      audio effect attr
 *
 * @retval 0 						SUCCESS
 * @retval non 0					FAILED
 *
 * @notice :     1. it only can be called after AK_MPI_AUDIO_EFFECT_Init
 *               2. some attr can not reset by SetAttr, such as RES_enOutSampleRate 、SSL_stArrayGeometry ....
 *                  becasue these attr relevancy some resource that already malloc
 */
AK_S32 AK_MPI_AUDIO_EFFECT_SetAttr( AK_S32                  s32EffectUserId,
                                    AUDIO_EFFECT_TYPE_E     enEffectType,
                                    AUDIO_EFFECT_ATTR_S     *pstEffectAttr );

/**
 * get attr
 *
 * @param[IN] s32EffectUserId:    audio effect user id
 * @param[IN] enEffectType:       audio effect type
 * @param[OUT] pstEffectAttr:      audio effect attr
 *
 * @retval 0 						SUCCESS
 * @retval non 0					FAILED
 *
 * @notice :     1. it only can be called after AK_MPI_AUDIO_EFFECT_Init
 */
AK_S32 AK_MPI_AUDIO_EFFECT_GetAttr(AK_S32                  s32EffectUserId,
                                   AUDIO_EFFECT_TYPE_E     enEffectType,
                                   AUDIO_EFFECT_ATTR_S     *pstEffectAttr);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* _AK_AUDIO_EFFECT_H_ */
