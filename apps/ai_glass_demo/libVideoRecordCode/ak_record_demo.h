/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd. */
#ifndef __AK_RECORD_DEMO_H__
#define __AK_RECORD_DEMO_H__

#include "ak_common_video.h"

/**
 * @brief Sends a video stream frame to the recording module.
 *
 * @param venc_handle_id Handle ID of the video encoder.
 * @param video_stream_get Pointer to the video stream structure containing frame data.
 * @return 0 on success, negative error code on failure.
 */
int send_stream_to_record(int venc_handle_id, VIDEO_STREAM_S *video_stream_get);

/**
 * @brief Starts the video recording process.
 *
 * @param isp_conf_path Path to the ISP configuration file.
 * @param rec_sec Maximum recording duration in seconds.
 * @param i_frame_depth Video frame depth configuration.
 * @param video_thread_run_flag Flag to control the video thread running state.
 * @return 0 on success, negative error code on failure.
 */
int ak_record_start(char *isp_conf_path, unsigned int rec_sec, int i_frame_depth, int video_thread_run_flag);

/**
 * @brief Waits for the recording to stop.
 *
 * Blocks until the recording process has completely stopped.
 *
 * @return 0 on success, negative error code on failure.
 */
int ak_record_wait_stop(void);

/**
 * @brief Signals the recording process to stop.
 *
 * @return 0 on success, negative error code on failure.
 */
int ak_record_stop(void);

/**
 * @brief Stops the recording and retrieves the recorded file path.
 *
 * @param record_file_path Pointer to a string pointer where the file path will be stored.
 * @return 0 on success, negative error code on failure.
 */
int ak_record_stop_and_exit(char **record_file_path);

// int ak_sdk_env_init();
// int ak_sdk_env_destory();

#endif