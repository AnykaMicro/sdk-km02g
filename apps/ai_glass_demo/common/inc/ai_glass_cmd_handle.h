/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd. */
#ifndef _AI_GLASS_CMD_HANDLE_H__
#define _AI_GLASS_CMD_HANDLE_H__

#ifdef __cplusplus
extern "C" {
#endif


#include "ai_glass_common.h"
extern char g_eis_data_file_name[256];


#define OTA_PATH_NAME "/mnt/OTA/KM02G_OTA_V0.0.0.bin"

/**
 * @brief Handles setting the system timezone.
 *
 * @param glass_info Pointer to the structure containing the timezone offset (in minutes).
 */
void handle_set_timezone(const quick_glass_info_t *glass_info);

/**
 * @brief Sets the system date and time.
 *
 * @param glass_info Pointer to the structure containing year, month, day, hour, minute, and second.
 */
void handle_set_time(const quick_glass_info_t *glass_info);

/**
 * @brief Sets the local system time.
 *
 * @param glass_info Pointer to the structure containing local time information.
 */
void handle_set_local_time(const quick_glass_info_t *glass_info);

/**
 * @brief Retrieves and reports the count of files in storage.
 *
 * @param glass_info Pointer to the structure specifying the file type to count (e.g., video, photo).
 */
void handle_get_storage_file_count(const quick_glass_info_t *glass_info);

/**
 * @brief Performs a factory reset.
 *
 * This operation formats the SD card/NAND flash and restores default configurations.
 *
 * @param glass_info Pointer to the structure triggering the factory reset command.
 */
void handle_recovert_factory(const quick_glass_info_t *glass_info);

/**
 * @brief Gets the current IP address of the Wi-Fi station interface.
 *
 * @param glass_info Pointer to the structure to store or request the IP address information.
 */
void handle_get_wifi_ip(const quick_glass_info_t *glass_info);

/**
 * @brief Retrieves the connection status and SSID of the current Wi-Fi network.
 *
 * @param glass_info Pointer to the structure to store or request the Wi-Fi status.
 */
void handle_get_sta_wifi_status(const quick_glass_info_t *glass_info);

/**
 * @brief Stops the Wi-Fi connection and disconnects from the current network.
 *
 * @param glass_info Pointer to the device information structure.
 */
void handle_wifi_stop(const quick_glass_info_t *glass_info);

/**
 * @brief Connects to a specified Wi-Fi network in Station (STA) mode.
 *
 * @param glass_info Pointer to the device information structure containing SSID and password.
 */
void handle_wifi_sta_connect(const quick_glass_info_t *glass_info);

/**
 * @brief Starts the RTSP streaming service.
 *
 * Allows remote clients to view the live video feed.
 *
 * @param glass_info Pointer to the device information structure.
 */
void handle_start_rtsp(const quick_glass_info_t *glass_info);

/**
 * @brief Stops the RTSP streaming service.
 *
 * Releases associated resources and closes connections.
 *
 * @param glass_info Pointer to the device information structure.
 */
void handle_stop_rtsp(const quick_glass_info_t *glass_info);

/**
 * @brief Stops the current video recording session.
 *
 * @param glass_info Pointer to the device information structure.
 */
void handle_stop_video_record(const quick_glass_info_t *glass_info);

/**
 * @brief Starts a new video recording.
 *
 * The video will be saved to the local storage.
 *
 * @param glass_info Pointer to the device information structure.
 */
void handle_start_video_record(const quick_glass_info_t *glass_info);

/**
 * @brief Captures a single photo.
 *
 * The photo will be saved to the local storage.
 *
 * @param glass_info Pointer to the device information structure.
 */
void handle_take_photo(const quick_glass_info_t *glass_info);

/**
 * @brief Powers off the device.
 *
 * Performs system shutdown procedures including data synchronization and peripheral deinitialization.
 *
 * @param glass_info Pointer to the device information structure.
 */
void handle_power_off(const quick_glass_info_t *glass_info);

/**
 * @brief Configures the device to operate in Access Point (AP) mode.
 *
 * Allows other devices to connect directly to this device.
 *
 * @param glass_info Pointer to the device information structure.
 */
void handle_wifi_ap_mode(const quick_glass_info_t *glass_info);

/**
 * @brief Enables Wi-Fi Peer-to-Peer (P2P) mode.
 *
 * Enables direct device-to-device communication.
 *
 * @param glass_info Pointer to the device information structure.
 */
void handle_wifi_p2p_mode(const quick_glass_info_t *glass_info);

/**
 * @brief Starts audio recording.
 *
 * Records audio from the microphone and saves it as a file.
 *
 * @param glass_info Pointer to the device information structure.
 */
void handle_start_audio_record(const quick_glass_info_t *glass_info);

/**
 * @brief Stops the current audio recording.
 *
 * @param glass_info Pointer to the device information structure.
 */
void handle_stop_audio_record(const quick_glass_info_t *glass_info);

/**
 * @brief Counts the total number of image and video files.
 *
 * Scans the storage device for media files and returns the total count.
 *
 * @return The total number of image and video files found.
 */
int handle_get_num_img_video(void);

/**
 * @brief Handles the OTA (Over-The-Air) upgrade process.
 *
 * This includes version checking, firmware download, verification, and installation.
 *
 * @param glass_info Pointer to the device information structure.
 */
void handle_ota(const quick_glass_info_t *glass_info);

/**
 * @brief Automatically updates panoramic image processing.
 *
 * Monitors and updates panoramic camera data streams in real-time, executing image stitching,
 * distortion correction, and view optimization.
 */
void u_pan_auto_update(void);

/**
 * @brief Retrieves the current device work mode.
 *
 * Processes work mode requests, parses the current operating state, and configures display parameters.
 *
 * @param glass_info Pointer to the device information structure.
 */
void handle_get_work_mode(const quick_glass_info_t *glass_info);

/**
 * @brief Sets the USB mode.
 *
 * Configures USB endpoints based on the requested mode (e.g., Mass Storage, UVC).
 *
 * @param glass_info Pointer to the device information structure containing the requested USB mode.
 * @return 0 on success, negative error code on failure.
 */
int handle_usb_mode_set(const quick_glass_info_t *glass_info);

/**
 * @brief Retrieves storage information.
 *
 * Gets total and available space on the storage device.
 *
 * @param glass_info Pointer to the device information structure to store the storage info.
 * @return 0 on success, negative error code on failure.
 */
int handle_get_storage(const quick_glass_info_t *glass_info);

/**
 * @brief Deinitializes the command handling module.
 *
 * Releases resources allocated by the command handler.
 *
 * @return 0 on success, negative error code on failure.
 */
int handle_cmd_deinit(void);



#ifdef __cplusplus
}
#endif

#endif
