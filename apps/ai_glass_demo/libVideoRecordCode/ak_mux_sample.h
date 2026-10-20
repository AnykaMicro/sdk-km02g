/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd. */
/**
 * @file ak_mux_sample.h
 * @brief Header file for muxing and recording sample application.
 *
 * This file defines the constants, data structures, and function prototypes
 * for the video/audio recording and muxing module.
 */

#ifndef _AK_MUX_SAMPLE_H_
#define _AK_MUX_SAMPLE_H_

#include <stdatomic.h>

#include "ak_circular_buffer.h"
#include "ak_list.h"

/** @brief Lock-free ring buffer size (must be a power of 2). */
#define LOCKFREE_BUFFER_SIZE 256

#define LEN_CMD 2048                      /**< Command length */
#define LEN_FILE_UNION 2048               /**< Union file name length */
#define LEN_FILE 1024                     /**< File path length */
#define LEN_TEMPLATE 512                  /**< Template length */
#define LEN_HINT 512                      /**< Hint length */
#define LEN_NUM 16                        /**< Integer number string length */
#define LEN_EXT 8                         /**< Extension length */
#define LEN_OPTION_SHORT 512              /**< Short option length */
#define LEN_BUFF_RECORD (2 * 1024 * 1024) /**< Recording buffer length (2MB) */
#define LEN_BUFF_INDEX 0                  /**< Index buffer length is 0, currently needs immediate writing because recording repair requires complete index file */
#define LEN_BUFF_VIDEO (8 * 1024 * 1024)  /**< Video buffer length to be synthesized, default 8MB */
#define LEN_BUFF_AUDIO (256 * 1024)       /**< Audio buffer length to be synthesized, default 256KB */
#define LEN_CONF_RES 8192                 /**< Configuration file length */
#define LEN_PATTERN 1024                  /**< Pattern length */
#define LEN_FIX_NAME 18                   /**< File name of the repaired file */
#define HANDLE_NOT_INIT -1                /**< Handle not initialized */
#define HANDLE_INIT 0                     /**< Handle initialized */

#define NUM_NOT_INIT -1                   /**< Number not initialized */
#define RECORD_PATH_SAVE VIDEOS_PATH      /**< Default save path */
#define RECORD_PATH_TMP TEMP_VIDEOS_PATH  /**< Default temp path */
#define FILE_EXT_MP4 ".mp4"               /**< MP4 file extension */
#define FILE_EXT_AVI ".avi"               /**< AVI file extension */

#define AI_VOLUME 6                       /**< Audio input volume */
#define AENC_CHANNEL_NUM 1                /**< Audio encoder channel number */
#define AENC_SAMPLE_RATE 8000             /**< Audio encoder sample rate */
#define VI_WIDTH 1920                     /**< Video input width */
#define VI_HEIGHT 1440                    /**< Video input height */
#define VI_SUB_WIDTH 640                  /**< Video input sub-stream width */
#define VI_SUB_HEIGHT 480                 /**< Video input sub-stream height */
#define VI_FPS 30                         /**< Video input frame rate */
#define VI_FRAME_DEPTH 4                  /**< VI captured frame buff quantity. Setting to 2 may cause insufficient capture frame rate. */
#define VI_FRAME_DEPTH_MIN 2              /**< Minimum VI frame depth */
#define VI_FRAME_DEPTH_MAX 4              /**< Maximum VI frame depth */

#define VENC_GOP_LEN 50                   /**< Video encoder GOP length */
#define VENC_TARGET_KBPS 800              /**< Video encoder target bitrate (kbps) */
#define VENC_MAX_KBPS 1024                /**< Video encoder max bitrate (kbps) */
#define VENC_MINQP 25                     /**< Video encoder min QP */
#define VENC_MAXQP 50                     /**< Video encoder max QP */
#define VENC_INITQP ((VENC_MINQP + VENC_MAXQP) / 2) /**< Video encoder initial QP */
#define VENC_LEVEL 30                     /**< Video encoder level */
#define VENC_SMART_MODE 0                 /**< Video encoder smart mode */
#define VENC_SMART_GOPLEN 100             /**< Video encoder smart GOP length */
#define VENC_SMART_QUALITY 50             /**< Video encoder smart quality */
#define RECORD_SEC 10                     /**< Recording file duration in seconds */
#define RECORD_NUM 1                      /**< Number of recordings */
#define MONITOR_SEC 10                    /**< Monitor detection thread sleep duration */
#define ISP_CONF_FILE "/etc/config/isp.conf" /**< ISP configuration file path */
#define S2MS 1000                         /**< Seconds to Milliseconds conversion */
#define MS2US 1000                        /**< Milliseconds to Microseconds conversion */
#define S2US 1000000                      /**< Seconds to Microseconds conversion */
#define MS_CUT_ADD 100                    /**< Add this duration when judging whether the current video recording duration is met */
#define TIMES_CUT_LOOP 10                 /**< Polling times if there is no aenc data when truncating video */
#define MIN_AUDIO_INTERVAL 32             /**< Minimum audio interval */

// #define PATH_TEMPLATE "%Y%M%D"           // Path template
// #define FILE_TEMPLATE "%Y%M%D.%h%m%s.%S" // File name template
//                                          //%Y - Year
//                                          //%M - Month
//                                          //%D - Day
//                                          //%h - Hour
//                                          //%m - Minute
//                                          //%s - Second
//                                          //%S - Recording length ms

// #define PATH_TEMPLATE "%Y%M%D"           // Path template
#define PATH_TEMPLATE ""                    /**< Path template */
#define FILE_TEMPLATE "%Y%M%D%h%m%s"        /**< File name template */
                                            //%Y - Year
                                            //%M - Month
                                            //%D - Day
                                            //%h - Hour
                                            //%m - Minute
                                            //%s - Second

#ifndef VIDEO_DEV0
#define VIDEO_DEV0 0
#endif

/**
 * @brief Video channel enum (Temporarily defined).
 */
enum ak_vi_chn_enum
{
    VIDEO_CHN0 = 0,
    VIDEO_CHN1,
    VIDEO_CHN2,
    VIDEO_CHN3,
    VIDEO_CHN_TOTAL,
    VIDEO_CHN16 = 16,
    VIDEO_CHN17,
    VIDEO_CHN24 = 24,
    VIDEO_CHN25,
    VIDEO_CHN26,
    VIDEO_CHN31 = 31
};

/**
 * @brief File template save element index.
 */
enum dvr_template_type
{
    FILE_TEMPLATE_YEAR,
    FILE_TEMPLATE_MONTH,
    FILE_TEMPLATE_DAY,
    FILE_TEMPLATE_HOUR,
    FILE_TEMPLATE_MIN,
    FILE_TEMPLATE_SEC,
    FILE_TEMPLATE_MS,
    FILE_TEMPLATE_NUM,
};

/**
 * @brief Recording thread enumeration.
 */
enum thread_serial
{
    THREAD_VI_VENC, /**< Video capture and encoding thread */
    THREAD_AI,      /**< Audio input thread */
    THREAD_AENC,    /**< Audio encoding thread */
    THREAD_MUX,     /**< Muxing thread */
    THREAD_NUM,     /**< Total number of threads */
};

/**
 * @brief DVR date and time structure.
 */
struct dvr_date
{
    int year;     /**< Year (4 digits) */
    int mon;      /**< Month (1-12) */
    int day;      /**< Day (1-31) */
    int hour;     /**< Hour (0-23) */
    int min;      /**< Minute (0-59) */
    int sec;      /**< Second (0-59) */
    int timezone; /**< Local time zone (0-23) */

    struct timeval timeval_val;   /**< Time value (tv_sec, tv_usec) */
    struct timezone timezone_val; /**< Timezone struct */
    struct tm tm_val;             /**< TM struct */
};

/**
 * @brief Video encoder stream entry structure.
 */
struct venc_stream_entry
{
    VIDEO_STREAM_S stream;          /**< Video encoded data stream */
    struct dvr_date dvr_date_param; /**< Encoding timestamp */
    int exp_time;                   /**< Exposure time */
    struct ak_list list;          /**< Linked list node */
};

/**
 * @brief Audio encoder stream entry structure.
 */
struct aenc_stream_entry
{
    AUDIO_STREAM_S stream; /**< Audio encoded data stream */
    struct ak_list list; /**< Linked list node */
};

/**
 * @brief Frame entry structure.
 */
struct frame_entry
{
    FRAME_S frame_stream;  /**< Frame stream data */
    struct ak_list list; /**< Linked list node */
};

/**
 * @brief Lock-free ring buffer for video encoder streams.
 */
typedef struct {
    struct venc_stream_entry *entries[LOCKFREE_BUFFER_SIZE]; /**< Buffer entries */
    volatile atomic_int head;    /**< Consumer index */
    volatile atomic_int tail;    /**< Producer index */
    int size_mask;               /**< Size mask (size - 1) for fast modulo */
} lockfree_venc_buffer_t;

/**
 * @brief Lock-free ring buffer for audio encoder streams.
 */
typedef struct {
    struct aenc_stream_entry *entries[LOCKFREE_BUFFER_SIZE]; /**< Buffer entries */
    volatile atomic_int head;    /**< Consumer index */
    volatile atomic_int tail;    /**< Producer index */
    int size_mask;               /**< Size mask */
} lockfree_aenc_buffer_t;

/**
 * @brief Lock-free ring buffer for AI streams.
 */
typedef struct {
    struct frame_entry *entries[LOCKFREE_BUFFER_SIZE]; /**< Buffer entries */
    volatile atomic_int head;    /**< Consumer index */
    volatile atomic_int tail;    /**< Producer index */
    int size_mask;               /**< Size mask */
} lockfree_ai_buffer_t;

/**
 * @brief DVR configuration structure.
 *
 * Main configuration structure for the DVR application.
 */
struct dvr_conf
{
    int s32AiDevId;   /**< AI Device ID */
    int s32AiChnId;   /**< AI Channel ID */
    int s32AencId;    /**< AENC Channel ID */

    int s32ViDevId;   /**< VI Device ID */
    int s32ViChnId;   /**< VI Channel ID */

    int s32VpssGrpId; /**< VPSS Group ID */
    int s32VpssChnId; /**< VPSS Channel ID */

    int s32VencId;    /**< VENC Channel ID */

    int s32MuxHandle; /**< Muxer Handle */

    int i_width;      /**< Video Width */
    int i_height;     /**< Video Height */
    int i_sub_width;  /**< Sub-stream Width */
    int i_sub_height; /**< Sub-stream Height */
    int i_fps;        /**< Frame Rate */
    int i_frame_depth;/**< Frame Depth */
    int i_chn_idx;    /**< Channel Index */
    int i_venc_type;  /**< Video Codec Type (H264/H265/JPEG) */
    int i_br_mode;    /**< Bitrate Control Mode */
    int i_kbps;       /**< Bitrate in kbps */

    void *pv_aenc_handle;        /**< AENC Handle pointer */
    void *pv_aenc_stream_handle; /**< AENC Stream Handle pointer */

    MUX_CONF_S stMuxConf;        /**< Muxer Configuration */

    AUDIO_IN_PARAM_S stAiConf;   /**< AI Configuration */
    char *pc_ai_conf_file;       /**< AI Config File Path */
    char *pc_ai_conf_section;    /**< AI Config Section Name */

    VENC_CHN_ATTRS_S stVencConf; /**< VENC Configuration */

    int i_aenc_type;             /**< Audio Codec Type */
    char ac_isp_cfg[LEN_FILE];   /**< ISP Config File Path */

    int i_sec;                         /**< Recording duration */
    int i_run;                         /**< Running status flag */
    pthread_t a_pthread_t[THREAD_NUM]; /**< Thread IDs */
    char ac_path_save[LEN_FILE];       /**< Save path */
    char ac_path_tmp[LEN_FILE];        /**< Temp path */
    char ac_template_path[LEN_FILE];   /**< Path template */
    char ac_template_file[LEN_FILE];   /**< File name template */
    struct dbdesc *p_dbdesc_param;     /**< Database descriptor */

    struct dvr_date dvr_date_param;       /**< Recording start time */
    unsigned long long i_record_ms;       /**< Recording length in ms */
    char ac_file_video[LEN_FILE_UNION];   /**< Current video filename */
    char ac_file_idx[LEN_FILE_UNION];     /**< Current index filename */
    char final_file_video[LEN_FILE_UNION];/**< Final video filename */

    // Lock-free buffer (replaces original linked list and lock)
    lockfree_venc_buffer_t lockfree_venc_buffer; /**< Lock-free VENC buffer */
    lockfree_aenc_buffer_t lockfree_aenc_buffer; /**< Lock-free AENC buffer */
    lockfree_ai_buffer_t lockfree_ai_buffer;     /**< Lock-free AI buffer */

    // Atomic counter, used to monitor buffer usage
    volatile atomic_int current_venc_count; /**< Current VENC buffer count */
    volatile atomic_int current_aenc_count; /**< Current AENC buffer count */
    volatile atomic_int current_ai_count;   /**< Current AI buffer count */

    // Retain original linked lists and locks as backup (optional, for compatibility)
    pthread_mutex_t pthread_mutex_t_aenc; /**< AENC mutex */
    pthread_mutex_t pthread_mutex_t_venc; /**< VENC mutex */
    pthread_mutex_t pthread_mutex_t_ai;   /**< AI mutex */
    struct ak_list list_head_aenc;      /**< AENC list head */
    struct ak_list list_head_venc;      /**< VENC list head */
    struct ak_list list_head_ai;        /**< AI list head */

    int s32BuffRecord;   /**< Record buffer size */
    char *ps8BuffRecord; /**< Record buffer pointer */
    int s32BuffIndex;    /**< Index buffer size */
    char *ps8BuffIndex;  /**< Index buffer pointer */

    int i_buff_video; /**< Video synthesis buffer size */
    int i_buff_audio; /**< Audio synthesis buffer size */

    int i_list_video; /**< Current video list size */
    int i_list_audio; /**< Current audio list size */

    MUX_STATUS_E enMuxStatus;    /**< Muxer status */
    char c_mux_init;             /**< Muxer initialized flag */
    char c_mux_enable;           /**< Muxer enabled flag */
    char c_mux_manual_stop_flag; /**< Manual stop flag */




	CIRBUF_HANDLE vring_id; /**< Video Ring Buffer Handle */
	unsigned int vreader_id;/**< Video Reader ID */
};

#ifndef FREE_POINT
/** @brief Macro to safely free a pointer. */
#define FREE_POINT(POINT) \
    if (POINT != NULL)    \
    {                     \
        free(POINT);      \
        POINT = NULL;     \
    }
#endif

#define MUTEX_LOCK_AENC pthread_mutex_lock(&p_dvr_conf->pthread_mutex_t_aenc)
#define MUTEX_UNLOCK_AENC pthread_mutex_unlock(&p_dvr_conf->pthread_mutex_t_aenc);

#define MUTEX_LOCK_VENC pthread_mutex_lock(&p_dvr_conf->pthread_mutex_t_venc);
#define MUTEX_UNLOCK_VENC pthread_mutex_unlock(&p_dvr_conf->pthread_mutex_t_venc);

#define MUTEX_LOCK_AI pthread_mutex_lock(&p_dvr_conf->pthread_mutex_t_ai);
#define MUTEX_UNLOCK_AI pthread_mutex_unlock(&p_dvr_conf->pthread_mutex_t_ai);

#define HING_USAGE                                                                                             \
    "For example:\n"                                                                                           \
    "#MP3 H264 AVI\n"                                                                                          \
    "%s --path_save ./dvr --record_sec 60 --ai_sample_rate 8000 --vi_isp_cfg `ls /etc/config/isp*` "           \
    "--vi_width 1920 --vi_height 1080 --vi_fps 20 --venc_goplen 40 --aenc_type 2 --venc_type 0 --mux_type 0 "  \
    "--path_template '' --path_tmp ./tmp\n"                                                                    \
    "#AAC H265 AVI\n"                                                                                          \
    "%s --path_save ./dvr --record_sec 60 --ai_sample_rate 8000 --vi_isp_cfg `ls /etc/config/isp*` "           \
    "--vi_width 1920 --vi_height 1080 --vi_fps 20 --venc_goplen 40 --aenc_type 4 --venc_type 2 --mux_type 0 "  \
    "--path_template '' --path_tmp ./tmp\n"                                                                    \
    "#AMR H264 MP4\n"                                                                                          \
    "%s --path_save ./dvr --record_sec 60 --ai_sample_rate 8000 --vi_isp_cfg `ls /etc/config/isp*` "           \
    "--vi_width 1920 --vi_height 1080 --vi_fps 20 --venc_goplen 40 --aenc_type 3 --venc_type 0 --mux_type 1 "  \
    "--path_template '' --path_tmp ./tmp\n"                                                                    \
    "#g711a H265 MP4\n"                                                                                        \
    "%s --path_save ./dvr --record_sec 60 --ai_sample_rate 8000 --vi_isp_cfg `ls /etc/config/isp*` "           \
    "--vi_width 1920 --vi_height 1080 --vi_fps 20 --venc_goplen 40 --aenc_type 17 --venc_type 2 --mux_type 1 " \
    "--path_template '' --path_tmp ./tmp\n"

#define COLOR_BEGIN "\033[%d;%d;%dm"
#define COLOR_END "\033[0m"

/** @brief Console output color mode. */
enum COLOR_MODE
{
    CM_NORMAL = 0,     /**< Default mode */
    CM_BOLD = 1,       /**< Bold */
    CM_UNDERLINED = 4, /**< Underlined */
    CM_BLINK = 5,      /**< Blink */
    CM_NEGATIVE = 7,   /**< Inverse */
};

/** @brief Console output background color. */
enum COLOR_BACKGROUND
{
    CB_BLACK = 40,
    CB_RED = 41,
    CB_GREEN = 42,
    CB_YELLOW = 43,
    CB_BLUE = 44,
    CB_PURPLE = 45,
    CB_CYAN = 46,
    CB_WHITE = 47,
};

/** @brief Console output font color. */
enum COLOR_FONT
{
    CF_BLACK = 30,
    CF_RED = 31,
    CF_GREEN = 32,
    CF_YELLOW = 33,
    CF_BLUE = 34,
    CF_PURPLE = 35,
    CF_CYAN = 36,
    CF_WHITE = 37,
};

#ifndef AK_RTOS
/** @brief Debug print macro with color support (Linux). */
#define PRINTD(COLOR_MODE, COLOR_BACK, COLOR_FRONT, ...)                        \
    if (1)                                                                      \
    {                                                                           \
        printf("\033[%d;%d;%dm[%d:%ld]%s:%d:%s( ) ",                            \
               COLOR_MODE, COLOR_BACK, COLOR_FRONT,                             \
               getpid(), (long int)syscall(224), __FILE__, __LINE__, __func__); \
        printf(__VA_ARGS__);                                                    \
        printf("%s", COLOR_END);                                                \
        fflush(stdout);                                                         \
    }
#else
/** @brief Debug print macro with color support (RTOS). */
#define PRINTD(COLOR_MODE, COLOR_BACK, COLOR_FRONT, ...) \
    if (1)                                               \
    {                                                    \
        printf("\033[%d;%d;%dm%s:%d:%s( ) ",             \
               COLOR_MODE, COLOR_BACK, COLOR_FRONT,      \
               __FILE__, __LINE__, __func__);            \
        printf(__VA_ARGS__);                             \
        printf("%s", COLOR_END);                         \
        fflush(stdout);                                  \
    }
#endif

/**
 * @brief Send stream to record.
 *
 * @param p_dvr_conf Pointer to DVR configuration structure.
 * @param venc_handle_id Video encoder handle ID.
 * @param video_stream_get Pointer to the video stream data.
 * @return int Returns 0 on success, negative value on failure.
 */
int ak_dvr_send_stream_to_record(struct dvr_conf *p_dvr_conf, int venc_handle_id, VIDEO_STREAM_S *video_stream_get);

/**
 * @brief Fix DVR recorder.
 *
 * Attempts to repair the recording file if needed.
 *
 * @param[in] p_dvr_conf DVR configuration structure.
 * @return int Returns 0 on success, other values on failure.
 */
int ak_dvr_fix(struct dvr_conf *p_dvr_conf);

/**
 * @brief Initialize all parameters for DVR.
 *
 * @param[in] p_dvr_conf DVR configuration structure.
 * @return int Returns 0 on success, other values on failure.
 */
int ak_dvr_init(struct dvr_conf *p_dvr_conf);

/**
 * @brief Start DVR recorder.
 *
 * @param[in] p_dvr_conf DVR configuration structure.
 * @param video_thread_run_flag Flag to indicate if video thread is running.
 * @return int Returns 0 on success, other values on failure.
 */
int ak_dvr_start(struct dvr_conf *p_dvr_conf, int video_thread_run_flag);

/**
 * @brief Stop DVR recorder.
 *
 * @param[in] p_dvr_conf DVR configuration structure.
 * @return int Returns 0 on success, other values on failure.
 */
int ak_dvr_stop(struct dvr_conf *p_dvr_conf);

/**
 * @brief Manually stop DVR recorder.
 *
 * @param[in] p_dvr_conf DVR configuration structure.
 * @return int Returns 0 on success, other values on failure.
 */
int ak_dvr_manual_stop(struct dvr_conf *p_dvr_conf);

/**
 * @brief Exit DVR recorder and release all resources.
 *
 * @param[in] p_dvr_conf DVR configuration structure.
 * @return int Returns 0 on success, other values on failure.
 */
int ak_dvr_exit(struct dvr_conf *p_dvr_conf);

/**
 * @brief Get GC run flag.
 *
 * @param[out] gc_run_out Output for GC run status.
 * @param[out] saving_flag_out Output for saving status.
 * @return int Returns 0 on success.
 */
int get_gcrun_flag(char *gc_run_out, char *saving_flag_out);

#endif
