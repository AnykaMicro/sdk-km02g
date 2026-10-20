/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd. */
#ifndef _AI_GLASS_ISP_QUICK_H__
#define _AI_GLASS_ISP_QUICK_H__

#include "media_configs.h"

/**
 * @brief Initializes the ISP (Image Signal Processor) quick start module.
 *
 * @return 0 on success, negative error code on failure.
 */
int ai_glass_isp_quick_init(void);

/**
 * @brief De-initializes the ISP quick start module.
 *
 * @return 0 on success, negative error code on failure.
 */
int ai_glass_isp_quick_deinit(void);

/**
 * @brief Retrieves video input device information.
 *
 * @param u32ViDev Video device ID.
 * @param dev_info Pointer to the structure to store device information.
 * @return 0 on success, negative error code on failure.
 */
int get_vi_dev_info(AK_U32 u32ViDev, QS_VIDEV_INFO *dev_info);

/**
 * @brief Retrieves video encoder attribute information.
 *
 * @param venc_chn_id Video encoder channel ID.
 * @param venc_info Pointer to the structure to store encoder attributes.
 * @return 0 on success, negative error code on failure.
 */
int get_venc_attr(AK_U32 venc_chn_id, quick_start_venc_attr_t *venc_info);

/**
 * @brief Retrieves extended video input device information.
 *
 * @param u32ViDev Video device ID.
 * @param info Pointer to the structure to store extended device information.
 * @return 0 on success, negative error code on failure.
 */
int get_vi_dev_info_ex(AK_U32 u32ViDev, vi_dev_info *info);


#endif
