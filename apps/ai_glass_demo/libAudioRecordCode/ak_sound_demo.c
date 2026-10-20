/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd. */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <errno.h>

#include "ak_sound_demo.h"
#include "ai_glass_common.h"

#include "ak_common.h"
#include "ak_log.h"
#include "ak_thread.h"

#include "ak_common_audio.h"
#include "ak_ai.h"
#include "ak_aenc.h"
#include "aenc_mp3.h"
#include "aenc_aac.h"
#include "aenc_amr.h"
#ifdef AI_DENOISE_ENABLE
#include "ak_npu.h"
#endif

#define SAVE_FILE_FLAG
#define SAVE_PATH       "/mnt/record_audio_data.mp3"

#define SAMPLE_BITS     (AK_AUDIO_SMPLE_BIT_16)
#define SAMPLE_RATE     (AK_AUDIO_SAMPLE_RATE_16000)
#define SOUND_MODE      (AUDIO_SOUND_MODE_MONO)
#define SOUND_ENC_TYPE  (AK_AUDIO_TYPE_MP3)

// #define SAMPLE_BITS (AK_AUDIO_SMPLE_BIT_16)
// #define SAMPLE_RATE (AK_AUDIO_SAMPLE_RATE_48000)
// #define SOUND_MODE (AUDIO_SOUND_MODE_STEREO)
// #define SOUND_ENC_TYPE (AK_AUDIO_TYPE_AAC)

//audio
static AK_CHAR *spchConfFile        = "/etc/config/audio_config.ini";   // config path
static AK_CHAR *spchConfSection     = "def_near";                       // config section
static AK_S32 s32FrameInterval      = 32;
static AK_S32 s32AiChnId            = 0;                                //ai chn id
static AK_S32 s32AiDevId            = DEV_ADC;                          //ai dev id
static AUDIO_IN_PARAM_S stAiParam   = {0};
static AK_S32 s32Gain               = 30;
static AK_S32 s32AencUserId         = 0;
static AK_S32 s32Flag               = 0;
static AK_PTHREAD_T encode_tid;
static AK_PTHREAD_T get_tid;
static unsigned char g_save_file_path_name[256] = {0};

// Recording duration control related variables
static unsigned int g_record_sec = 0;          // Total recording duration (seconds)
static unsigned int g_record_min_sec = 0;      // Minimum recording duration (seconds)
static struct timeval g_record_start_time;     // Recording start time
static AK_S32 g_auto_stop_flag = 0;            // Auto stop flag
static AK_S32 g_manual_stop_flag = 0;          // Manual stop flag
static AK_S32 g_recording_finished = 0;        // Recording finished flag

#ifdef AI_DENOISE_ENABLE
/**
 * InitAIDenoise - when use AIDenoise, init npu
 * notes:
 */
static AK_S32 InitAIDenoise(AK_VOID)
{
    AK_MPI_AUDIO_RegisterAIDenoise();
    int s32Ret = AK_MPI_NPU_Init();
    if (s32Ret) {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "ERROR: AK_MPI_NPU_Init,  s32Ret = %d\n", s32Ret);
        return AK_FAILED;
    }
    return AK_SUCCESS;
}

/**
 * DestroyAIDenoise - close AIDenoise
 * notes:
 */
static AK_VOID DestroyAIDenoise(AK_VOID)
{
    AK_MPI_NPU_Deinit();
}
#endif


/**
 * GetStream - get stream from encoder
 * @arg[IN]: arg is NULL
 * notes:
 */
static AK_VOID* GetStream(AK_VOID *arg)
{
    AK_BASE_ThreadSetName("[aenc stream]attachment");
    AUDIO_STREAM_S stAencStream = {0};
    AK_S32 s32Ret = -1;
    AK_S32 sleep_time = s32FrameInterval == 0 ? 16: (s32FrameInterval / 2);
    AK_BASE_PrintNormalEx(MODULE_ID_APP, "get_stream sleep_time=%d\n", sleep_time);

#ifdef SAVE_FILE_FLAG
    FILE *fp = fopen(g_save_file_path_name, "wb");
    if (NULL == fp) {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "create file err path:%s\n",g_save_file_path_name);
        return NULL;
    }
#endif

    while(1 == s32Flag) {
        s32Ret = AK_MPI_AENC_GetStream(s32AencUserId,&stAencStream, 200);
        if (s32Ret) {
            if (ERROR_AENC_NO_DATA == s32Ret) {
                AK_BASE_SleepMs(sleep_time);
                continue;
            } else {
                AK_BASE_PrintErrorEx(MODULE_ID_APP,
                                    "AK_MPI_AENC_GetStream error s32Ret = %d \n",
                                    s32Ret);
                break;
            }
        } else {
            if (stAencStream.pu8Data && stAencStream.u32Len) {

#ifdef SAVE_FILE_FLAG
                if(stAencStream.u64SeqNo%100 == 0)
                AK_BASE_PrintNormalEx(MODULE_ID_APP,
                                        "get audio frame[%llu] len:%u ts:%llu \n",
                                        (unsigned long long)stAencStream.u64SeqNo,
                                        stAencStream.u32Len,
                                        (unsigned long long)stAencStream.u64Ts);

                fwrite(stAencStream.pu8Data, stAencStream.u32Len, 1, fp);
#endif

            } else {
                continue;
            }
        }
        AK_MPI_AENC_ReleaseStream(s32AencUserId, &stAencStream);
    }

#ifdef SAVE_FILE_FLAG
    fflush(fp);
    fclose(fp);
    fp = NULL;
#endif

    AK_BASE_PrintNoticeEx(MODULE_ID_APP, "%s exit\n", __func__);
    AK_BASE_ThreadExit();
}

/**
 * GetFrameEncode - ai get frame and then send to aenc encode
 * @arg[IN]: arg is NULL
 * notes:
 */
static AK_VOID * GetFrameEncode(AK_VOID *arg)
{
    AK_BASE_ThreadSetName("[aenc frame]attachment");
    static TIME_VAL_S cur_time = {0};

    AK_BASE_GetOstime(&cur_time);
    FRAME_S AIFRAME  = {0};
    AK_U32 read_len = 0;
    AK_U32 total_len = 0;

    AK_S32 s32Ret = AK_FAILED;
    AK_S32 sleep_time = s32FrameInterval == 0 ? 16: (s32FrameInterval / 2);
    AK_BASE_PrintNormalEx(MODULE_ID_APP, "sleep_time=%d\n", sleep_time);

    while(s32Flag) {
        // Check if recording duration reached
        if (g_record_sec > 0) {
            struct timeval current_time;
            gettimeofday(&current_time, NULL);
            unsigned int elapsed_sec = current_time.tv_sec - g_record_start_time.tv_sec;

            if (elapsed_sec >= g_record_sec) {
                AK_BASE_PrintNoticeEx(MODULE_ID_APP, "Recording time reached %u seconds, auto stopping\n", g_record_sec);
                g_auto_stop_flag = 1;
                s32Flag = 0;
                break;
            }
        }

        // Check if manual stop
        if (g_manual_stop_flag) {
            AK_BASE_PrintNoticeEx(MODULE_ID_APP, "Manual stop requested\n");
            s32Flag = 0;
            break;
        }

        /*get pcm data*/
        s32Ret = AK_MPI_AI_GetFrame(s32AiDevId, s32AiChnId,&AIFRAME, 1000, 0);
        if (s32Ret) {
            if (ERROR_AI_NO_DATA == s32Ret) {
                AK_BASE_SleepMs(sleep_time);
                continue;
            } else {
                AK_BASE_PrintErrorEx(MODULE_ID_APP, "AK_MPI_AI_GetFrame error\n");
                /*use to stop get stream*/
                s32Flag = 0;
                break;
            }
        }
        /*send frame to encoder*/
        s32Ret = AK_MPI_AENC_SendFrame(s32AencUserId, &AIFRAME, 2000);
        if(s32Ret) {
            AK_MPI_AI_ReleaseFrame(s32AiDevId, s32AiChnId, &AIFRAME, 0);
            AK_BASE_PrintErrorEx(MODULE_ID_APP, "send frame error\n");
            s32Flag = 0;
            break;
        }

        AK_MPI_AI_ReleaseFrame(s32AiDevId, s32AiChnId, &AIFRAME, 0);
    }

    // Set recording finished flag
    g_recording_finished = 1;

    AK_MPI_AENC_SendEndOfFrame(s32AencUserId);
    AK_BASE_PrintNoticeEx(MODULE_ID_APP, "%s get_frame_encode exit\n", __func__);
    AK_BASE_ThreadExit();
    return NULL;
}

static int init_ai()
{
    AK_S32 s32Ret = 0;
    stAiParam.enSampleBits = SAMPLE_BITS;
    stAiParam.enSampleRate = SAMPLE_RATE;
    stAiParam.enSoundMode = SOUND_MODE;
    stAiParam.u32SrcCnt = 1;
    stAiParam.u32ChnCnt = 1;
    stAiParam.unSrcConfig.stAdcConfig.enSourceType = AUDIO_ADC_SOURCE_MIC;

    if (AK_MPI_AI_SetPubAttr(s32AiDevId, &stAiParam)) {
        AK_BASE_PrintErrorEx(MODULE_ID_APP,"AK_MPI_AI_SetPubAttr failed \n");
        return AK_FAILED;
    }

    if (AK_MPI_AI_Enable(s32AiDevId)) {
        AK_BASE_PrintErrorEx(MODULE_ID_APP,"AK_MPI_AI_Enable failed \n");
        return AK_FAILED;
    }
    if (s32AiDevId != DEV_PDM) {
        s32Ret = AK_MPI_AI_SetGain(s32AiDevId, s32AiChnId, s32Gain);
        if(s32Ret){
            AK_BASE_PrintErrorEx(MODULE_ID_APP, "AK_MPI_AI_SetGain failed\n");
            return AK_FAILED;
        }
    }
    AK_S32  s32FrameLen = 0;
    AK_MPI_AI_GetFrmLen(s32AiDevId, s32AiChnId, &s32FrameLen);// use default frame length
    s32FrameInterval = AudioLenToInterval(stAiParam.enSampleRate, AK_AUDIO_SMPLE_BIT_16,
                                        stAiParam.enSoundMode, s32FrameLen);

    if(AK_MPI_AI_LoadConf(s32AiDevId, s32AiChnId, spchConfFile, spchConfSection)){
        AK_BASE_PrintWarningEx(MODULE_ID_APP,"AK_MPI_AI_LoadConf failed \n");
    }

    if (AK_MPI_AI_EnableChn(s32AiDevId,s32AiChnId)) {
        AK_BASE_PrintErrorEx(MODULE_ID_APP,"AK_MPI_AI_EnableChn failed \n");
        return AK_FAILED;
    }
    return AK_SUCCESS;
}

static int destroy_ai_aenc(void)
{
    int ret = 0;

    ret = AK_MPI_AENC_Close(s32AencUserId);
    if(ret)
    {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "AK_MPI_AENC_Close failed! ret:%d %#x\n",ret,ret);
        return AK_FAILED;
    }

    ret = AK_MPI_AI_DisableChn(s32AiDevId, s32AiChnId);
    if(ret)
    {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "AK_MPI_AI_DisableChn failed! ret:%d %#x\n",ret,ret);
        return AK_FAILED;
    }

    ret = AK_MPI_AI_Disable(s32AiDevId);
    if(ret)
    {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "AK_MPI_AI_Disable failed! ret:%d %#x\n",ret,ret);
        return AK_FAILED;
    }

    return AK_SUCCESS;
}

static int init_aenc()
{
    /*use to aenc open*/
    AENC_PARAM_S stAencParam = {0};

    stAencParam.enType          = SOUND_ENC_TYPE;
    stAencParam.enSampleBits    = SAMPLE_BITS;
    stAencParam.enSampleRate    = SAMPLE_RATE;
    stAencParam.enSoundMode     = SOUND_MODE;

    /*use to register*/
    AENC_ENCODE_S pstEncode = {0};
    pstEncode.enType = stAencParam.enType;
    switch (stAencParam.enType) {
        case AK_AUDIO_TYPE_MP3:
            printf("AK_AUDIO_TYPE_MP3\n");
            pstEncode.pfnEncOpen = AENC_OpenMp3;
            if (AK_MPI_AENC_RegisterEncoder(&pstEncode)) {
                AK_BASE_PrintErrorEx(MODULE_ID_APP, "AK_MPI_AENC_RegisterEncoder failed\n");
                return AK_FAILED;
            }
            break;
        case AK_AUDIO_TYPE_AAC:
            printf("AK_AUDIO_TYPE_AAC\n");
            /*set aac param*/
            stAencParam.unAencAttr.stAacParam.enAacFrmHead = AENC_AAC_SAVE_FRAME_HEAD;
            pstEncode.pfnEncOpen = AENC_OpenAac;
            if (AK_MPI_AENC_RegisterEncoder(&pstEncode)) {
                AK_BASE_PrintErrorEx(MODULE_ID_APP, "AK_MPI_AENC_RegisterEncoder failed\n");
                return AK_FAILED;
            }
            break;
        case AK_AUDIO_TYPE_AMR:
            printf("AK_AUDIO_TYPE_AMR\n");
            pstEncode.pfnEncOpen = AENC_OpenAmr;
            if (AK_MPI_AENC_RegisterEncoder(&pstEncode)) {
                AK_BASE_PrintErrorEx(MODULE_ID_APP, "AK_MPI_AENC_RegisterEncoder failed\n");
                return AK_FAILED;
            }
            break;
        case AK_AUDIO_TYPE_PCM_ALAW:
        case AK_AUDIO_TYPE_PCM_ULAW:
            break;
        default:
            break;
    }

    if(AK_MPI_AENC_Open(s32AencUserId, &stAencParam)) {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "AK_MPI_AENC_Open failed\n");
        return AK_FAILED;
    }

    return AK_SUCCESS;
}

int ak_sound_demo_start_record(char *audio_save_file_name, unsigned int record_sec, unsigned int record_min_sec)
{
    if (audio_save_file_name == NULL) {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "audio_save_file_name is NULL\n");
        return -1;
    }
    ensure_directory_exists(audio_save_file_name);
    char audio_file_name[256] = {0};
    DATE_S sDate = {0};
    AK_BASE_GetLocalDate(&sDate);

    snprintf(audio_file_name, sizeof(audio_file_name),"%s/%04d%02d%02d%02d%02d%02d.mp3",
        audio_save_file_name,
        sDate.s32Year,sDate.s32Month,sDate.s32Day,
        sDate.s32Hour,sDate.s32Month,sDate.s32Second);

    // Check if the filename ends with .mp3
    int len = strlen(audio_file_name);
    if (len < 4 || strcmp(audio_file_name + len - 4, ".mp3") != 0) {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "audio_file_name must end with .mp3\n");
        return -1;
    }

    // Initialize recording control parameters
    g_record_sec = record_sec;
    g_record_min_sec = record_min_sec;
    g_auto_stop_flag = 0;
    g_manual_stop_flag = 0;
    g_recording_finished = 0;
    gettimeofday(&g_record_start_time, NULL);

    s32Flag = 1;

    strncpy(g_save_file_path_name,audio_file_name,256);

    AK_BASE_PrintNormalEx(MODULE_ID_APP, "Start recording: max_time=%u sec, min_time=%u sec, file=%s\n",
                         record_sec, record_min_sec, audio_file_name);

    /*get encode stream */
    AK_BASE_ThreadCreate(&(get_tid), GetStream, NULL,ANYKA_THREAD_MIN_STACK_SIZE, 19);
    /*get ai frame and to encode*/
    AK_BASE_ThreadCreate(&(encode_tid),GetFrameEncode, NULL, ANYKA_THREAD_MIN_STACK_SIZE, 19);

    return 0;
}

int ak_sound_demo_get_already_record_times()
{
    if (!s32Flag) {
        // Recording has not started or has ended
        return 0;
    }

    struct timeval current_time;
    gettimeofday(&current_time, NULL);

    // Calculate recorded duration (seconds)
    unsigned int elapsed_sec = current_time.tv_sec - g_record_start_time.tv_sec;

    return (int)elapsed_sec;
}

int ak_sound_demo_is_recording_finished()
{
    return g_recording_finished;
}

int ak_sound_demo_stop_record()
{
    if (!s32Flag) {
        AK_BASE_PrintWarningEx(MODULE_ID_APP, "Recording is not active\n");
        return 0;
    }

    // Set manual stop flag
    g_manual_stop_flag = 1;

    // Calculate current recording duration
    struct timeval current_time;
    gettimeofday(&current_time, NULL);
    unsigned int elapsed_sec = current_time.tv_sec - g_record_start_time.tv_sec;

    AK_BASE_PrintNormalEx(MODULE_ID_APP, "Stop recording requested, elapsed time: %u seconds\n", elapsed_sec);

    // Stop recording thread
    s32Flag = 0;
    AK_BASE_ThreadJoin(get_tid);
    AK_BASE_ThreadJoin(encode_tid);

    // Check if recording duration meets minimum requirement
    if (g_record_min_sec > 0 && elapsed_sec < g_record_min_sec) {
        AK_BASE_PrintWarningEx(MODULE_ID_APP,
                              "Recording time (%u sec) is shorter than minimum required (%u sec), deleting file: %s\n",
                              elapsed_sec, g_record_min_sec, g_save_file_path_name);

        // Delete recording file
        if (unlink(g_save_file_path_name) == 0) {
            AK_BASE_PrintNormalEx(MODULE_ID_APP, "File deleted successfully: %s\n", g_save_file_path_name);
        } else {
            AK_BASE_PrintErrorEx(MODULE_ID_APP, "Failed to delete file: %s (%s)\n",
                                g_save_file_path_name, strerror(errno));
        }
        return -1; // Return -1 indicating duration too short, file deleted
    } else {
        AK_BASE_PrintNormalEx(MODULE_ID_APP, "Recording completed successfully, file saved: %s\n", g_save_file_path_name);
        return 0; // Return 0 indicating success
    }
}

int ak_sound_demo_init()
{
    int ret = 0;

#ifdef AI_DENOISE_ENABLE
    if (InitAIDenoise()) {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "init AIDenoise failed\n");
        return AK_FAILED;
    }
#endif

    ret = init_ai();
    if(ret)
    {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "init_ai failed! ret:%d %#x\n",ret,ret);
        return AK_FAILED;
    }

    ret = init_aenc();
    if(ret)
    {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "init_aenc failed! ret:%d %#x\n",ret,ret);
        return AK_FAILED;
    }

    return 0;
}

int ak_sound_demo_destroy()
{
    destroy_ai_aenc();
#ifdef AI_DENOISE_ENABLE
    DestroyAIDenoise();
#endif

    return 0;
}
