/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd. */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <regex.h>
#include <getopt.h>
#include <errno.h>
#include <dirent.h>
#include <stdint.h>
#include "ak_record_demo.h"
#include "media_configs.h"

#include "ak_common.h"
#include "ak_common_video.h"
#include "ak_common_audio.h"
#include "ak_ai.h"
#include "ak_aenc.h"
#include "ak_thread.h"
#include "ak_log.h"
#include "ak_mem.h"
#include "ak_vi.h"
#include "ak_vpss.h"
#include "ak_venc.h"
#include "ak_mux.h"
#include "ak_mux_sample.h"
#include "aenc_mp3.h"
#include "aenc_aac.h"
#include "aenc_amr.h"

#include "ak_mux_sample.h"
#include "ai_glass_common.h"



static struct dvr_conf g_dvr_conf = {0};



int send_stream_to_record(int venc_handle_id, VIDEO_STREAM_S *video_stream_get)
{
    return ak_dvr_send_stream_to_record(&g_dvr_conf, venc_handle_id, video_stream_get);
}

int ak_record_stop_and_exit(char **record_file_path)
{
    int ret = 0;

    ret = ak_dvr_stop(&g_dvr_conf);
    if(ret != 0)
    {
        printf("ak_dvr_stop failed! ret:%d\n",ret);
        ret = -1;
    }

    ret = ak_dvr_exit(&g_dvr_conf);
    if(ret != 0)
    {
        printf("ak_dvr_stop failed! ret:%d\n",ret);
        ret = -1;
    }

    // Destroy mutexes
    pthread_mutex_destroy(&g_dvr_conf.pthread_mutex_t_aenc);
    pthread_mutex_destroy(&g_dvr_conf.pthread_mutex_t_venc);
    pthread_mutex_destroy(&g_dvr_conf.pthread_mutex_t_ai);

    *record_file_path = g_dvr_conf.final_file_video;
    return ret;
}

int ak_record_start(char *isp_conf_path, unsigned int rec_sec, int i_frame_depth, int video_thread_run_flag)
{
    int ret = 0;
    char gc_run_out = 0;
    char saving_flag_out = 0;
    get_gcrun_flag(&gc_run_out, &saving_flag_out);

    if(saving_flag_out == AK_TRUE)
    {
        printf("saving_flag_out == AK_TRUE please reboot\n");
        return -1;
    }

    char ac_cmd[ LEN_CMD ]= {0};

    if(i_frame_depth < 2)
    {
        printf("i_frame_depth too small:%d, change to 2\n",i_frame_depth);
        i_frame_depth = 2;
    }

    // Use memset to ensure the structure is completely cleared
    memset(&g_dvr_conf, 0, sizeof(struct dvr_conf));

    // // Initialize mutexes
    // pthread_mutex_init(&g_dvr_conf.pthread_mutex_t_aenc, NULL);
    // pthread_mutex_init(&g_dvr_conf.pthread_mutex_t_venc, NULL);
    // pthread_mutex_init(&g_dvr_conf.pthread_mutex_t_ai, NULL);

    // Initialize all fields of g_dvr_conf structure
    g_dvr_conf.s32AiDevId = 0;
    g_dvr_conf.s32AiChnId = 0;
    g_dvr_conf.s32AencId = 0;
    g_dvr_conf.s32ViDevId = 0; // vi device
    g_dvr_conf.s32ViChnId = 0;
    g_dvr_conf.s32VpssGrpId = 0;
    g_dvr_conf.s32VpssChnId = 0;
    g_dvr_conf.s32VencId = 0;
    g_dvr_conf.s32MuxHandle = HANDLE_NOT_INIT;

    snprintf(g_dvr_conf.ac_isp_cfg, sizeof(g_dvr_conf.ac_isp_cfg), "%s", isp_conf_path); // Sensor configuration file

    g_dvr_conf.i_sub_width = VI_SUB_WIDTH; // Sub-channel resolution
    g_dvr_conf.i_sub_height = VI_SUB_HEIGHT;
    g_dvr_conf.i_width = gm_config.record_param.main_vi_width; // Main channel resolution
    g_dvr_conf.i_height = gm_config.record_param.main_vi_height;
    g_dvr_conf.i_fps = gm_config.record_param.main_chn_fps; // Capture frame rate
    g_dvr_conf.i_frame_depth = i_frame_depth; // Capture buffer setting

    g_dvr_conf.i_venc_type = gm_config.record_param.main_chn_enc_type;
    g_dvr_conf.i_br_mode = gm_config.record_param.main_chn_br_mode;
    g_dvr_conf.i_kbps = gm_config.record_param.main_chn_target_kbps;

    // Note: pc_ai_conf_file and pc_ai_conf_section are char pointers, not arrays
    g_dvr_conf.pc_ai_conf_file = "/etc/config/audio_config.ini";
    g_dvr_conf.pc_ai_conf_section = "def_near";

    g_dvr_conf.stAiConf.enSampleBits = AK_AUDIO_SMPLE_BIT_16; // Currently only supports 16-bit values
    g_dvr_conf.stAiConf.enSoundMode = AUDIO_SOUND_MODE_MONO; // Mono
    g_dvr_conf.stAiConf.enSampleRate = AK_AUDIO_SAMPLE_RATE_16000; // Sample rate
    g_dvr_conf.stAiConf.u32ChnCnt = 1; // Number of channels
    g_dvr_conf.stAiConf.unSrcConfig.stAdcConfig.enSourceType = AUDIO_ADC_SOURCE_MIC; // Set MIC input
    g_dvr_conf.stAiConf.u32SrcCnt = 1; // Number of data sources

    g_dvr_conf.i_aenc_type = AK_AUDIO_TYPE_AAC; // Audio encoding format
    g_dvr_conf.stVencConf.stExtAttrs.stAdvance.bEnableMMA = AK_TRUE;

    g_dvr_conf.stVencConf.stVencAttrs.u16Width = gm_config.record_param.main_chn_width; // Resolution width
    g_dvr_conf.stVencConf.stVencAttrs.u16Height = gm_config.record_param.main_chn_height; // Resolution height
    g_dvr_conf.stVencConf.stVencAttrs.u32MaxBufSize = 0;
    g_dvr_conf.stVencConf.stVencAttrs.u32MaxDMASize = 0;
    g_dvr_conf.stVencConf.stVencAttrs.enChromaMode = VENC_CHROMA_YUV420;
    g_dvr_conf.stVencConf.stVencAttrs.u32Priority = 0;
    g_dvr_conf.stVencConf.stVencAttrs.enOutputType = gm_config.record_param.main_chn_enc_type;

    g_dvr_conf.stVencConf.stVencAttrs.stH264Attr.u16Framerate = VI_FPS; // FPS
    g_dvr_conf.stVencConf.stVencAttrs.stH264Attr.u16GopLen = VENC_GOP_LEN; // I-frame frequency
    g_dvr_conf.stVencConf.stVencAttrs.stH264Attr.enEncLevel = VENC_HEVC_AVC_LEVEL_5;
    g_dvr_conf.stVencConf.stVencAttrs.stH264Attr.enGdrMode = VENC_GDR_OFF; // GDR gradual decoding refresh type

    g_dvr_conf.stVencConf.stVencAttrs.stH265Attr.u16Framerate = VI_FPS; // FPS
    g_dvr_conf.stVencConf.stVencAttrs.stH265Attr.u16GopLen = VENC_GOP_LEN; // I-frame frequency
    g_dvr_conf.stVencConf.stVencAttrs.stH265Attr.enEncLevel = VENC_HEVC_AVC_LEVEL_5;
    g_dvr_conf.stVencConf.stVencAttrs.stH265Attr.enGdrMode = VENC_GDR_OFF; // GDR gradual decoding refresh type

    // Audio encoding format (AVI format optional values: - MP3:2, AAC:4, PCM:6, (g711a)PCM_ALAW:17, (g711u)PCM_ULAW:18)
    g_dvr_conf.stMuxConf.enAudioType = AK_AUDIO_TYPE_AAC; // Audio encoding format configured by mux
    // (3gp/mp4 format optional values: - AMR:3, AAC:4, (g711a)PCM_ALAW:17, (g711u)PCM_ULAW:18)
    g_dvr_conf.stMuxConf.enVencOutputType = gm_config.record_param.main_chn_enc_type; // venc encoding type (optional values: 0, H264; 1, JPEG; 2, H265)
    g_dvr_conf.stMuxConf.enMuxType = MUX_TYPE_MP4; // Composite file type (optional values: 0: MUX_TYPE_AVI, 1: MUX_TYPE_MP4)
    g_dvr_conf.stMuxConf.u16Width = gm_config.record_param.main_chn_width; // Video resolution configured by mux
    g_dvr_conf.stMuxConf.u16Height = gm_config.record_param.main_chn_height;
    g_dvr_conf.stMuxConf.u16Fps = VI_FPS; // Frame rate configured by mux
    g_dvr_conf.stMuxConf.u32RecordSec = RECORD_SEC; // Recording duration
    g_dvr_conf.stMuxConf.u8CaptureAudio = AK_TRUE; // Whether to capture audio

    // Sample rate (optional values: 8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000)
    g_dvr_conf.stMuxConf.stAudioDataAttr.enSampleRate = AK_AUDIO_SAMPLE_RATE_16000;
    g_dvr_conf.stMuxConf.stAudioDataAttr.enSoundMode = AUDIO_SOUND_MODE_MONO;
    g_dvr_conf.stMuxConf.stAudioDataAttr.enSampleBits = AK_AUDIO_SMPLE_BIT_16;
    g_dvr_conf.stMuxConf.s32FrameLen = 0; // AI frame length
    g_dvr_conf.stMuxConf.enMuxDataDest = MUX_DATA_TO_LOCAL_FILE;

    g_dvr_conf.i_sec = rec_sec; // Recording duration
    snprintf(g_dvr_conf.ac_path_save, sizeof(g_dvr_conf.ac_path_save), "%s", RECORD_PATH_SAVE); // Recording save path
    snprintf(g_dvr_conf.ac_path_tmp, sizeof(g_dvr_conf.ac_path_tmp), "%s", RECORD_PATH_TMP); // Recording save path during recording
    snprintf(g_dvr_conf.ac_template_path, sizeof(g_dvr_conf.ac_template_path), "%s", PATH_TEMPLATE); // Path template for file saving
    snprintf(g_dvr_conf.ac_template_file, sizeof(g_dvr_conf.ac_template_file), "%s", FILE_TEMPLATE); // Filename template for file saving

    g_dvr_conf.p_dbdesc_param = NULL; // Database record

    g_dvr_conf.s32BuffRecord = LEN_BUFF_RECORD; // Recording buffer length
    g_dvr_conf.s32BuffIndex = LEN_BUFF_INDEX; // Index buffer length

    g_dvr_conf.i_buff_video = LEN_BUFF_VIDEO; // Limit value of composite video buffer
    g_dvr_conf.i_buff_audio = LEN_BUFF_AUDIO; // Limit value of composite audio buffer
    g_dvr_conf.i_list_video = 0; // Video buffer length to be composed
    g_dvr_conf.i_list_audio = 0; // Audio buffer length to be composed

    g_dvr_conf.enMuxStatus = MUX_STATUS_STOP; // Media library status
    g_dvr_conf.c_mux_init = AK_FALSE; // Media library initialization status
    g_dvr_conf.c_mux_enable = AK_FALSE; // Media library availability status

    // snprintf( ac_cmd, sizeof(ac_cmd), "mkdir -p %s", g_dvr_conf.ac_path_save );
    // system( ac_cmd ); // Create recording save directory
    // snprintf( ac_cmd, sizeof(ac_cmd), "mkdir -p %s", g_dvr_conf.ac_path_tmp );
    // system( ac_cmd );

#if 0
    // Repair the last damaged file (takes time)
    ret = ak_dvr_fix( &g_dvr_conf );
    if(ret != 0)
    {
        printf("ak_dvr_fix failed! ret:%d\n",ret);
        return -1;
    }
#else
    // Do not repair the file, start a new recording directly
#endif

    ret = ak_dvr_init( &g_dvr_conf );
    if(ret != 0)
    {
        printf("ak_dvr_init failed! ret:%d\n",ret);
        return -1;
    }

    ret = ak_dvr_start( &g_dvr_conf ,video_thread_run_flag);
    if(ret != 0)
    {
        printf("ak_dvr_start failed! ret:%d\n",ret);
        return -1;
    }


    return 0;
}

int ak_record_wait_stop()
{
    unsigned int sec_count = 0;
    char gc_run_out = 0;
    char saving_flag_out = 0;

    for(;; )
    {
        if(gc_run_out == 1 && saving_flag_out == 0)
        {
            printf("------[%d]recording %us\n",g_dvr_conf.i_sec,sec_count);
        }
        else if(gc_run_out == 1 && saving_flag_out == 1)
        {
            printf("------[%d]record file saving %us\n",g_dvr_conf.i_sec,sec_count - g_dvr_conf.i_sec);
        }
        else
        {
            printf("------[%d]wait to start\n",g_dvr_conf.i_sec);
        }

        sleep( 1 );

        get_gcrun_flag(&gc_run_out, &saving_flag_out);
        if( AK_TRUE != gc_run_out) {
            printf("%s %d get_gcrun_flag break!\n",__func__,__LINE__);
            break;
        }

        sec_count++;
    }

    return sec_count;
}

int ak_record_stop()
{
    return ak_dvr_manual_stop( &g_dvr_conf );
}
