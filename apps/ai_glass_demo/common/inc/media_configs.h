/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd. */
#ifndef __MEDIA_CONFIG_H__
#define __MEDIA_CONFIG_H__

#include "ak_common.h"
#include "ak_venc.h"

#define SD_NAND_MOUNT_PATH  "/mnt"
#define SD_NAND_DEVICE_PATH  "/dev/mmcblk1p2"

#define TEMP_PATH           SD_NAND_MOUNT_PATH"/Temp"          // Temporary file directory
#define TEMP_AUDIO_PATH     SD_NAND_MOUNT_PATH"/TempAudio"     // Temporary audio file directory
#define LOG_PATH            SD_NAND_MOUNT_PATH"/Log"           // Log file directory
#define VIDEOS_PATH         SD_NAND_MOUNT_PATH"/Videos"        // Video file directory (Format: yyyymmddhhmmss, Type: .mp4)
#define TEMP_VIDEOS_PATH    SD_NAND_MOUNT_PATH"/TempVideos"    // Temporary video file directory (Format: yyyymmddhhmmss, Type: .mp4)
#define PICTURES_PATH       SD_NAND_MOUNT_PATH"/Pictures"      // Picture file directory (Format: yyyymmddhhmmss, Type: .jpeg)
#define AUDIO_PATH          SD_NAND_MOUNT_PATH"/Audio"         // Audio file directory (Format: yyyymmddhhmmss, Type: .mp3)
#define MUSIC_PATH          SD_NAND_MOUNT_PATH"/Music"         // Music file directory
#define SCHEDULE_PATH       SD_NAND_MOUNT_PATH"/Schedule"      // Schedule file directory
#define OTA_PATH            SD_NAND_MOUNT_PATH"/OTA"           // OTA upgrade file directory


#define MAX_CHAR_LEN    128



typedef struct
{
    int main_chn_width;       /**< Main channel width */
    int main_chn_height;      /**< Main channel height */
    int main_vi_width;        /**< Video input width */
    int main_vi_height;       /**< Video input height */
    int main_chn_fps;         /**< Frame rate */
    int main_chn_target_kbps; /**< Target bitrate in kbps */
    int main_chn_max_kbps;    /**< Maximum bitrate in kbps */
    int main_chn_gop_len;     /**< Group of Pictures (GOP) length */
    int main_chn_enc_type;    /**< Encoding type (e.g., H.264, H.265) */
    int main_chn_br_mode;     /**< Bitrate control mode */
	int eis_enable;           /**< EIS enable flag */
    int image_debug_mode;     /**< Image debug mode flag */
} ai_glass_record_params_t;


typedef struct
{
    int main_chn_width;       /**< Main channel width */
    int main_chn_height;      /**< Main channel height */
    int main_vi_width;        /**< Video input width */
    int main_vi_height;       /**< Video input height */
    int main_chn_fps;         /**< Frame rate */
    int main_chn_target_kbps; /**< Target bitrate in kbps */
    int main_chn_max_kbps;    /**< Maximum bitrate in kbps */
    int main_chn_gop_len;     /**< Group of Pictures (GOP) length */
    int main_chn_enc_type;    /**< Encoding type */
    int main_chn_br_mode;     /**< Bitrate control mode */
	int eis_enable;           /**< EIS enable flag */
} ai_glass_rtsp_params_t;

typedef struct
{
    int main_chn_fps;              /**< Capture frame rate */
    int main_org_width;            /**< Original width */
    int main_org_height;           /**< Original height */
    int main_res_width;            /**< Resolution width */
    int main_res_height;           /**< Resolution height */
    int main_jpg_slice_num;        /**< Number of JPEG slices */
    int main_org_slice_one_height; /**< Height of one original slice */
    int main_jpg_slice_one_height; /**< Height of one JPEG slice */
} ai_glass_photo_params_t;

typedef struct
{
    double data[3][3]; /**< EIS matrix data */
} ai_glass_eis_params_t;


typedef struct {
    char snap_isp_config[MAX_CHAR_LEN];  /**< Snapshot ISP config path */
    char video_isp_config[MAX_CHAR_LEN]; /**< Video ISP config path */

    ai_glass_record_params_t record_param; /**< Video recording parameters */
    ai_glass_rtsp_params_t rtsp_param;     /**< RTSP streaming parameters */
    ai_glass_photo_params_t photo_param;   /**< Photo capture parameters */
    ai_glass_eis_params_t eis_param;       /**< EIS parameters */
} GLASS_GlobalConfig_User;

extern GLASS_GlobalConfig_User gm_config;




typedef struct
{
    int dmesg;          /**< dmesg log level */
    int middle_log;     /**< Middleware log level */
    int rtt_log;        /**< RTT log level */
    int app_log;        /**< Application log level */
    int app_log_save;   /**< Flag to save app log */
    short power_off_time; /**< Auto power off time in seconds */
    char debug_ap_ssid[33];     /**< [DebugAp] ssid of the debug-mode AP (1-32 chars) */
    char debug_ap_password[64]; /**< [DebugAp] password: WPA2 passphrase of the debug-mode AP; empty = AP stays off */
} common_params_t;

typedef struct {
    common_params_t common_param; /**< Common parameters */
} GlobalConfig_User;

typedef struct {

    int main_frame_depth; /**< Frame depth */

    int crop_left;   /**< Crop left coordinate */
    int crop_top;    /**< Crop top coordinate */
    int crop_width;  /**< Crop width */
    int crop_height; /**< Crop height */

    int frame_rate;    /**< Frame rate */
    int input_pixfmt;  /**< Input pixel format */
    int output_pixfmt; /**< Output pixel format */
}vi_dev_info;



extern GlobalConfig_User g_config;

/**
 * @brief Retrieves extended video input device information.
 *
 * @param u32ViDev Video device ID.
 * @param dev_info Pointer to the structure to store the device info.
 * @return 0 on success, negative error code on failure.
 */
int get_vi_dev_info_ex(AK_U32 u32ViDev, vi_dev_info *dev_info);



/**
 * @brief Stop the RTSP live streaming service
 * @return int - 0 on success, negative value on error
 */
int stop_rtsp_live_stream(void);

/**
 * @brief RTSP thread main function for handling streaming operations
 * @param arg - Thread argument pointer
 * @return void* - Thread return value
 */
void *rtsp_thread_func(void *arg);

/**
 * @brief Start RTSP live streaming with EIS (Electronic Image Stabilization) enabled
 * @param isp_conf_path - Path to ISP configuration file
 * @return int - 0 on success, negative value on error
 */
int run_rtsp_live_stream_with_eis(char *isp_conf_path);

/**
 * @brief Start the RTSP live streaming service
 * @return int - 0 on success, negative value on error
 */
int start_rtsp_live_stream(void);

/**
 * @brief Stop video recording
 * @return int - 0 on success, negative value on error
 */
int stop_video_record(void);

/**
 * @brief Start taking photo/capture still image
 * @return int - 0 on success, negative value on error
 */
int start_take_photo(void);

/**
 * @brief Start video recording for specified duration
 * @param record_time - Recording duration in seconds
 * @return int - 0 on success, negative value on error
 */
int start_video_record(int record_time);

/**
 * @brief Start audio recording with specified parameters
 * @param record_duration - Total recording duration (in seconds or specific units)
 * @param record_min_duration - Minimum recording duration (in seconds or specific units)
 * @return int - 0 on success, negative value on error
 */
int start_audio_record(uint8_t record_duration, uint8_t record_min_duration);

/**
 * @brief Stop audio recording
 * @return int - 0 on success, negative value on error
 */
int stop_audio_record(void);

/**
 * @brief Exits the AI Glass demo application.
 *
 * Performs necessary cleanup and resource release before exiting.
 */
void ai_glass_demo_exit(void);

/**
 * @brief Checks if the USB cable has been removed.
 *
 * @return 1 if USB is removed, 0 otherwise.
 */
int get_usb_remove(void);

/**
 * @brief Detects USB connection status.
 *
 * @return 1 if USB is connected, 0 otherwise.
 */
int usbdetect_fun(void);

/**
 * @brief Prints the software version information.
 */
void ai_glass_print_sw_version(void);

/**
 * @brief Loads system configuration parameters from persistent storage.
 */
void load_configuration(void);

/**
 * @brief Loads camera-specific configuration parameters (resolution, FPS, etc.).
 */
void load_camera_configuration(void);

/**
 * @brief Loads EIS (Electronic Image Stabilization) calibration parameters.
 *
 * @return 0 on success, negative error code on failure.
 */
int load_eis_configuration(void);

#endif
