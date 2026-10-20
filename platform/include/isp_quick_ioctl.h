/* SPDX-License-Identifier: Apache-2.0 */
/**********************************************************************
 * @FILENAME isp_quick_ioctl.h
 * @BRIEF isp quick start device include files.
 * Copyright (C) 2021 Anyka Guangzhou
 * @DATE 2021-10-29
 * @REF
 **********************************************************************/

#ifndef __ISP_QUICK_IOCTL_H__
#define __ISP_QUICK_IOCTL_H__
//#include "dev_info.h"

//camera command
#define CMD_ISP_QUICK_START 'Q'

#define IO_ISP_QUICK_START_UPDATE_AE_TABLE  _IO(CMD_ISP_QUICK_START, 1)
#define IO_ISP_QUICK_START_GET_MISC_CONF    _IO(CMD_ISP_QUICK_START, 2)
#define IO_ISP_QUICK_START_SET_MISC_CONF    _IO(CMD_ISP_QUICK_START, 3)
#define IO_ISP_QUICK_START_GET_CHN_ATTR     _IO(CMD_ISP_QUICK_START, 4)
#define IO_ISP_QUICK_START_SET_CHN_ATTR     _IO(CMD_ISP_QUICK_START, 5)
#define IO_ISP_QUICK_START_SET_VENC_ATTR    _IO(CMD_ISP_QUICK_START, 6)
#define IO_ISP_QUICK_START_GET_VENC_ATTR    _IO(CMD_ISP_QUICK_START, 7)
#define IO_ISP_SWITCH_ISP_CAPTURE_MODE      _IO(CMD_ISP_QUICK_START, 8)
#define IO_ISP_QUICK_START_SWITCH_SENSOR    _IO(CMD_ISP_QUICK_START, 9)
#define IO_ISP_QUICK_START_REINIT_SENSOR    _IO(CMD_ISP_QUICK_START, 10)
#define IO_ISP_QUICK_START_FIRST_RUN_SENSOR _IO(CMD_ISP_QUICK_START, 11)
#define IO_ISP_STANDBY_IN_SENSOR            _IO(CMD_ISP_QUICK_START, 12)
#define IO_ISP_STANDBY_OUT_SENSOR           _IO(CMD_ISP_QUICK_START, 13)
#define IO_ISP_STANDBY_NIGTH_INFO           _IO(CMD_ISP_QUICK_START, 14)
#define IO_ISP_STANDBY_SENSOR_FSYNC         _IO(CMD_ISP_QUICK_START, 15)
#define IO_ISP_SET_CAPTURE_MODE             _IO(CMD_ISP_QUICK_START, 16)
#define IO_ISP_QUICK_GET_DEV_ATTR           _IO(CMD_ISP_QUICK_START, 17)
#define IO_ISP_QUICK_SET_DEV_ATTR           _IO(CMD_ISP_QUICK_START, 18)
#define IO_ISP_QUICK_START_SAVE_ATTR        _IO(CMD_ISP_QUICK_START, 19)
#define IO_ISP_QUICK_GET_GLASS_INFO         _IO(CMD_ISP_QUICK_START, 20)
#define IO_ISP_QUICK_SET_GLASS_INFO         _IO(CMD_ISP_QUICK_START, 21)

#if 0
#define CMD_NO_CMD                      0x00
#define CMD_PHOTO                       0x01
#define CMD_RECORD                      0x02
#define CMD_SNAPSHOT                    0x03
#define CMD_SMARTCONFIG                 0x04
#define CMD_OTA                         0x05
#define CMD_IMPORT                      0x06
#define CMD_P2P                         0x07
#define CMD_STOP_RECORD                 0x08
#define CMD_RTSP                        0x09
#define CMD_USBCAM                      0x0a
#define CMD_POWER_OFF_REQ               0x0b
#define CMD_SET_TIME                    0x0c
#define CMD_SET_TIMEZONE                0x0d
#define CMD_CHK_STORAGE_IS_FULL         0x0e
#define CMD_CHK_STORAGE_SPC             0x10
#define CMD_CHK_STORAGE_FILE_COUNT      0x11
#define CMD_AUDIO_RECORD                0x12
#define CMD_RECOVERY_FACTORY            0x13
#define CMD_FACTORY                     0x14
#define CMD_FEEDBACK                    0x15
#define CMD_AP                          0x16
#define CMD_AUDIO_RECORD_STOP           0x17
#define CMD_POWER_ON_NOTIFY             0x18
#define CMD_FTP_LOGIN_NOTIFY            0x19
#define CMD_FTP_LOGOUT_NOTIFY           0x1a
#define CMD_WIFI_STOP                   0x1b
#define CMD_WIFI_STA                    0x1c
#define CMD_WIFI_STA_GET_STATUS         0x1d
#define CMD_WIFI_STA_GET_IP             0x1e
#define CMD_WIFI_STA_CONNECT            0x1f
#define CMD_RTSP_LOGIN_NOTIFY           0x20
#define CMD_RTSP_LOGOUT_NOTIFY          0x21
#define CMD_GET_LINUX_VERSION           0x22
#define CMD_USB_MASS_STORAGE            0x23
#define CMD_GET_NUM_IMG_VIDEO           0x24
#define CMD_UMONT_SDNAND                0x25
#define CMD_SET_USB_WORK_MODE           0x26
#define CMD_LINUX_OTA_PROGRESS_NOTIFY   0x27
#define CMD_START_RTSP                  0x28
#define CMD_STOP_RTSP                   0x29
#define CMD_GET_STATUS                  0x30
#define CMD_START_HTTP                  0x31
#define CMD_STOP_HTTP                   0x32

//
#define IND_READY           0x70
#define IND_ACK             0x71
#define IND_BAUDRATE        0x72
#define IND_UPLOAD          0x73
#define IND_UPLOAD_PACK     0x74
#define IND_COMP            0x75
#else

//bt cmd
#define BT_CMD_NO_CMD                           0x00
#define BT_CMD_POWER_OFF_REQ                    0x01
#define BT_CMD_SET_LOCAL_TIME                   0x02
#define BT_CMD_USB_CONFIG                       0x03




#define BT_CMD_FACTORY                          0x73
#define BT_CMD_PHOTO                            0x74
#define BT_CMD_SNAPSHOT                         0x75
#define BT_CMD_RECORD                           0x76
#define BT_CMD_STOP_RECORD                      0x77
#define BT_CMD_RECORD_AUDIO                     0x78
#define BT_CMD_STOP_AUDIO                       0x79
#define BT_CMD_FEEDBACK                         0x7a
#define BT_IND_READY                            0x7b
#define BT_IND_ACK                              0x7c
#define BT_IND_UPLOAD_HEAD                      0x7d
#define BT_IND_UPLOAD_PACK                      0x7e
#define BT_IND_COMP                             0x7f


// app cmd
#define APP_CMD_NO_CMD                          0x80
#define APP_CMD_POWER_OFF_REQ                   0x81
#define APP_CMD_GET_STORAGE                     0x82
#define APP_CMD_GET_STORAGE_FILE_COUNT          0x83
#define APP_CMD_IMPORT                          0x84
#define APP_CMD_UNLOCK_BOUND                    0x85
#define APP_CMD_GET_VERSION                     0x86
#define APP_CMD_GET_PHOTO_VIDEO_COUNT           0x87
#define APP_CMD_AP                              0x88
#define APP_CMD_AP_DICONN                       0x89
#define APP_CMD_P2P                             0x8a
#define APP_CMD_P2P_DICONN                      0x8b
#define APP_CMD_OTA                             0x8c
#define APP_CMD_RCOVERY_FACTORY                 0x8d
#define APP_CMD_START_RTSP                      0x8e


#define APP_CMD_FACTORY                         0xf3
#define APP_CMD_PHOTO                           0xf4
#define APP_CMD_SNAPSHOT                        0xf5
#define APP_CMD_RECORD                          0xf6
#define APP_CMD_STOP_RECORD                     0xf7
#define APP_CMD_RECORD_AUDIO                    0xf8
#define APP_CMD_STOP_AUDIO                      0xf9
#define APP_CMD_FEEDBACK                        0xfa
#define APP_IND_READY                           0xfb
#define APP_IND_ACK                             0xfc
#define APP_IND_UPLOAD_HEAD                     0xfd
#define APP_IND_UPLOAD_PACK                     0xfe
#define APP_IND_COMP                            0xff
#endif

//BIT7 as LINUX APP/SYS COMM MASK, RESERVED FOR LINUX
#define FILE_OPERATE         0x80

struct qs_wb_gain {
    unsigned short r_gain;
    unsigned short g_gain;
    unsigned short b_gain;
    signed short r_offset;
    signed short g_offset;
    signed short b_offset;
};

struct qs_fast_ae_info {
    int sensor_exp_time;
    int sensor_a_gain;
    int sensor_d_gain;
    int isp_d_gain;
    struct qs_wb_gain wb;
    int dev_id;
    int reserved;
};

typedef enum
{
    NIGHT_MODE_IRFEED_LIGHT,
    NIGHT_MODE_WHITE_LIGHT,
    DAY_MODE_FORCE_NO_LIGHT,
    NIGHT_MODE_FORCE_IRFEED_LIGHT,
    DAY_MODE_IRFEED_LIGHT,
}T_NIGHT_MODE_LIGHT_SELECT;


typedef struct isp_quick_start_misc_info
{
    unsigned int daynight_adc_threshold;
    unsigned int light_sensor_enable;
    unsigned int curing_day_night_mode;
    unsigned int white_light_gpio_nr;
    T_NIGHT_MODE_LIGHT_SELECT night_mode_light_select;
    unsigned int mirror_en;
    unsigned int flip_en;
    unsigned int night2night_threshold;
    unsigned int orig_target_lumiance;
    unsigned int target_lumiance;
    unsigned int ae_strgy_type;
    unsigned char stitch_first_sensor;/*0->sensor0 is first, 1->sensor1 is first*/
    unsigned char reserved1;
    unsigned char reserved2;
    unsigned char reserved3;
}isp_quick_start_misc_info_t;

typedef struct isp_quick_start_chn_info
{
    unsigned int main_chn_width;
    unsigned int main_chn_height;
    unsigned int sub_chn_width;
    unsigned int sub_chn_height;
    unsigned int thd_chn_width;
    unsigned int thd_chn_height;
    int fps;
    unsigned int main_chn_enable;
    unsigned int sub_chn_enable;
    unsigned int thd_chn_enable;
    int crop_left;
    int crop_top;
    int crop_width;
    int crop_height;

    unsigned int main_crop_left;
    unsigned int main_crop_top;
    unsigned int main_crop_width;
    unsigned int main_crop_height;

    unsigned int sub_crop_left;
    unsigned int sub_crop_top;
    unsigned int sub_crop_width;
    unsigned int sub_crop_height;

    unsigned int thd_crop_left;
    unsigned int thd_crop_top;
    unsigned int thd_crop_width;
    unsigned int thd_crop_height;

}isp_quick_start_chn_info_t;

typedef struct qs_videv_info {
    isp_quick_start_chn_info_t chnn_info;

    int main_chn_mode;
    int sub_chn_mode;
    int thd_chn_mode;
    int main_chn_buf;
    int sub_chn_buf;
    int thd_chn_buf;
    int venc_encode_mode;

    int crop_left;
    int crop_top;
    int crop_width;
    int crop_height;

    int frame_rate;
    int input_pixfmt;
    int output_pixfmt;
}QS_VIDEV_INFO;

typedef struct quick_start_venc_attr
{
    unsigned int bind_vi_dev;
    unsigned int bind_vi_chn;
    unsigned int width;
    unsigned int height;
    unsigned int fps;
    unsigned int goplen;
    unsigned int target_kbps;
    unsigned int max_kbps;
    unsigned int profile;
    unsigned int br_mode;
    unsigned int initqp;       //recommend Dynamic bit rate parameter[0,100]
    unsigned int minqp;        //recommend Dynamic bit rate parameter[20,25]
    unsigned int maxqp;        //recommend Dynamic bit rate parameter[45,50]
    unsigned int jpeg_qlevel;
    unsigned int chroma_mode;
    unsigned int enc_out_type; //encode output type, h264 or jpeg or h265
    unsigned int max_picture_size;
    unsigned int enc_level;
    //0:disable smart, 1:mode of LTR, 2:mode of changing GOP length
    unsigned int smart_mode;
    unsigned int smart_goplen;       //smart goplen
    unsigned int smart_quality;      //smart quality
    unsigned int smart_static_value; //smart static value
    unsigned int gdr_mode;
    unsigned int max_frame_size;
}quick_start_venc_attr_t;

typedef struct isp_quick_start_ioctl_info {
    isp_quick_start_misc_info_t misc_info;
    int sensor_idex;
    int first_run_sensor_idex;//first run sensor
} isp_quick_start_ioctl_info_t;


typedef struct isp_quick_start_chn_ioctl_info {
    isp_quick_start_chn_info_t chnn_info;
    int sensor_idex;
    int first_run_sensor_idex;//first run sensor
} isp_quick_start_chn_ioctl_info_t;


typedef struct isp_quick_start_night_info {
    unsigned int mode;   //1->nigth, 0->day
    unsigned int led_ctl_mode; //1->pwm control,0->gpio control
    unsigned int pin;          //GPIO control, pin is GPIO, if PWM control, it is PWM ID (0~4)
    unsigned int sensor_idex;
} isp_quick_start_night_info_t;

typedef struct isp_quick_start_switch_info {
    unsigned int capture_mode;//1->continue, 0->single
    unsigned int sensor_idex;
} isp_quick_start_switch_info_t;

typedef struct quick_start_venc_ioctl_info {
    quick_start_venc_attr_t attr;
    int chn_id;
} quick_start_venc_ioctl_info_t;

typedef struct quick_start_dev_ioctl_info {
    struct qs_videv_info dev_info;
    int dev_id;
} quick_start_dev_ioctl_info_t;

typedef struct quick_glass_info {
    unsigned int cmd;
    unsigned int param_len;
    unsigned int crc;

    uint8_t parameter[255];  // Parameter content
} quick_glass_info_t;

typedef struct{
    uint8_t ap_ssid[64];
    uint8_t ap_password[64];
 }ak_glass_wifi_ap_t;

typedef struct {
    int32_t connnect_status;
    uint8_t wifi_ssid[64];
    uint8_t wifi_password[64];
} ak_glass_wifi_status_t;

typedef struct {
    int32_t status;
    uint8_t wifi_ip[32];
} ak_glass_wifi_ip_t;

#pragma pack(1)
// Date time structure
typedef struct {
    uint16_t year;      // Year (e.g. 2023)
    uint8_t month;      // Month (1-12)
    uint8_t day;        // Day (1-31)
    uint8_t hour;       // Hour (0-23)
    uint8_t min;        // Minute (0-59)
    uint8_t sec;        // Second (0-59)
} ak_glass_datetime_t;

// Timezone structure (in minutes)
typedef struct {
    int16_t minutes;    // Timezone offset minutes (e.g. -480 means UTC-8)
} ak_glass_timezone_t;


typedef struct {
    ak_glass_timezone_t timezone;   // Timezone offset
    ak_glass_datetime_t datetime;   // Date time
} ak_glass_localtime_t;



//p2p control command structure
typedef struct {
    uint8_t state; //Bind: 0x00 Import map: 0x01 Live: 0x02
    char p2p_dev_name[12];
} ak_glass_p2p_config_t;

//usb work mode control command structure
typedef struct {
    uint8_t mode; // 1:U-disk  2:uvc debug photo 3:uvc debug record
} ak_glass_usb_mode_config_t;

#pragma pack()

// File type query enum
typedef enum
{
    SYNC_FILE_TYPE_ALL = 0,
    SYNC_FILE_TYPE_BEGIN,   // begin
    SYNC_FILE_TYPE_VIDEOS = 1,
    SYNC_FILE_TYPE_PICTURES = 2,
    SYNC_FILE_TYPE_AUDIO = 3,
    SYNC_FILE_TYPE_END,     // end
} sync_file_type_e;

typedef struct {
    int16_t file_count_type;
    int16_t file_count;
} file_count_t;


typedef struct {
    int8_t sw_version_major;
    int8_t sw_version_minor;
    int8_t sw_version_patch;
} ai_glass_version_t;

/* Upgrade start info */
typedef struct {
    uint8_t ota_file_path[128];                  /*OTA file path*/
} ak_ota_info_t;

/* Upgrade statistics info */
typedef struct {
    int32_t ota_status;                 /*Status 0 normal, others abnormal */
    uint32_t progress;                  /*Progress [0-10000]*/
} ak_ota_stats_t;

#endif

/*end of file*/
