/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd. */
#ifndef __VIDEO_EIS_H__
#define __VIDEO_EIS_H__

#include "ak_common_video.h"

#define CROP_RATIO      0.70f /**< Crop ratio */

#define EXPOSE_OFFSET_TIME 12 /**< Exposure offset time ms */

#define LEN_TIME         64

#define MUTEX_DEFINE( MUTEX )   pthread_mutex_t pthread_mutex_t_##MUTEX = PTHREAD_MUTEX_INITIALIZER
#define MUTEX_INIT( MUTEX )     pthread_mutex_init( &pthread_mutex_t_##MUTEX, NULL )
#define MUTEX_LOCK( MUTEX )     pthread_mutex_lock( &pthread_mutex_t_##MUTEX )
#define MUTEX_UNLOCK( MUTEX )   pthread_mutex_unlock( &pthread_mutex_t_##MUTEX )
#define MUTEX_DESTROY( MUTEX )  pthread_mutex_destroy( &pthread_mutex_t_##MUTEX )

typedef int (*VideoCallBackFunc)(int,VIDEO_STREAM_S *);

/**
 * @brief Initialization function for EIS
 * @param vpss_grp VPSS group ID
 * @param vpss_chn VPSS channel ID
 * @param vi_w Input width
 * @param vi_h Input height
 * @param out_width Output width
 * @param out_height Output height
 * @param frame_depth Frame depth
 * @param video_cb Video callback function
 * @return 0 on success, negative value on failure
 */
int eis_init(int vpss_grp, int vpss_chn, int vi_w,int vi_h, int out_width, int out_height,int frame_depth, VideoCallBackFunc video_cb);

/**
 * @brief Start EIS threads
 * @return 0 on success, negative value on failure
 */
int eis_start_threads(void);

/**
 * @brief Stop EIS threads
 * @return 0 on success, negative value on failure
 */
int eis_stop_threads(void);

/**
 * @brief Cleanup EIS resources
 */
void eis_cleanup(void);

#endif
