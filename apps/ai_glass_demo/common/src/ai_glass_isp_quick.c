/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>
#include <getopt.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <sys/time.h>
#include <time.h>
#include <linux/netlink.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "ak_common.h"

#include "isp_quick_ioctl.h"

#include "ai_glass_isp_quick.h"
#include "fun_log.h"
#include "ai_glass_common.h"

AK_S32 isp_quick_fd = -1;


int ai_glass_isp_quick_init()
{
	// get quick_start fd
    isp_quick_fd = open("/dev/quick_start", O_RDWR, 0);
	if (isp_quick_fd < 0)
    {
        log_error_app("open isp quick start device fail, run on normal mode\n");
		return -1;
    }

	return 0;
}

int ai_glass_isp_quick_deinit()
{
	if (isp_quick_fd >= 0)
    {
        close(isp_quick_fd);
        isp_quick_fd = -1;
    }

	return 0;
}


int get_vi_dev_info(AK_U32 u32ViDev, QS_VIDEV_INFO *dev_info)
{
    quick_start_dev_ioctl_info_t ioctl_dev_info = {0};log_info_app("here QS_VIDEV_INFO len=%zu\n", sizeof(QS_VIDEV_INFO));

    if (ioctl(isp_quick_fd, IO_ISP_QUICK_GET_DEV_ATTR, &ioctl_dev_info) < 0)
    {
        log_error_app("IO_ISP_QUICK_GET_DEV_ATTR err!\n");
        return -1;
    }

    memcpy(dev_info, &ioctl_dev_info.dev_info, sizeof(QS_VIDEV_INFO));log_info_app("here\n");

    return 0;
}

int get_venc_attr(AK_U32 venc_chn_id, quick_start_venc_attr_t *venc_info)
{
    quick_start_venc_ioctl_info_t ioctl_venc_info = {0};log_info_app("here quick_start_venc_attr_t len=%zu\n",sizeof(quick_start_venc_attr_t));
    ioctl_venc_info.chn_id = venc_chn_id;log_info_app("here\n");

    if (ioctl(isp_quick_fd, IO_ISP_QUICK_START_GET_VENC_ATTR, &ioctl_venc_info) < 0)
    {
        log_error_app("QUICK_START_GET_VENC_ATTR err!\n");
        return -1;
    }

    memcpy(venc_info, &ioctl_venc_info.attr, sizeof(quick_start_venc_attr_t));log_info_app("here\n");
	log_info_app("here\n");

    return 0;
}

extern qs_demo_dev_t qs_dev;
int get_vi_dev_info_ex(AK_U32 u32ViDev,vi_dev_info *info)
{
	static int run = 0;

	if(run == 0)
	{
	    quick_start_dev_ioctl_info_t ioctl_dev_info = {0};

	    if (ioctl(isp_quick_fd, IO_ISP_QUICK_GET_DEV_ATTR,&ioctl_dev_info ) < 0)
	    {
	        log_error_app("IO_ISP_QUICK_GET_DEV_ATTR err!\n");
	        return -1;
	    }

	    memcpy(&qs_dev.dev_info[u32ViDev], &ioctl_dev_info.dev_info, sizeof(QS_VIDEV_INFO));

        run = 1;
	}

	info->main_frame_depth = qs_dev.dev_info[u32ViDev].main_chn_buf;
	info->frame_rate = qs_dev.dev_info[u32ViDev].frame_rate;

    return 0;
}
