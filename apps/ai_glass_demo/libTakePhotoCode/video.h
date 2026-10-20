/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd. */
#ifndef _VIDEO_H_
#define _VIDEO_H_

#include "fw_media_types.h"

#define CAMERA_GC5603           1
//#define CAMERA_OG01A1X          1

/* resolation define */
struct resolution_t {
    unsigned int width;     /**< Width in pixels */
    unsigned int height;    /**< Height in pixels */
    unsigned char str[20];  /**< String representation */
};

#if CAMERA_OG01A1X
/**
 * @brief Video resolutions for OG01A1X camera.
 */
typedef enum {
    DE_VIDEO_240P = 0,  /**< 240p resolution */
    DE_VIDEO_320P,      /**< 320p resolution */
    DE_VIDEO_VGA,       /**< VGA resolution */
    DE_VIDEO_480P,      /**< 480p resolution */
    DE_VIDEO_720P,      /**< 720p resolution */
    DE_VIDEO_1024P,     /**< 1024p resolution */
} ch_resolutions;
#else
/**
 * @brief Video resolutions for other cameras (e.g., GC5603).
 */
typedef enum {
//    DE_VIDEO_240P = 0,
//    DE_VIDEO_320P,
//    DE_VIDEO_VGA,
//    DE_VIDEO_480P,
//    DE_VIDEO_720P,
    DE_VIDEO_1080P,     /**< 1080p resolution */
//    DE_VIDEO_1440P,
} ch_resolutions;
#endif

/**
 * @brief Device channel configuration.
 */
struct dev_ch_t {
     int main_ch;   /**< Main channel ID */
     int sub_ch;    /**< Sub channel ID */
 };


/**
 * @brief Initializes the UVC (USB Video Class) video module.
 *
 * @param pvenc Pointer to the video encoding parameters structure.
 * @param get_frame_fun Callback function to retrieve video frames.
 * @return 0 on success, negative error code on failure.
 */
int video_uvc_init(fw_video_enc_param *pvenc, FW_STREAM_CB get_frame_fun);




#endif
