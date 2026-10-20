/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd. */
#ifndef __AK_RTSP_DEMO_H__
#define __AK_RTSP_DEMO_H__

#include "ak_common_video.h"

/**
 * @brief RTSP event type enumeration
 */
typedef enum {
    RTSP_EVENT_LOGIN = 0,    /**< RTSP user login event */
    RTSP_EVENT_LOGOUT        /**< RTSP user logout event */
} rtsp_event_type_t;

/**
 * @brief RTSP event callback function type
 *
 * @param event RTSP event type
 * @param client_ip Client IP address string
 */
typedef void (*rtsp_event_callback_t)(rtsp_event_type_t event, const char* client_ip);

// int ak_rtsp_demo_init_event_cb_func(rtsp_event_callback_t callback);


/**
 * @brief Sends a video stream to the RTSP server.
 *
 * @param venc_handle_id Video encoder handle ID.
 * @param pstStream Pointer to the video stream structure.
 * @return 0 on success, negative error code on failure.
 */
int send_stream_to_rtsp(int venc_handle_id, VIDEO_STREAM_S *pstStream);

/**
 * @brief Initializes the audio environment for RTSP.
 *
 * @return 0 on success, negative error code on failure.
 */
int ak_rtsp_demo_audio_env_init(void);

/**
 * @brief Destroys the audio environment for RTSP.
 *
 * @return 0 on success, negative error code on failure.
 */
int ak_rtsp_demo_audio_env_destory(void);

/**
 * @brief Initializes the video environment for RTSP.
 *
 * @return 0 on success, negative error code on failure.
 */
int ak_rtsp_demo_video_env_init(void);

/**
 * @brief Destroys the video environment for RTSP.
 *
 * @return 0 on success, negative error code on failure.
 */
int ak_rtsp_demo_video_env_destory(void);

/**
 * @brief Initializes the RTSP demo environment.
 *
 * @param isp_conf_path Path to the ISP configuration file.
 * @param venc_thread_run_flag Flag to control the video encoder thread.
 * @param callback Callback function for RTSP events.
 * @return 0 on success, negative error code on failure.
 */
int ak_rtsp_demo_env_init(char* isp_conf_path, int venc_thread_run_flag, rtsp_event_callback_t callback);

/**
 * @brief Destroys the RTSP demo environment.
 *
 * @param venc_thread_run_flag Flag to control the video encoder thread.
 * @return 0 on success, negative error code on failure.
 */
int ak_rtsp_demo_env_destory(int venc_thread_run_flag);

// int ak_sdk_env_init();
// int ak_sdk_env_destory();

#endif
