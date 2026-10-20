/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd. */
#ifndef __AK_SNAP_DEMO_H__
#define __AK_SNAP_DEMO_H__

/**
 * @brief Takes a snapshot and saves it to a file.
 *
 * @param photo_save_path Path where the snapshot will be saved.
 * @return int 0 on success, negative error code on failure.
 */
int ak_snap_photo(const char *photo_save_path);

/**
 * @brief Initializes the snapshot environment.
 *
 * @param isp_conf_path Path to the ISP configuration file.
 * @param i_fps Frame rate setting.
 * @param i_frame_depth Frame depth setting.
 * @return int 0 on success, negative error code on failure.
 */
int ak_snap_env_init(char* isp_conf_path, int i_fps, int i_frame_depth);

/**
 * @brief Destroys the snapshot environment.
 *
 * @return int 0 on success, negative error code on failure.
 */
int ak_snap_env_destory(void);

/**
 * @brief Enables or disables the snapshot environment (optionally UVC mode).
 *
 * @param bUVC Flag to enable UVC mode (1 for UVC, 0 for local).
 * @return int 0 on success, negative error code on failure.
 */
int ak_snap_env_enable(int bUVC);

// int ak_sdk_env_init();
// int ak_sdk_env_destory();

#endif