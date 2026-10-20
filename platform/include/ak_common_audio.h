/* SPDX-License-Identifier: Apache-2.0 */
/******************************************************************************

  Copyright (C), 2023-2026, Guangzhou Anyka Microelectronics Co., Ltd.

 ******************************************************************************
  File Name         	: ak_common_audio.h
  Version        		: V1.0.0
  Author         		: Anyka Linux SDK Team
  Created       		: 2023/8/4
  Description  		    : audio common head file
  Example               :
  History       		:
  1.Date                : 2023/8/4
    Author              : 202107131457
    Modification        : Created file
******************************************************************************/

#ifndef _AK_COMMON_AUDIO_H_
#define _AK_COMMON_AUDIO_H_

#include "ak_types.h"

#define AI_MAX_CHN_CNT                      (8)   // ai max channel count
#define AO_MAX_CHN_CNT                      (2)    // ao max channel count
#define CONF_FILE_NAME_STR_MAX_NUM	        (256)  // config file full name max len
#define CONF_FILE_SECTION_STR_MAX_NUM	    (64)   // config file section max len
#define MAX_STR_LEN                         (1024) // max str len
#define MAX_CONF_FILE_STR_LEN               (1024) // MAX_CONF_FILE_STR_LEN <= MAX_STR_LEN
#define MAX_CONF_SECTION_STR_LEN            (256)  // max section str len
#define EQ_MAX_BANDS                        (10)   // max eq bands cnt
#define MAX_AUDIO_FILE_PATH_LEN             (256)  // max audio file path len
#define MAX_AUDIO_FILE_NAME_LEN             (256)  // max audio file name len
#define DEF_DEV_BUF_LEN                     (512)  // dev buffer len
#define MAX_FRAME_LEN                       (8192) // max frame len in audio unit:byte
#define MIN_FRAME_LEN                       (320)  // min frame len in audio unit:byte

#define DMA_PERIODS_MIN                     (8)    // min periods of device driver buffer
#define DMA_PERIODS_MAX                     (80)   // max periods of device driver buffer
#define PRIORITY_MIN                        (1)    // min priority of inner thread
#define PRIORITY_MAX                        (99)   // max priority of inner thread

#define AO_DMA_PERIODS_MIN                  DMA_PERIODS_MIN   // min periods of ao dev driver buffer
#define AO_DMA_PERIODS_MAX                  DMA_PERIODS_MAX   // max periods of ao dev driver buffer
#define AO_DMA_PERIODS_DEF                  DMA_PERIODS_MIN   // default periods of ao dev driver buffer
#define AO_PRIORITY_MIN                     PRIORITY_MIN      // min priority of ao inner thread
#define AO_PRIORITY_MAX                     PRIORITY_MAX      // max priority of ao inner thread
#define AO_PRIORITY_DEF                     (50)              // default priority of ao inner thread

#define AI_DMA_PERIODS_MIN                  DMA_PERIODS_MIN   // min periods of ai dev driver buffer
#define AI_DMA_PERIODS_MAX                  DMA_PERIODS_MAX   // max periods of ai dev driver buffer
#define AI_DMA_PERIODS_DEF                  DMA_PERIODS_MIN   // default periods of ai dev driver buffer
#define AI_PRIORITY_MIN                     PRIORITY_MIN      // min priority of ai inner thread
#define AI_PRIORITY_MAX                     PRIORITY_MAX      // max priority of ai inner thread
#define AI_PRIORITY_DEF                     (50)              // default priority of ai inner thread

#define AEC_DMA_PERIODS_MIN                 DMA_PERIODS_MIN   // min periods of aec dev driver buffer
#define AEC_DMA_PERIODS_MAX                 DMA_PERIODS_MAX   // max periods of aec dev driver buffer
#define AEC_DMA_PERIODS_DEF                 DMA_PERIODS_MIN   // default periods of aec dev driver buffer
#define AEC_PRIORITY_MIN                    PRIORITY_MIN      // min priority of aec inner thread
#define AEC_PRIORITY_MAX                    PRIORITY_MAX      // max priority of aec inner thread
#define AEC_PRIORITY_DEF                    (50)              // default priority of aec inner thread

/* default vqe param */

/* aec lib param */
#define SYNC_COMPEN_STATUS                  (170)   // samples to compensate for far leading near

/* ai agc */
#define DEF_AI_AGC_LEVEL                    (24576) // auto gain control, default(0.75)*(1 << 15)
#define DEF_AI_AGC_MIN_GAIN                 (0)     // ai min gain value in agc
#define DEF_AI_AGC_MAX_GAIN                 (4)     // ai max gain value in agc
#define DEF_AI_NEAR_SENSITIVE               (40)    // ai near sensitive value in agc

/* ai nr */
#define DEF_AI_NOISE_SUP_DB                 (-40)   // noise suppress db

/* ai aec */
#define DEF_AEC_OUT_DIGI_GAIN               (1024)  // set digital gain after aec
#define DEF_AEC_IN_DIGI_GAIN                (1024)  // set  digital gain defore aec
#define DEF_AEC_TAIL                        (512)   // default 512

/* ai aslc */
#define DEF_AI_LIMIT                        (32768) // limit volume in ai
#define DEF_AI_DB                           (0)


//////////////////////////////////////
/* ao nr */
#define DEF_AO_NOISE_SUP_DB     (-40) // noise suppress db

/* ao aslc */
#define DEF_AO_LIMIT                        (32768) // limit volume in ao
#define DEF_AO_DB                           (0)

typedef enum {
    VQE_NEAR = 0,
    VQE_FAR = 1,
} VQE_PATH_E;
/* use to dumpfile cb*/
typedef AK_VOID (*SET_DUMP_CALLBACK)(AK_S32 item_id, AK_S32 size, AK_VOID *data,
                                     AK_VOID *meta, AK_VOID *cb_param);


typedef struct stWaveParam {
    AK_S32 s32Enable; // enable=1 start to save dump file, enable=0, not save dump file
    AK_S32 s32UserId; // vqe user id
    AK_S32 s32DevId;  // ai or ao device id
    AK_S32 s32ChnId;  // ai or ao channel id
} WAVE_CB_PARAM_S;

/*vqe type */
typedef enum akAUDIO_VQE_TYPE_E
{
  AUDIO_VQE_NR,
  AUDIO_VQE_AGC,
  AUDIO_VQE_AEC,
  AUDIO_VQE_EQ,
  AUDIO_VQE_ASLC,
  AUDIO_VQE_HS,
  AUDIO_VQE_MAX,
} AUDIO_VQE_TYPE_E;

/*buffer type of driver use to set driver buffer cnt*/
typedef enum akAUDIO_DRV_BUF_TYPE_E
{
    AUDIO_DRV_BUF_AI,
    AUDIO_DRV_BUF_LOOPBACK,
    AUDIO_DRV_BUF_AO
} AUDIO_DRV_BUF_TYPE_E;

/* audio channel type */
typedef enum akAUDIO_SOUND_MODE_E
{
    AUDIO_SOUND_MODE_QUEUE  = 0,   // after set it ,vqe can not take effect anymore,use to get raw data.
    AUDIO_SOUND_MODE_MONO   = 1,   // mono mode
    AUDIO_SOUND_MODE_STEREO = 2    // stereo mode
} AUDIO_SOUND_MODE_E;

    /*codec type*/
typedef enum akAUDIO_TYPE_E
{
    AK_AUDIO_TYPE_UNKNOWN,
    AK_AUDIO_TYPE_MIDI,
    AK_AUDIO_TYPE_MP3,
    AK_AUDIO_TYPE_AMR,
    AK_AUDIO_TYPE_AAC,
    AK_AUDIO_TYPE_WMA,      // 5
    AK_AUDIO_TYPE_PCM,
    AK_AUDIO_TYPE_ADPCM_IMA,
    AK_AUDIO_TYPE_ADPCM_MS,
    AK_AUDIO_TYPE_ADPCM_FLASH,
    AK_AUDIO_TYPE_APE,//10
    AK_AUDIO_TYPE_FLAC,
    AK_AUDIO_TYPE_OGG_FLAC,
    AK_AUDIO_TYPE_RA8LBR,
    AK_AUDIO_TYPE_DRA,
    AK_AUDIO_TYPE_OGG_VORBIS,// 15
    AK_AUDIO_TYPE_AC3,
    AK_AUDIO_TYPE_PCM_ALAW,
    AK_AUDIO_TYPE_PCM_ULAW,
    AK_AUDIO_TYPE_SBC,
    AK_AUDIO_TYPE_MSBC,// 20
    AK_AUDIO_TYPE_SPEEX,
    AK_AUDIO_TYPE_SPEEX_WB,
    AK_AUDIO_TYPE_OPUS,
    AK_AUDIO_TYPE_G722,
    AK_AUDIO_TYPE_VPWM, //25
    AK_AUDIO_TYPE_G729A,
    AK_AUDIO_TYPE_MAX
} AUDIO_TYPE_E;

/*set type of inner thread*/
typedef enum akAUDIO_THREAD_E
{
    AUDIO_THREAD_AI,
    AUDIO_THREAD_LOOPBACK,
    AUDIO_THREAD_AO
} AUDIO_THREAD_E;

/*set atc lib print level*/
typedef enum akATCLIB_PRINT_LEVEL_E
{
    // debug zones
    AUDIO_PRINT_LEVEL_ERROR    =  0x0001,
    AUDIO_PRINT_LEVEL_WARNING  =  0x0002,
    AUDIO_PRINT_LEVEL_INFO     =  0x0004,// print user information
    AUDIO_PRINT_LEVEL_FUNCTION =  0x0008,// print function in or out information
    AUDIO_PRINT_LEVEL_PARAM    =  0x0010,// print function param
    AUDIO_PRINT_LEVEL_TRACE    =  0x0020,// print trace information
    AUDIO_PRINT_LEVEL_OPEN     =  0x0040,// Open information
    AUDIO_PRINT_LEVEL_CLOSE    =  0x0080,// Close information
    AUDIO_PRINT_LEVEL_EVENT    =  0x0100,// print user receive event
    AUDIO_PRINT_LEVEL_VERBOSE  =  0x8000,
    AUDIO_PRINT_LEVEL_DEFAULT  =  0x10000
} ATCLIB_PRINT_LEVEL_E;

/* type of samplerate */
typedef enum akAUDIO_SAMPLE_RATE_E
{
    AK_AUDIO_SAMPLE_RATE_8000  = 8000,
    AK_AUDIO_SAMPLE_RATE_11025 = 11025,
    AK_AUDIO_SAMPLE_RATE_12000 = 12000,
    AK_AUDIO_SAMPLE_RATE_16000 = 16000,
    AK_AUDIO_SAMPLE_RATE_22050 = 22050,
    AK_AUDIO_SAMPLE_RATE_24000 = 24000,
    AK_AUDIO_SAMPLE_RATE_32000 = 32000,
    AK_AUDIO_SAMPLE_RATE_44100 = 44100,
    AK_AUDIO_SAMPLE_RATE_48000 = 48000,
    AK_AUDIO_SAMPLE_RATE_64000 = 64000,// now not support
    AK_AUDIO_SAMPLE_RATE_96000 = 96000// 37E support
} AUDIO_SAMPLE_RATE_E;

/*type of dev buffer size*/
typedef enum akAUDIO_DEV_BUF_SIZE_E
{
    AK_AUDIO_DEV_BUF_SIZE_256  = 256,// now not support
    AK_AUDIO_DEV_BUF_SIZE_512  = 512,
    AK_AUDIO_DEV_BUF_SIZE_1024 = 1024,
    AK_AUDIO_DEV_BUF_SIZE_2048 = 2048,
    AK_AUDIO_DEV_BUF_SIZE_3072 = 3072,
    AK_AUDIO_DEV_BUF_SIZE_4096 = 4096
} AUDIO_DEV_BUF_SIZE_E;

/* sample bit */
typedef enum akAUDIO_SAMPLE_BIT_E
{
    AK_AUDIO_SMPLE_BIT_16 = 16,
    AK_AUDIO_SMPLE_BIT_24 = 24 // now not support
} AUDIO_SAMPLE_BIT_E;

/*audio stream use to aenc or adec*/
typedef struct akAUDIO_STREAM_S
{
    AK_U8       *pu8Data;     //stream data
    AK_U32      u32Len;        //stream len in bytes
    AK_U64      u64Ts;   //timestamp(ms)
    AK_U64      u64SeqNo;    //current stream sequence no according to frame
} AUDIO_STREAM_S;

/* eq attr */
typedef struct akAUDIO_EQ_ATTR_S
{
    AK_S16		s16PreGain;                 // [-12288, 12288]
    AK_U32		u32Bands;                    //1~10
    AK_U32		au32BandFreqs[EQ_MAX_BANDS];
    AK_S16		as16BandGains[EQ_MAX_BANDS];  // [-32768, 0) U (0, 32768]
    AK_U16		au16BandQ[EQ_MAX_BANDS];     // [0, 65536)
/* HPF(hight pass filter);HSF(high shelf filter;LPF(low pass filter);\
LSF(low shelf filter);PF(peaking filter)*/
    AK_U8		au8BandTypes[EQ_MAX_BANDS];
    AK_U8		u8SmoothEna;               // deprecated and no longer in use
    AK_U16		u16SmoothTime;              // deprecated and no longer in use
    AK_U8		u8DcRmEna;                // deprecated and no longer in use
    AK_U32		u32Dcfb;                   // deprecated and no longer in use
    AK_U8		u8AslcEna;               // deprecated and no longer in use
    AK_U16		u16AslcLevelMax;         // deprecated and no longer in use
    AK_U8		u8Enable;                   // 0: disable, 1: enable
    AK_U8		au8BandEnable[EQ_MAX_BANDS];
} AUDIO_EQ_ATTR_S;

/* eq type */
typedef enum akAUDIO_EQ_TYPE_E
{
    TYPE_NO ,
    TYPE_HPF ,
    TYPE_LPF ,
    TYPE_HSF ,
    TYPE_LSF ,
    TYPE_PF1    //PeaKing filter
} AUDIO_EQ_TYPE_E;

/* audio data attr */
typedef struct akAUDIO_DATA_ATTR_S
{
    AUDIO_SAMPLE_RATE_E enSampleRate;
    AUDIO_SAMPLE_BIT_E  enSampleBits;
    AUDIO_SOUND_MODE_E  enSoundMode;
} AUDIO_DATA_ATTR_S;

/* info of save file ,use to set fill info*/
typedef struct akAUDIO_SAVE_FILE_INFO_S
{
    AK_S32          s32Enable;    //enable
    AK_CHAR         achFilePath[MAX_AUDIO_FILE_PATH_LEN];  //file path
    AK_CHAR         achFileName[MAX_AUDIO_FILE_NAME_LEN];  //file name
    AK_S32          s32SaveSecond;// unit: second, value in [0-180]second
} AUDIO_SAVE_FILE_INFO_S;

/* aec param*/
typedef struct akAUDIO_AEC_PARAM_S
{
    /* now no use this param! max amplitude of far signal, 1~32767, \
    0: use default. default is 9830*/
    //AK_U64   u64AudioOutThreshold;
    /* digtal gain for far signal,1~1024, 0: use default. default is 1024*/
    AK_S16          s16AudioOutDigiGain;
    /*digtal gain for near signal, 1~1024, 0: use default. default is 1024*/
    AK_S16          s16AudioInDigiGain;
    //AK_S32 s32Reserve;
    AK_U16          u16Tail;
    AK_S32          s32Enable;
    AK_S32          s32AudioFarBreakdownThresh;
    AK_S32          s32Crc;
    AK_S32          s32Reserve;
} AUDIO_AEC_PARAM_S;
/* aec attr*/
typedef struct akAUDIO_AEC_ATTR_S
{
    AUDIO_AEC_PARAM_S   stAecParam;
    AK_S32              s32AoDevId;
    AK_S32              s32AoChnId;
} AUDIO_AEC_ATTR_S;

/* agc attr*/
typedef struct akAUDIO_AGC_ATTR_S
{
    AK_U64      u64AgcLevel;          // 1~32767, 0: use default.default is 24576
    AK_S16      s16AgcMaxGain;       // agc's max_gain, 2~8, default is 4
    AK_S16      s16AgcMinGain;       // agc's min_gain, default is 102
    /* sensitivity of near-end speech [1:100], 0: use default, default is 20 */
    AK_S16      s16NearSensitivity;
    AK_S32      s32Crc;
    AK_S32      s32Enable;
} AUDIO_AGC_ATTR_S;

/*nr attr*/
typedef struct akAUDIO_NR_ATTR_S
{
/*attenuation of noise in dB (negative number),0: use default, default is -40*/
    AK_S16          s16NoiseSuppressDb;
    AK_S32          s32Crc;
    AK_S32          s32Enable;  //0:diable 1:enable
} AUDIO_NR_ATTR_S;

/*aslc attr*/
typedef struct akAUDIO_ASLC_ATTR_S
{
/* max amplitude of samples,1~32767(Q15), 0: use default. use AK32Q15*/
    AK_S32          s32Limit;
/*automatic sound volume control,aslc_db is for the voluume,unit db,-90~20db*/
    AK_S32          s32AslcDb;
    AK_S32          s32Crc;
} AUDIO_ASLC_ATTR_S;

typedef struct akAUDIO_HS_INFO_S// the same as T_ECHO_HOWLING_INFO
{
    AK_U32   u32HowlingFreq;  // howling frequency
    // trigger of howling event, 1: howling freq arises, 0: howling freq falls
    AK_CHAR  achTrigger;
    AK_CHAR  achHowlingFreq; // number of howling frequencies still active
    AK_U64   ts;           // timestamp of the event
    // b[0]: user set it to disable internal global volume control strategy. default: enable
    AK_U32   u32Flags;
} AUDIO_HS_INFO_S;

/**
 *ePath: Near part or far part
 *s32DevId: AI device id or AO device id
 *s32ChnId: AI channel id or AO channel id
 *pstHsInfo: AI hs information or AO hs information
 */
typedef AK_S32 (*HS_PROCESS_FUN)(VQE_PATH_E ePath, AK_S32 s32DevId,
                                 AK_S32 s32ChnId, AUDIO_HS_INFO_S *pstHsInfo);

/* hs attr */
typedef struct akAUDIO_HS_ATTR_S
{
    AK_S32              s32Crc;
    AK_S32              s32Enable;
    HS_PROCESS_FUN      pHsCb;
    AK_S16              s16PtprThr; // PTPR detection threshold value
    AK_S32              s32Reserve;
} AUDIO_HS_ATTR_S;

/*dev id of ai*/
typedef enum akAUDIO_AI_DEV_ID_E
{
    DEV_ADC     = 0,
    DEV_PDM     = 8,
    DEV_I2S0_AI = 16,   //now not support
    DEV_I2S1_AI = 17,   //now not support
    DEV_AI_MAX  = 20
} AUDIO_AI_DEV_ID_E;

/*dev id of ao*/
typedef enum akAUDIO_AO_DEV_ID_E
{
    DEV_DAC     = 0,
    DEV_I2S0_AO = 4,//now not support
    DEV_AO_MAX  = 8
} AUDIO_AO_DEV_ID_E;

/*vqe attr of audio*/
typedef struct akAUDIO_VQE_ATTR_S
{
  union {
      AUDIO_NR_ATTR_S	stNrAttr;
      AUDIO_AGC_ATTR_S	stAgcAttr;
      AUDIO_AEC_ATTR_S	stAecAttr;
      AUDIO_EQ_ATTR_S	stEqAttr;
      AUDIO_ASLC_ATTR_S stAslcAttr;
      AUDIO_HS_ATTR_S   stHsAttr;
  } unVqeAttr;
} AUDIO_VQE_ATTR_S;

/* dev buffer status */
typedef struct akDEV_BUF_STATUS_S
{
    AK_S32 s32DevBufTotalLen;  // ai or ao device buffer total length, unit: byte
    AK_S32 s32DevBufRemainLen; // ai or ao device buffer remain length, unit: byte
    AK_S64 s64DevFillZeroLen;  // ao device fill zero len unit: byte
    AK_S32 s32VqeBufTotalLen;  // ai or ao vqe buffer total length unit: byte
    AK_S32 s32VqeBufRemainLen; // ai or ao vqe buffer remain length unit: byte
    AK_S32 s32Reserve;
} DEV_BUF_STATUS_S;

/* check samplerate param */
static inline AK_S32 AudioCheckSampleRate(AK_S32 s32SampleRate)
{
    if (s32SampleRate != AK_AUDIO_SAMPLE_RATE_8000  &&
        s32SampleRate != AK_AUDIO_SAMPLE_RATE_12000 &&
        s32SampleRate != AK_AUDIO_SAMPLE_RATE_11025 &&
        s32SampleRate != AK_AUDIO_SAMPLE_RATE_16000 &&
        s32SampleRate != AK_AUDIO_SAMPLE_RATE_22050 &&
        s32SampleRate != AK_AUDIO_SAMPLE_RATE_24000 &&
        s32SampleRate != AK_AUDIO_SAMPLE_RATE_32000 &&
        s32SampleRate != AK_AUDIO_SAMPLE_RATE_44100 &&
        s32SampleRate != AK_AUDIO_SAMPLE_RATE_48000 &&
        s32SampleRate != AK_AUDIO_SAMPLE_RATE_96000) {
        /* sample rate not support  */
        return -1;
    }
    return 0;
}
/* check samplerate bit */
static inline AK_S32 AudioCheckSampleBit(AK_S32 s32SampleBit)
{
    if (s32SampleBit != AK_AUDIO_SMPLE_BIT_16) {
        return -1;
    }
    return 0;
}

/* check samplerate soundmode */
static inline AK_S32 AudioCheckSoundMode(AK_S32 s32SoundMode)
{
    if (s32SoundMode != AUDIO_SOUND_MODE_MONO   &&
        s32SoundMode != AUDIO_SOUND_MODE_STEREO &&
        s32SoundMode != AUDIO_SOUND_MODE_QUEUE) {
        return -1;
    }
    return 0;
}

/**
 *@brief AudioLenToInterval - calculate audio data ts interval, unit ms
 *@param pcm_attr[IN]: audio pcm attribute, include sample rate, sample bit, channel number
 *@param data_len[IN]: audio data length, unit byte
 *@return: audio data ts interval
 *@note:
 */
static inline AK_U32 AudioLenToInterval(AUDIO_SAMPLE_RATE_E enSampleRate,
                                            AUDIO_SAMPLE_BIT_E enSampleBits,
                                            AUDIO_SOUND_MODE_E enSoundMode,
                                            AK_U32 u32DataLen)
{
    /*   time(ms) = data_len  / (sample rete* channel num *sample bit /8 / 1000)  */
    /* 8 is bit to byte */
    /* 1000 is second to ms */
    /* dev->frame_interval is us */

    AK_U32 sample_bytes = (enSampleBits >> 3);
    AK_U32 sec_bytes = (enSampleRate * sample_bytes * enSoundMode);

    if (0 == sec_bytes)
        sec_bytes = 8000 * (16 >> 3) * 1;

    return ((1000 * u32DataLen) / sec_bytes);
}

/**
 *@brief AudioIntervalToLen - calculate the size of pcm
 *@param pcm_attr[IN]: audio pcm attribute, include sample rate, sample bit, channel number
 *@param interval[IN]: audio data ts interval, unit ms
 *@return: pcm size, unit byte
 *@note:
 */
static inline AK_U32 AudioIntervalToLen(AUDIO_SAMPLE_RATE_E enSampleRate,
                                            AUDIO_SAMPLE_BIT_E enSampleBits,
                                            AUDIO_SOUND_MODE_E enSoundMode,
                                            AK_S32 interval)
{
    AK_U32 sample_bytes = (enSampleBits >> 3);
    /* size = sample rete* channel num * time(ms) *sample bit /8 / 1000 */
    /* 8 is bit to byte */
    /* 1000000 is second to ms */
    /* dev->frame_interval is ms */
    AK_U32 size = (enSampleRate * enSoundMode * \
    interval * sample_bytes ) / 1000;
    if (size & 1) {
        size++;
    }

    return size;
}

/**
 *@brief AudioStereoToMono: copy frome double channel to single channel
 *@param dest[IN]: single channel data
 *@param src[IN]: double channel data
 *@param src_len[IN]: double channel data length
 *@return: AK_VOID
 *@note:
 */
static inline AK_VOID AudioStereoToMono(AK_VOID *dest, AK_VOID *src, AK_S32 src_len)
{
    AK_S32 i;

    for (i=0; i<src_len/4; i++) {
        ((AK_S16*)dest)[i] = ((AK_S16*)src)[2*i];
    }
}

/**
 *@brief AudioMonoToStereo: copy frome single channel to double channel
 *@param src[IN]: single channel data
 *@param src_len[IN]: single channel data length
 *@param dest[IN]: double channel data
 *@return: AK_VOID
 *@note:
 */
static inline AK_VOID AudioMonoToStereo(const AK_U8 *src, \
    AK_S32 s32SrcLen, AK_U8 *dest)
{
    AK_S32 i = 0;
    AK_S32 buf_size = s32SrcLen << 1;

    for (i=buf_size/4; i>0; i--) {
        ((AK_S16*)dest)[2*i-1] = ((AK_S16*)src)[i-1];
        ((AK_S16*)dest)[2*i-2] = ((AK_S16*)src)[i-1];
    }
}

/**
 *@brief AudioGetDevBufLen: get ai or ao device buffer length
 *@param sample_rate[IN]: audio sample rate
 *@return: u32DevBufLen
 *@note:
 */
static inline AK_U32 AudioGetDevBufLen(AK_S32 s32SampleRate)
{
    AK_U32 u32DevBufLen = 0;

    if (AK_AUDIO_SAMPLE_RATE_16000 > s32SampleRate)// 8000 --512
        u32DevBufLen = DEF_DEV_BUF_LEN;
    else if (AK_AUDIO_SAMPLE_RATE_32000 > s32SampleRate)// 16000~22050 -1024
        u32DevBufLen = DEF_DEV_BUF_LEN << 1;
    else if (AK_AUDIO_SAMPLE_RATE_48000 >= s32SampleRate)
        u32DevBufLen = DEF_DEV_BUF_LEN << 2;// 32000~48000-2048
    else
        u32DevBufLen = DEF_DEV_BUF_LEN << 3;// 96000-4096
    return u32DevBufLen;
}

/**
 *@brief AudioGetMinFrameLen: get ai or ao device buffer length
 *@param sample_rate[IN]: audio sample rate
 *@return: u32DevMinBufLen
 *@note:
 */
static inline AK_U32 AudioGetMinFrameLen(AK_S32 s32SampleRate, \
AUDIO_SAMPLE_BIT_E enSampleBits, AUDIO_SOUND_MODE_E enSoundMode)
{
    AK_U32 u32DevMinBufLen = 0;

    if (AK_AUDIO_SAMPLE_RATE_16000 > s32SampleRate) {// 8000 --320
        u32DevMinBufLen = MIN_FRAME_LEN;
    } else if (AK_AUDIO_SAMPLE_RATE_32000 > s32SampleRate)// 16000~22050 -640
        u32DevMinBufLen = MIN_FRAME_LEN << 1;
    else if (AK_AUDIO_SAMPLE_RATE_48000 >= s32SampleRate)
        u32DevMinBufLen = MIN_FRAME_LEN << 2;// 32000~48000-1280
    else
        u32DevMinBufLen = MIN_FRAME_LEN << 3;// 96000-2560

    if (AUDIO_SOUND_MODE_MONO != enSoundMode) {
        u32DevMinBufLen = u32DevMinBufLen << 1;
    }

    return u32DevMinBufLen;
}

/**
 *@brief AudioGetMaxFrameLen: get ai or ao device buffer length
 *@param sample_rate[IN]: audio sample rate
 *@return: u32DevMaxBufLen
 *@note:
 */
static inline AK_U32 AudioGetMaxFrameLen(AK_S32 s32SampleRate, \
AUDIO_SAMPLE_BIT_E enSampleBits, AUDIO_SOUND_MODE_E enSoundMode)
{
    AK_U32 u32DevMaxBufLen = 0;

    if (AK_AUDIO_SAMPLE_RATE_48000 >= s32SampleRate)
        u32DevMaxBufLen = MAX_FRAME_LEN >> 1;// 8000~48000- 4096
    else
        u32DevMaxBufLen = MAX_FRAME_LEN ;// 96000-8192

    if (AUDIO_SOUND_MODE_MONO != enSoundMode) {
        u32DevMaxBufLen =  u32DevMaxBufLen << 1;
    }
    return u32DevMaxBufLen;
}

// register audio module into the MPI subsystem
AK_VOID  AK_MPI_AUDIO_RegisterAIDenoise(AK_VOID);

#endif

/* end of file */
