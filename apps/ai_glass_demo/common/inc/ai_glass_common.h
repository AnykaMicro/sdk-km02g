/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd. */


#ifndef _AI_GLASS_COMMON_H__
#define _AI_GLASS_COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>

#include "ak_types.h"
#include "isp_quick_ioctl.h"
#include "ak_log.h"
#include "ak_common.h"
#include "ai_glass_log.h"
#include "media_configs.h"
#include "ai_glass_cmd_queue.h"
#include "ai_glass_time.h"
#include "sdnand_manager.h"
#include "wifi_manager.h"
#include "ai_glass_cmd_handle.h"
#include "ai_glass_cmd_ipi.h"


#define CAMERA_SRC_FILE     "/mnt/camera.ini"
#define CAMERA_DEST_FILE    "/etc/config/camera.ini"
#define CAMERA_BACKUP_FILE  "/etc/config/camera.ini.bak"


#define APP_SRC_FILE     "/mnt/app_main_config.ini"
#define APP_DEST_FILE    "/etc/config/app_main_config.ini"
#define APP_BACKUP_FILE  "/etc/config/app_main_config.ini.bak"

#define GET_GLASS_INFO              0x10
#define SET_GLASS_INFO              0x11
#define NOTIFY_GLASS_INFO           0x12
#define NOTIFY_USER_READY           0x13

#define VIDEO_MAX_NUM 3
#define VIDEO_DEV0 0
#define VIDEO_DEV1 1

#define P2P_CONNECT_MAX_RETRIES 1000
#define P2P_SH_PATH "/fast/usr/sbin/app_go_group.sh"
#define AP_SH_PATH "/fast/usr/sbin/wifi_driver.sh"
#define WAIT_SDNAND_MAX_RETRIES 1000

//extern int glass_fd;
extern int ota_run_flag;
extern int global_cmd_id;

typedef struct qs_demo_dev
{
    struct qs_videv_info dev_info[VIDEO_MAX_NUM];
    quick_start_venc_attr_t venc_chn[VIDEO_MAX_NUM];
} qs_demo_dev_t;


// RTSP and recording thread parameter structure
typedef struct
{
    char isp_conf_path[256]; /**< Path to the ISP configuration file */
} rtsp_thread_params_t;

typedef struct
{
    char isp_conf_path[256];      /**< Path to the ISP configuration file */
    unsigned int record_direction; /**< Recording orientation 0: Horizontal 1: Vertical */
    unsigned int record_duration;  /**< Maximum recording duration in seconds */
    unsigned int record_min_duration; /**< Minimum recording duration in seconds */
    int frame_depth;              /**< Video frame depth */
} record_thread_params_t;

enum usb_mode
{
    USB_MODE_CHARGE = 0, /**< Charging only mode */
    USB_MODE_STROGE = 1, /**< Mass storage mode (MSC) */
    USB_MODE_UVC    = 2  /**< USB Video Class mode (Webcam) */
};
#pragma pack(1)
typedef struct {
    char error_code;     /**< Error code returned by the operation */
    short time;          /**< Time information */
    char reserved;       /**< Reserved for future use */
} ReturnStruct;
#pragma pack()


typedef enum work_mode
{
    WORK_MODE_IDLE      = -1, /**< Idle state */
    WORK_MODE_PHOTO     = 0,  /**< Taking photo */
    WORK_MODE_STOGE     = 1,  /**< Mass storage mode */
    WORK_MODE_UVC       = 2,  /**< UVC camera mode */
    WORK_MODE_RECORD    = 3,  /**< Video recording */
    WORK_MODE_P2P       = 4,  /**< P2P download file */
    WORK_MODE_RTSP      = 5,  /**< RTSP video preview */
    WORK_MODE_HTTP      = 6,  /**< HTTP server mode */
    WORK_MODE_OTA       = 7,  /**< OTA upgrade mode */
    WORK_MODE_AUDIO     = 8,  /**< Audio recording */
    WORK_MODE_FACTORY   = 9   /**< Factory reset mode */

}work_mode_t;

typedef enum {
    RETURN_CODE_SUCCESS                = 0x00,         // Success
    RETURN_CODE_PARAM_INVALID          = 0x01,         // Invalid parameter
    RETURN_CODE_PARAM_CONFIG_FAILED    = 0x02,         // Parameter configuration failed
    RETURN_CODE_RECORD_TIME_SHORT      = 0x03,         // Recording time too short
    RETURN_CODE_RECORD_TIME_LONG       = 0x04,         // Recording time too long
    RETURN_CODE_FILE_WRITE_FAILED      = 0x05,         // File write failed
    RETURN_CODE_FILE_READ_FAILED       = 0x06,         // File read failed
    RETURN_CODE_FILE_CREATE_FAILED     = 0x07,         // File creation failed
    RETURN_CODE_MEMORY_ALLOC_FAILED    = 0x08,         // Memory allocation failed
    RETURN_CODE_DEVICE_BUSY            = 0x09,         // Device busy
    RETURN_CODE_TIMEOUT                = 0x0A,         // Timeout
    RETURN_CODE_SYSTEM_NOT_MOUNTED     = 0x0B,         // SD nand not mounted
    RETURN_CODE_WIFI_NODE_FAILED       = 0x0C,         // WiFi node failed
    RETURN_CODE_UMOUNT_FAILED          = 0x0D,         // Unmount SD nand failed
    RETURN_CODE_DEVICE_NO_USE          = 0x0E,         // Device not running
    RETURN_CODE_SD_NAND_SPACE_FULL     = 0x0F,         // Device space full
    RETURN_CODE_CREATE_FAIL            = 0x10,         // Creation failed
    RETURN_CODE_SD_NAND_FORMART_FAIL   = 0x11,         // Format failed
    RETURN_CODE_SD_NAND_NO_FILES       = 0x12,         // No photo/video files in card
    RETURN_CODE_UNKNOWN_ERROR          = 0xFF          // Unknown error
} return_code_t;


/**
 * @brief Destroys and cleans up the SDK environment resources.
 *
 * @return 0 on success, negative value on failure.
 */
int ak_sdk_env_destory();

/**
 * @brief Sets the brightness level of the specified LED.
 *
 * @param led_name Name of the LED device (e.g., "red", "blue").
 * @param brightness Brightness value (0-255).
 * @return 0 on success, negative value on failure.
 */
int set_led_brightness(const char *led_name, int brightness);

/**
 * @brief Configures the flash pattern (blink) of the specified LED.
 *
 * @param led_name Name of the LED device (e.g., "red", "blue").
 * @param on_time Duration in milliseconds the LED stays ON.
 * @param off_time Duration in milliseconds the LED stays OFF.
 * @return 0 on success, negative value on failure.
 */
int set_led_flash(const char *led_name, int on_time, int off_time);

/**
 * @brief Saves current system configuration parameters to persistent storage.
 */
void set_configuration(void);

/**
 * @brief Saves the FasyLog system log to a file.
 *
 * @return 0 on success, negative error code on failure.
 */
int save_fasylog_to_file(void);

/**
 * @brief Saves the kernel ring buffer (dmesg) to a file.
 *
 * @return 0 on success, negative error code on failure.
 */
int save_dmesg_to_file(void);

/**
 * @brief Saves the application log to a file.
 *
 * @return 0 on success, negative error code on failure.
 */
int save_app_log_to_file(void);

/**
 * @brief Sets the current working mode of the device.
 *
 * @param work_mode The new working mode to set.
 */
void set_work_mode(work_mode_t work_mode);

/**
 * @brief Retrieves the current working mode of the device.
 *
 * @return The current working mode.
 */
int get_work_mode(void);

/**
 * @brief Checks if the 7th bit of a byte is set.
 *
 * @param hex_value The byte value to check.
 * @return 1 if bit 7 is set, 0 otherwise.
 */
int is_bit7_set(unsigned char hex_value);

/**
 * @brief Appends data to a file.
 *
 * @param filename Path to the file.
 * @param pdata Pointer to the data to write.
 * @param len Length of the data to write.
 * @return Number of bytes written on success, negative error code on failure.
 */
int append_write_file(char *filename, char* pdata, int len);

/**
 * @brief Ensures that a directory exists, creating it if necessary.
 *
 * @param path Path to the directory.
 * @return 0 on success, negative error code on failure.
 */
int ensure_directory_exists(const char *path);

/**
 * @brief Checks the integrity of the camera configuration file.
 *
 * @return 0 if valid, negative error code if invalid or missing.
 */
int check_camera_ini(void);

/**
 * @brief Checks the integrity of the application configuration file.
 *
 * @return 0 if valid, negative error code if invalid or missing.
 */
int check_app_ini(void);

/**
 * @brief Renames a file safely.
 *
 * @param src Source file path.
 * @param dst Destination file path.
 * @return 0 on success, negative error code on failure.
 */
int ak_rename_file(const char *src, const char *dst);



#ifdef __cplusplus
}
#endif

#endif // _AI_GLASS_COMMON_H__
