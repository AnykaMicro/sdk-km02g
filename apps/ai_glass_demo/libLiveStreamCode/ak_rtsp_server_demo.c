/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
// #include <error.h>
#include <errno.h>
#include "ak_common.h"
#include "ak_log.h"
#include "ak_rtsp_server.h"
#include "ak_rtsp_demo.h"
#include "wifi_manager.h"
#ifdef EIS_ENABLE
#include "video_eis.h"
#include "media_configs.h"
#endif

#define RTSP_URL_CHN0 "vs0"
#define RTSP_URL_CHN1 "vs1"
#define RTSP_URL_PREFIX "vs"
#define RTSP_MAX_CLIENT 8
typedef struct
{
    int iUsed;
    int iStreamType; // 0:Real-time stream; 1:Recording playback
    int iChn;
    int iHandleV;
    int iHandleA;
} rtsp_stream_handle;
rtsp_stream_handle stStreamHandle[RTSP_MAX_CLIENT] = {0};
static rtsp_event_callback_t rtsp_event_callback_func = NULL;     /**< Event callback function */
int iRecordStartTime = 0;
int rtsp_get_idle_handle()
{
    int nRet = -1;
    for (int i = 0; i < RTSP_MAX_CLIENT; i++)
    {
        if (stStreamHandle[i].iUsed == 0)
        {
            nRet = i;
            break;
        }
    }
    return nRet;
}

int aa_ls_get_audio_enc_param(rtsp_audio_enc_param *param);
int aa_ls_get_video_enc_param(int dev, int chn, rtsp_video_enc_param *param);
// 0:main stream ; 1:sub stream ; 2:audio stream
int aa_ls_reg_stream_client(int nDevId, int stream_id, int *pnHandle, char *pReaderName);
int aa_ls_unreg_stream_client(int nDevId, int stream_id, int nHandle);
int aa_ls_force_i_frame(int dev, int chn);
int aa_ls_get_frame(int nDevId, int stream_id, int nHandle, rtsp_frame_info *pFrame);

// Error return code callback
int rtsp_server_get_error_code_cb(rtsp_error_type_e eErrorCode)
{
    printf("[%s %d]eErrorCode:%d\n", __func__, __LINE__, eErrorCode);
    return 0;
}

int rtsp_server_get_dev_param_cb(rtsp_param_type_e eParamType, rtsp_dev_param *pParam)
{
    switch (eParamType)
    {
    case RTSP_NET_ADDR_IPV4:
        // Advertise the Wi-Fi address; 0.0.0.0 until wlan0 has one
        if (wifi_get_ip(pParam->cAddrIPv4) != 0)
            strcpy(pParam->cAddrIPv4, "0.0.0.0");
        break;

    case RTSP_NET_ADDR_IPV6:
        // ak_net_get_ipv6("eth0", pParam->cAddrIPv6);
        break;
    default:
        break;
    }

    return 0;
}
int rtsp_server_get_video_param_cb(const char *pSourceToken, rtsp_video_enc_param *pParam)
{
    printf("[%s %d]pSourceToken:%s\n", __func__, __LINE__, pSourceToken);
    char *pStart = strstr(pSourceToken, RTSP_URL_PREFIX);
    if (pStart != NULL)
    {
        int iChn = atoi(pStart + strlen(RTSP_URL_PREFIX));
        // aa_video_enc_param param = { 0 };

        aa_ls_get_video_enc_param(0, iChn, pParam);
        printf("[%s %d]eCodeType:%d\n", __func__, __LINE__, pParam->eCodeType);
        return 0;
    }

    return 0;
}
// Get audio parameter callback
int rtsp_server_get_audio_param_cb(const char *pSourceToken, rtsp_audio_enc_param *pParam)
{
    // vs
    char *pStart = strstr(pSourceToken, RTSP_URL_PREFIX);
    if (pStart != NULL)
    {
        int iChn = atoi(pStart + strlen(RTSP_URL_PREFIX));
        // aa_ls_get_audio_enc_param(0, iChn,pParam);

        printf("[%s %d]iChn:%d \n", __func__, __LINE__, iChn);
        return aa_ls_get_audio_enc_param(pParam);
    }

    return -1;
}

int rtsp_server_accept_client_cb(char *pClientIP, int iClientPort)
{
    printf("[%s %d]pClientIP:%s iClientPort=%d\n", __func__, __LINE__, pClientIP, iClientPort);
    if(NULL != rtsp_event_callback_func)
        rtsp_event_callback_func(RTSP_EVENT_LOGIN,pClientIP);
    return 0;
}

int rtsp_server_play_cb(char *pSourceToken)
{
    // rtsp://192.168.100.1:554/vs0?aspect_ratio=16_9
    printf("[%s %d]play source token:%s\n", __func__, __LINE__, pSourceToken);
    int i = 0;

//#ifndef RTSP_START_FIRST_ENABLE
//    // Parse aspect_ratio parameter
//    char *aspect_ratio_param = strstr(pSourceToken, "aspect_ratio=");
//    if (aspect_ratio_param != NULL) {
//        aspect_ratio_param += strlen("aspect_ratio=");

//        // Find the end of the parameter value (encounter & or string end)
//        char *param_end = strchr(aspect_ratio_param, '&');
//        int param_len;
//        if (param_end != NULL) {
//            param_len = param_end - aspect_ratio_param;
//        } else {
//            param_len = strlen(aspect_ratio_param);
//        }

//        // Extract parameter value
//        char aspect_ratio_value[16] = {0};
//        strncpy(aspect_ratio_value, aspect_ratio_param, param_len);
//        aspect_ratio_value[param_len] = '\0';

//        // Remove trailing whitespace
//        char *end = aspect_ratio_value + strlen(aspect_ratio_value) - 1;
//        while (end > aspect_ratio_value && (*end == ' ' || *end == '\t' || *end == '\r' || *end == '\n')) {
//            *end = '\0';
//            end--;
//        }

//        // Print parsed result
//        printf("[%s %d] Parsed aspect_ratio: %s (len=%d)\n", __func__, __LINE__, aspect_ratio_value, (int)strlen(aspect_ratio_value));

//        // Handle according to different ratio parameters
//        if (strcmp(aspect_ratio_value, "16_9") == 0) {
//            printf("[%s %d] Video aspect ratio: 16:9 (widescreen)\n", __func__, __LINE__);
//        } else if (strcmp(aspect_ratio_value, "4_3") == 0) {
//            printf("[%s %d] Video aspect ratio: 4:3 (standard)\n", __func__, __LINE__);
//        } else if (strcmp(aspect_ratio_value, "3_4") == 0) {
//            printf("[%s %d] Video aspect ratio: 3:4 (portrait)\n", __func__, __LINE__);
//        } else {
//            printf("[%s %d] Unknown aspect ratio: '%s' (len=%d)\n", __func__, __LINE__, aspect_ratio_value, (int)strlen(aspect_ratio_value));
//        }
//    } else {
//        printf("[%s %d] No aspect_ratio parameter found in URL\n", __func__, __LINE__);
//    }

//#endif

//#ifndef RTSP_START_FIRST_ENABLE

//    //Start audio and video
//    ak_rtsp_demo_video_env_init();

//#ifdef EIS_ENABLED
//    int ret = 0;
//    // Eis init
//    ret = eis_init(0, 0, RTSP_MAIN_CHN_WIDTH, RTSP_MAIN_CHN_HEIGHT, send_stream_to_rtsp);
//    if (ret != 0) {
//        printf("Failed to initialize EIS system\n");
//        // ak_rtsp_demo_env_destory(0);
//        // ak_sdk_env_destory();
//        return -1;
//    }

//    // Eis run
//    ret = eis_start_threads();
//    if (ret != 0) {
//        printf("Failed to start EIS threads\n");
//        eis_cleanup();
//        // ak_rtsp_demo_env_destory(0);
//        // ak_sdk_env_destory();
//        return -1;
//    }
//#endif

//    ak_rtsp_demo_audio_env_init();
//#endif

    char *pStart = strstr(pSourceToken, RTSP_URL_PREFIX);
    if (pStart != NULL)
    {
        int iChn = atoi(pStart + strlen(RTSP_URL_PREFIX));
        i = rtsp_get_idle_handle();
        if (i < 0)
        {
            printf("[%s %d]get idle handle error.\n", __func__, __LINE__);
            return -1;
        }
        char client[16] = {0};
        sprintf(client, "Vclient%d", i);
        int ret = aa_ls_reg_stream_client(0, iChn, &stStreamHandle[i].iHandleV, client);
        if (ret)
        {
            return -1;
        }
        sprintf(client, "Aclient%d", i);
        ret = aa_ls_reg_stream_client(0, 2, &stStreamHandle[i].iHandleA, client);
        if (ret)
        {
            aa_ls_unreg_stream_client(0, stStreamHandle[i].iChn, stStreamHandle[i].iHandleV);
            return -1;
        }
        printf("[%s %d]iHandleV:%d iHandleA=%d\n", __func__, __LINE__, stStreamHandle[i].iHandleV, stStreamHandle[i].iHandleA);

        stStreamHandle[i].iUsed = 1;
        stStreamHandle[i].iStreamType = 0;
        printf("force i frame i:%d iChn:%d\n", i, iChn);
        aa_ls_force_i_frame(0, iChn);
        stStreamHandle[i].iChn = iChn;
        return i;
    }

    return 0;
}

int rtsp_server_teardown_cb(int iHandle)
{
    printf("[%s %d]fun=%s handle = %d ,iStreamType=%d\n", __func__, __LINE__, __FUNCTION__, iHandle, stStreamHandle[iHandle].iStreamType);
    if (stStreamHandle[iHandle].iStreamType == 0)
    {
        printf("[%s %d]handle  %d \n", __func__, __LINE__, iHandle);
        aa_ls_unreg_stream_client(0, stStreamHandle[iHandle].iChn, stStreamHandle[iHandle].iHandleV);
        aa_ls_unreg_stream_client(0, 2, stStreamHandle[iHandle].iHandleA);
        stStreamHandle[iHandle].iUsed = 0;
        stStreamHandle[iHandle].iStreamType = -1;

//#ifndef RTSP_START_FIRST_ENABLE

//        // Close audio and video
//        // printf("[%s %d]stop audio\n",__func__,__LINE__);
//        ak_rtsp_demo_audio_env_destory();

//#ifdef EIS_ENABLED
//        printf("EIS system OUT!!!\n");
//        // stop
//        eis_stop_threads();

//        // clean
//        eis_cleanup();
//#endif

//        // printf("[%s %d]stop video\n",__func__,__LINE__);
//        ak_rtsp_demo_video_env_destory();

//        // printf("[%s %d]stop done\n",__func__,__LINE__);
//#endif
        if(NULL != rtsp_event_callback_func)
            rtsp_event_callback_func(RTSP_EVENT_LOGOUT,NULL);

        return 0;
    }


    return 0;
}
// Get video streaming callback
int rtsp_server_get_video_cb(int iHandle, rtsp_frame_info *pVideo, void *pUserData)
{
    //    printf("handle = %d ,iStreamType=%d\n",iHandle,stStreamHandle[iHandle].iStreamType);
    if (stStreamHandle[iHandle].iStreamType == 0)
    {
        // ak_print_error_ex(MODULE_ID_APP, "handle = %d ,iStreamType=%d\n",iHandle,stStreamHandle[iHandle].iStreamType);
        int iVideoHandle = stStreamHandle[iHandle].iHandleV;
        int iChn = stStreamHandle[iHandle].iChn;

        return aa_ls_get_frame(0, iChn, iVideoHandle, pVideo);
    }
    return 0;
}

// Get audio streaming callback
int rtsp_server_get_audio_cb(int iHandle, rtsp_frame_info *pAudio, void *pUserData)
{
    int iAudioHandle = stStreamHandle[iHandle].iHandleA;
    return aa_ls_get_frame(0, 2, iAudioHandle, pAudio);
}

int rtsp_server_init(rtsp_event_callback_t callback)
{
    rtsp_server_init_param(NULL);
    if(callback!= NULL)
        rtsp_event_callback_func = callback;

    // Set network information
    rtsp_server_set_dev_param_callback(rtsp_server_get_dev_param_cb);

    // Set callback to get video parameters
    rtsp_server_set_video_param_callback(rtsp_server_get_video_param_cb);

    // Set callback to get audio parameters
    rtsp_server_set_audio_param_callback(rtsp_server_get_audio_param_cb);

    // Client information callback
    rtsp_server_set_accept_client_callback(rtsp_server_accept_client_cb);

    // Play callback, return streaming url
    rtsp_server_set_play_callback(rtsp_server_play_cb);

    // Teardown callback, return play handle
    rtsp_server_set_teardown_callback(rtsp_server_teardown_cb);

    // Set video streaming callback
    rtsp_server_set_video_callback(rtsp_server_get_video_cb, NULL);

    // Set audio streaming callback
    rtsp_server_set_audio_callback(rtsp_server_get_audio_cb, NULL);

    // Set streaming callback, including video and audio
    // rtsp_server_set_media_callback(rtsp_server_get_media_cb, NULL);

    // Set error return code callback
    rtsp_server_set_error_code_callback(rtsp_server_get_error_code_cb);

    // Start rtsp service
    rtsp_server_start();

    return 0;
}

int rtsp_server_deinit()
{
    rtsp_server_stop();
    return 0;
}
