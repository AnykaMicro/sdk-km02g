/* SPDX-License-Identifier: Apache-2.0 */
/**
* Copyright (C) 2018 Anyka(Guangzhou) Microelectronics Technology CO.,LTD.
* File Name: ak_video_sample.c
* Description: This is a simple example to show how the video module working.
* Notes: Before running please insmod sensor_xxx.ko ak_isp.ko ak_venc_adapter.ko ak_venc_bridge.ko at first.
* History: V1.0.0
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <getopt.h>
#include <unistd.h>
#include <poll.h>
#include <errno.h>
#include "ak_common.h"
#include "ak_log.h"
#include "ak_common_video.h"
#include "ak_venc.h"
#include "ak_thread.h"
#include "ak_mem.h"
#include "ak_vi.h"
#include "ak_video.h"
#include "ak_vpss.h"
#include "usb_webcam.h"
#include "video.h"

#define THREAD_PRIO -1
#define __STATIC__
#define PRINT(fmt, ...) printf("[video] " fmt, ##__VA_ARGS__)



#define VIDEO_MAX_COUNT         1

/*

#define ENC_TYPE_MAX            3
#define MAX_ENC_NUM             4

#define THIRD_CHN_SUPPORT       0

#define CHN_MODE                FRAME_MODE
#define DEF_FRAME_DEPTH         2
#define DEF_SLICE_DEPTH         4

#define VIDEO_DEV_MUX           2


#if CAMERA_OG01A1X
#define VIDEO_RESOLUTION        DE_VIDEO_1024P
#else
#define VIDEO_RESOLUTION        DE_VIDEO_1080P
#endif

decoder resolution */
/*static const struct resolution_t resolutions[] = {
#if CAMERA_OG01A1X
    {320,   240,   "DE_VIDEO_240P"},
    {480,   320,   "DE_VIDEO_320P"},
    {640,   480,   "DE_VIDEO_VGA"},
    {800,   480,   "DE_VIDEO_480P"},
    {1280,  720,  "DE_VIDEO_720P"},
    {1280,  1024, "DE_VIDEO_1024P"},
#else
//    {320,   240,   "DE_VIDEO_240P"},
//    {480,   320,   "DE_VIDEO_320P"},
//    {640,   480,   "DE_VIDEO_VGA"},
//    {800,   480,   "DE_VIDEO_480P"},
//    {1280,  720,  "DE_VIDEO_720P"},
    {1920,  1080,  "DE_VIDEO_1080P"},
//    {2560,  1440,  "DE_VIDEO_1440P"},
#endif
	{3840,	2880,  "DE_VIDEO_2880P"}
};

#define DE_VIDEO_SIZE_MAX       (ARRAY_SIZE(resolutions))
*/

struct video_param_t{
    unsigned short fps;
    unsigned int max_width;
    unsigned int max_height;
    int  dev_id;
    int  ch_id;
    char *isp_cfg;
    unsigned short goplen;
    unsigned short target_kbps;
    unsigned short max_kbps;
    BITRATE_CTRL_MODE_E br_mode;
    int venc_handle;

    struct webcam *cam;
    AK_PTHREAD_T uvc_ctrl_th;
    AK_PTHREAD_T uvc_stream_th;
    AK_MUTEX_T stream_lock;

	FW_STREAM_CB get_frame_fun;
};

static struct video_param_t video[VIDEO_MAX_COUNT] = {
{
#if CAMERA_OG01A1X
    .max_width = 1280,
    .max_height = 1024,
    .isp_cfg = "/etc/isp_og01a1x_mipi_2lane_av200_dual.conf",
#elif CAMERA_GC5603
    .max_width = 2944,
    .max_height = 1664,
    .isp_cfg = "/etc/config/isp_gc5603_mipi_2lane_av200.conf",
#endif
    .fps = 30,
    .dev_id = 0,
    .ch_id = 0,
    .goplen = 25*2,
    .target_kbps = 1024,
    .max_kbps = 1024,
    .br_mode  = BR_MODE_VBR,
}
};
#define  UVC_STREAMING_MAXPKT   (512)

static AK_S32 g_s32FrameRate = 30;      //default 30fps
static VI_RECTANGLE_S stViRectangle = {0};
static AK_S32 g_s32MainChnIndex = 0;    //main chn index
static AK_S32 g_s32SubChnIndex = 1;     //sub chn index
static AK_S32 g_s32UvcDevCnt = 1;



/* handle UVC gadget events: control requests and stream on/off */
static void *video_uvc_control_th(void *arg)
{
    AK_BASE_ThreadSetName("uvc_ctrl_th");
    struct video_param_t *video = (struct video_param_t *)arg; //venc handle
    struct pollfd pfd = { .fd = webcam_fd(video->cam), .events = POLLPRI };

    while (1) {
        if (poll(&pfd, 1, -1) < 0) {
            if (errno == EINTR)
                continue;
            PRINT("waiting for USB gadget events failed: %s\n", strerror(errno));
            break;
        }
        if (webcam_handle_events(video->cam) == WEBCAM_EVENT_ERROR) {
            PRINT("USB webcam stopped\n");
            break;
        }
    }
    AK_BASE_ThreadExit();
    return NULL;
}

static void *video_uvc_stream_th(void *arg)
{
    struct video_param_t *video = (struct video_param_t *)arg; //venc handle

    AK_BASE_ThreadSetName("uvc_stream_th");
    while (1) {
        fw_frame_info frame = {0};
        int ret;

        if (!webcam_is_streaming(video->cam)) {
            AK_BASE_SleepMs(5);
            continue;
        }
        /* webcam_send_frame() waits for the host to return a buffer */
        if (video->get_frame_fun(0, &frame, NULL) != 0 || !frame.pData || !frame.uiDataLen)
            continue;
        ret = webcam_send_frame(video->cam, frame.pData, frame.uiDataLen);
        if (ret < 0 && ret != -EAGAIN)
            PRINT("frame of %u bytes dropped: %s\n", frame.uiDataLen, strerror(-ret));
    }
    AK_BASE_ThreadExit();
    return NULL;
}
/* end of func */

int video_uvc_init(fw_video_enc_param *pvenc, FW_STREAM_CB get_frame_fun)
{
    int ret = AK_FAILED;

    for (int i=0; i<g_s32UvcDevCnt; i++)
    {
        struct webcam_config cfg = {
            .devnode = NULL,            /* first V4L2 output node: the UVC gadget */
            .width = pvenc->width,
            .height = pvenc->height,
            .fps = pvenc->framerate,
            .nbufs = 2,                 /* one with the host, one being filled */
            .max_payload = UVC_STREAMING_MAXPKT,
        };

        /* the gadget node must be open before either thread starts */
        video[i].cam = webcam_open(&cfg);
        if (video[i].cam == NULL)
            return AK_FAILED;
        video[i].get_frame_fun = get_frame_fun;

        ret = AK_BASE_ThreadCreate(&video[i].uvc_ctrl_th, video_uvc_control_th, &video[i], \
                ANYKA_THREAD_MIN_STACK_SIZE, THREAD_PRIO);
        if(ret != AK_SUCCESS){
            PRINT("cannot start the USB event thread\n");
            webcam_close(video[i].cam);
            video[i].cam = NULL;
            return ret;
        }

        /* from here on the event thread uses the webcam, so it stays open */
        ret = AK_BASE_ThreadCreate(&video[i].uvc_stream_th, video_uvc_stream_th, &video[i], \
                                ANYKA_THREAD_MIN_STACK_SIZE, THREAD_PRIO);
        if(ret != AK_SUCCESS){
            PRINT("cannot start the USB frame thread\n");
            return ret;
        }
    }

    return ret;
}
/* end of func */


int video_uvc_exit(void)
{
    int i;

    for (i = 0; i < g_s32UvcDevCnt; i++) {
        AK_BASE_ThreadJoin(video[i].uvc_stream_th);
        AK_BASE_ThreadJoin(video[i].uvc_ctrl_th);
        webcam_close(video[i].cam);
        video[i].cam = NULL;
    }
    return 0;
}

/* end of file */
