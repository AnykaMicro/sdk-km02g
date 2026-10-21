/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd. */
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <regex.h>
#include <getopt.h>
#include <errno.h>
#include <dirent.h>
#include <stdint.h>
#ifdef AK_RTOS
#include <kernel.h>
#endif

#include "ak_common_video.h"
#include "ak_common_audio.h"
#include "ak_ai.h"
#include "ak_aenc.h"
#include "ak_thread.h"
#include "ak_log.h"
#include "ak_mem.h"
#include "ak_vi.h"
#include "ak_vpss.h"
#include "ak_venc.h"
#include "ak_mux.h"
#include "ak_mux_sample.h"
#include "aenc_mp3.h"
#include "aenc_aac.h"
#include "aenc_amr.h"
#include "fun_log.h"
#include "media_configs.h"
#include "video_eis.h"
#include "ai_glass_common.h"
#include "ak_eis.h"
#ifdef AI_DENOISE_ENABLE
#include "ak_npu.h"
#endif

#include "ak_circular_buffer.h"


char g_eis_data_file_name[256] = {0};

static int dvr_init_ai(struct dvr_conf *p_dvr_conf);
static int dvr_init_vi_vpss(struct dvr_conf *p_dvr_conf);
static int dvr_init_aenc(struct dvr_conf *p_dvr_conf);
static int dvr_init_venc(struct dvr_conf *p_dvr_conf);
static int dvr_init_mux(struct dvr_conf *p_dvr_conf);
static void *thread_ai(void *pv_conf);
static void *thread_aenc(void *pv_conf);
static void *thread_vi_venc(void *pv_conf);
static void *thread_mux(void *pv_conf);

static volatile char saving_flag = AK_FALSE; // Save to file after recording is finished
static volatile char gc_run = AK_TRUE; // Running status flag
static char *gpc_prog_name = NULL;     // sample|demo name
static int gi_record_num = RECORD_NUM; // Total number of recorded videos
static int g_video_thread_run_flag = 0;

// ==================== Lock-free queue operation functions ====================

// Initialize lock-free buffer
static void init_lockfree_venc_buffer(lockfree_venc_buffer_t *buffer) {
    atomic_store(&buffer->head, 0);
    atomic_store(&buffer->tail, 0);
    buffer->size_mask = LOCKFREE_BUFFER_SIZE - 1;
    memset(buffer->entries, 0, sizeof(buffer->entries));
}

static void init_lockfree_aenc_buffer(lockfree_aenc_buffer_t *buffer) {
    atomic_store(&buffer->head, 0);
    atomic_store(&buffer->tail, 0);
    buffer->size_mask = LOCKFREE_BUFFER_SIZE - 1;
    memset(buffer->entries, 0, sizeof(buffer->entries));
}

static void init_lockfree_ai_buffer(lockfree_ai_buffer_t *buffer) {
    atomic_store(&buffer->head, 0);
    atomic_store(&buffer->tail, 0);
    buffer->size_mask = LOCKFREE_BUFFER_SIZE - 1;
    memset(buffer->entries, 0, sizeof(buffer->entries));
}

// VENC lock-free queue operations
static int lockfree_venc_push(lockfree_venc_buffer_t *buffer, struct venc_stream_entry *entry) {
    int current_tail = atomic_load(&buffer->tail);
    int next_tail = (current_tail + 1) & buffer->size_mask;

    // Check if the queue is full
    if (next_tail == atomic_load(&buffer->head)) {
        return -1;  // Queue full
    }

    buffer->entries[current_tail] = entry;
    atomic_thread_fence(memory_order_release);  // Ensure data writing is complete
    atomic_store(&buffer->tail, next_tail);

    return 0;
}

static struct venc_stream_entry* lockfree_venc_pop(lockfree_venc_buffer_t *buffer) {
    int current_head = atomic_load(&buffer->head);

    // Check if the queue is empty
    if (current_head == atomic_load(&buffer->tail)) {
        return NULL;  // Queue empty
    }

    struct venc_stream_entry *entry = buffer->entries[current_head];
    atomic_thread_fence(memory_order_acquire);  // Ensure data reading is complete
    atomic_store(&buffer->head, (current_head + 1) & buffer->size_mask);

    return entry;
}

// Batch pop (improve efficiency)
static int lockfree_venc_pop_batch(lockfree_venc_buffer_t *buffer,
                                   struct venc_stream_entry *batch[], int max_count) {
    int count = 0;
    int current_head = atomic_load(&buffer->head);
    int current_tail = atomic_load(&buffer->tail);

    // Calculate available data quantity
    int available = (current_tail - current_head) & buffer->size_mask;
    if (available == 0) return 0;

    int to_pop = (available < max_count) ? available : max_count;

    for (int i = 0; i < to_pop; i++) {
        batch[count] = buffer->entries[current_head];
        current_head = (current_head + 1) & buffer->size_mask;
        count++;
    }

    atomic_thread_fence(memory_order_acquire);
    atomic_store(&buffer->head, current_head);

    return count;
}

// AENC lock-free queue operations
static int lockfree_aenc_push(lockfree_aenc_buffer_t *buffer, struct aenc_stream_entry *entry) {
    int current_tail = atomic_load(&buffer->tail);
    int next_tail = (current_tail + 1) & buffer->size_mask;

    if (next_tail == atomic_load(&buffer->head)) {
        return -1;
    }

    buffer->entries[current_tail] = entry;
    atomic_thread_fence(memory_order_release);
    atomic_store(&buffer->tail, next_tail);

    return 0;
}

static struct aenc_stream_entry* lockfree_aenc_pop(lockfree_aenc_buffer_t *buffer) {
    int current_head = atomic_load(&buffer->head);

    if (current_head == atomic_load(&buffer->tail)) {
        return NULL;
    }

    struct aenc_stream_entry *entry = buffer->entries[current_head];
    atomic_thread_fence(memory_order_acquire);
    atomic_store(&buffer->head, (current_head + 1) & buffer->size_mask);

    return entry;
}

static int lockfree_aenc_pop_batch(lockfree_aenc_buffer_t *buffer,
                                   struct aenc_stream_entry *batch[], int max_count) {
    int count = 0;
    int current_head = atomic_load(&buffer->head);
    int current_tail = atomic_load(&buffer->tail);

    int available = (current_tail - current_head) & buffer->size_mask;
    if (available == 0) return 0;

    int to_pop = (available < max_count) ? available : max_count;

    for (int i = 0; i < to_pop; i++) {
        batch[count] = buffer->entries[current_head];
        current_head = (current_head + 1) & buffer->size_mask;
        count++;
    }

    atomic_thread_fence(memory_order_acquire);
    atomic_store(&buffer->head, current_head);

    return count;
}

// AI lock-free queue operations
static int lockfree_ai_push(lockfree_ai_buffer_t *buffer, struct frame_entry *entry) {
    int current_tail = atomic_load(&buffer->tail);
    int next_tail = (current_tail + 1) & buffer->size_mask;

    if (next_tail == atomic_load(&buffer->head)) {
        return -1;
    }

    buffer->entries[current_tail] = entry;
    atomic_thread_fence(memory_order_release);
    atomic_store(&buffer->tail, next_tail);

    return 0;
}

static struct frame_entry* lockfree_ai_pop(lockfree_ai_buffer_t *buffer) {
    int current_head = atomic_load(&buffer->head);

    if (current_head == atomic_load(&buffer->tail)) {
        return NULL;
    }

    struct frame_entry *entry = buffer->entries[current_head];
    atomic_thread_fence(memory_order_acquire);
    atomic_store(&buffer->head, (current_head + 1) & buffer->size_mask);

    return entry;
}

// ==================== End of lock-free queue operation functions ====================

/**
 * calculate_adaptive_crop - Adaptive crop parameter calculation function
 * Function: Automatically calculate optimal crop parameters based on input and output resolutions
 * Supports arbitrary aspect ratio adaptation, ensuring maximum effective screen area
 */
static void calculate_adaptive_crop(
    unsigned int input_width,    // Input screen width (stViRectangle.u32Width)
    unsigned int input_height,   // Input screen height (stViRectangle.u32Height)
    unsigned int output_width,   // Output screen width (stVpssChnAttr.u32Width)
    unsigned int output_height,  // Output screen height (stVpssChnAttr.u32Height)
    unsigned int *crop_width,    // Output: Crop width
    unsigned int *crop_height,   // Output: Crop height
    unsigned int *crop_x,        // Output: Crop start X coordinate
    unsigned int *crop_y         // Output: Crop start Y coordinate
)
{
    // Calculate aspect ratio (use integer arithmetic to avoid floating point numbers, precision * 1000)
    unsigned int input_ratio = input_width * 1000 / input_height;   // Input aspect ratio * 1000
    unsigned int output_ratio = output_width * 1000 / output_height; // Output aspect ratio * 1000

    unsigned int max_crop_width = output_width * 2;   // Maximum crop width limit
    unsigned int max_crop_height = output_height * 2; // Maximum crop height limit

    printf("Adaptive Crop: Input(%ux%u, ratio=%u), Output(%ux%u, ratio=%u), Limits(%ux%u)\n",
           input_width, input_height, input_ratio,
           output_width, output_height, output_ratio,
           max_crop_width, max_crop_height);

    if (input_ratio > output_ratio) {
        // Input screen is "wider", adapt by height, crop left and right
        *crop_height = (input_height < max_crop_height) ? input_height : max_crop_height;
        *crop_width = (*crop_height * output_width) / output_height;

        // Ensure crop width does not exceed input width and maximum limit
        if (*crop_width > input_width) {
            *crop_width = input_width;
            *crop_height = (*crop_width * output_height) / output_width;
        }
        if (*crop_width > max_crop_width) {
            *crop_width = max_crop_width;
            *crop_height = (*crop_width * output_height) / output_width;
        }

        // Center crop
        *crop_x = (input_width - *crop_width) / 2;
        *crop_y = (input_height - *crop_height) / 2;
    } else {
        // Input screen is "taller", adapt by width, crop top and bottom
        *crop_width = (input_width < max_crop_width) ? input_width : max_crop_width;
        *crop_height = (*crop_width * output_height) / output_width;

        // Ensure crop height does not exceed input height and maximum limit
        if (*crop_height > input_height) {
            *crop_height = input_height;
            *crop_width = (*crop_height * output_width) / output_height;
        }
        if (*crop_height > max_crop_height) {
            *crop_height = max_crop_height;
            *crop_width = (*crop_height * output_width) / output_height;
        }

        // Center crop
        *crop_x = (input_width - *crop_width) / 2;
        *crop_y = (input_height - *crop_height) / 2;
    }

    printf("Adaptive Crop Result: Crop(%ux%u) at (%u,%u)\n",
           *crop_width, *crop_height, *crop_x, *crop_y);
}

#ifdef AI_DENOISE_ENABLE
/**
 * InitAIDenoise - when use AIDenoise, init npu
 * notes:
 */
static AK_S32 InitAIDenoise(AK_VOID)
{
    AK_MPI_AUDIO_RegisterAIDenoise();
    int s32Ret = AK_MPI_NPU_Init();
    if (s32Ret) {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "ERROR: AK_MPI_NPU_Init,  s32Ret = %d\n", s32Ret);
        return AK_FAILED;
    }
    return AK_SUCCESS;
}

/**
 * DestroyAIDenoise - close AIDenoise
 * notes:
 */
static AK_VOID DestroyAIDenoise(AK_VOID)
{
    AK_MPI_NPU_Deinit();
}
#endif

static int create_mux_file(struct dvr_conf *p_dvr_conf) // Create and start a mux file
{
    MUX_START_PARAM_S stMuxStartParam;
    int ret = AK_FAILED;

    switch (p_dvr_conf->stMuxConf.enMuxType)
    { // Concatenate temporary recording file name
    case MUX_TYPE_MP4:
        snprintf(p_dvr_conf->ac_file_video,
                 sizeof(p_dvr_conf->ac_file_video), "%s/%04d%02d%02d%02d%02d%02d%s", p_dvr_conf->ac_path_tmp,
                 p_dvr_conf->dvr_date_param.year, p_dvr_conf->dvr_date_param.mon, p_dvr_conf->dvr_date_param.day,
                 p_dvr_conf->dvr_date_param.hour, p_dvr_conf->dvr_date_param.min, p_dvr_conf->dvr_date_param.sec,
                 FILE_EXT_MP4);
        break;
    case MUX_TYPE_AVI:
        snprintf(p_dvr_conf->ac_file_video,
                 sizeof(p_dvr_conf->ac_file_video), "%s/%04d%02d%02d%02d%02d%02d%s", p_dvr_conf->ac_path_tmp,
                 p_dvr_conf->dvr_date_param.year, p_dvr_conf->dvr_date_param.mon, p_dvr_conf->dvr_date_param.day,
                 p_dvr_conf->dvr_date_param.hour, p_dvr_conf->dvr_date_param.min, p_dvr_conf->dvr_date_param.sec,
                 FILE_EXT_AVI);
        break;
    default:
        AK_BASE_PrintErrorEx(MODULE_ID_APP,
                             "p_dvr_conf->stMuxConf.enVencOutputType= %d\n",
                             p_dvr_conf->stMuxConf.enVencOutputType);
        return ret;
    }
    stMuxStartParam.ps8FilePath = (signed char *)p_dvr_conf->ac_file_video;   // Set file name
    stMuxStartParam.s32BuffRecord = p_dvr_conf->s32BuffRecord;                // Set video file buffer length
    stMuxStartParam.ps8BuffRecord = (signed char *)p_dvr_conf->ps8BuffRecord; // Set video file buffer pointer
    stMuxStartParam.s32BuffIndex = p_dvr_conf->s32BuffIndex;                  // Set index file buffer length
    stMuxStartParam.ps8BuffIndex = (signed char *)p_dvr_conf->ps8BuffIndex;   // Set index file buffer pointer
    stMuxStartParam.ps8TmpFilePath = "/tmp/";

    // Start mux
    if ((ret = AK_MPI_MUX_Start(p_dvr_conf->s32MuxHandle, &stMuxStartParam)) != AK_SUCCESS)
    {
        AK_BASE_PrintErrorEx(MODULE_ID_APP,
                             "p_dvr_conf->s32MuxHandle= %d p_dvr_conf->ac_file_video= '%s'\n",
                             p_dvr_conf->s32MuxHandle, p_dvr_conf->ac_file_video);
    }
    return ret;
}
/* end of func */

static int dvr_get_localdate(struct dvr_date *p_dvr_date) // Get current time
{
    if (p_dvr_date == NULL)
    {
        return AK_FAILED;
    }

    gettimeofday(&p_dvr_date->timeval_val, &p_dvr_date->timezone_val);
    localtime_r(&(p_dvr_date->timeval_val.tv_sec), &p_dvr_date->tm_val);
    p_dvr_date->year = p_dvr_date->tm_val.tm_year + 1900;       // Year
    p_dvr_date->mon = p_dvr_date->tm_val.tm_mon + 1;            // Month
    p_dvr_date->day = p_dvr_date->tm_val.tm_mday;               // Day
    p_dvr_date->hour = p_dvr_date->tm_val.tm_hour;              // Hour
    p_dvr_date->min = p_dvr_date->tm_val.tm_min;                // Minute
    p_dvr_date->sec = p_dvr_date->tm_val.tm_sec;                // Second
    p_dvr_date->timezone = p_dvr_date->timezone_val.tz_dsttime; // Timezone

    return AK_SUCCESS;
}
/* end of func */

/*
Replace string
Replace pc_key in pc_src with pc_rep and save to pc_res
*/
static int strrep(char *pc_src, char *pc_key, char *pc_rep, char *pc_tmp)
{
    int i_len = strlen(pc_key); // Get the length of the string to be replaced
    int i_times = 0;
    char *pc_pos = NULL, *pc_now = pc_src;

    pc_tmp[0] = 0x0;
    while ((pc_pos = strstr(pc_now, pc_key)) != NULL)
    {                                             // Locate the position of the string to be replaced
        strncat(pc_tmp, pc_now, pc_pos - pc_now); // Copy the string before the location to the end of pc_res, copy specified length
        strcat(pc_tmp, pc_rep);                   // Copy the replacement string to the end of pc_res
        pc_now = pc_pos + i_len;                  // Jump position
        i_times++;
    }
    if (i_times > 0)
    {
        strcat(pc_tmp, pc_now); // Copy the remaining string
        i_len = strlen(pc_tmp);
        memcpy(pc_src, pc_tmp, strlen(pc_tmp));
        pc_src[i_len] = 0x0; // Set string end to 0
    }
    return i_times;
}
/* end of func */

static void save_mux_file(struct dvr_conf *p_dvr_conf) // Save recording file
{
    char ac_file[LEN_FILE], ac_fullpath_file[LEN_FILE], ac_cmd[LEN_CMD];
    char ac_template_path[LEN_TEMPLATE] = {0}, ac_template_file[LEN_TEMPLATE] = {0}, ac_tmp[LEN_TEMPLATE] = {0};
    char ac_num[FILE_TEMPLATE_NUM][LEN_NUM];
    int i_len_full;

    // Convert ak_date structure date/time data to string
    snprintf(ac_num[FILE_TEMPLATE_YEAR], sizeof(ac_num[FILE_TEMPLATE_YEAR]),
             "%04d", p_dvr_conf->dvr_date_param.year); // Year
    snprintf(ac_num[FILE_TEMPLATE_MONTH], sizeof(ac_num[FILE_TEMPLATE_MONTH]),
             "%02d", p_dvr_conf->dvr_date_param.mon); // Month
    snprintf(ac_num[FILE_TEMPLATE_DAY], sizeof(ac_num[FILE_TEMPLATE_DAY]),
             "%02d", p_dvr_conf->dvr_date_param.day); // Day
    snprintf(ac_num[FILE_TEMPLATE_HOUR], sizeof(ac_num[FILE_TEMPLATE_HOUR]),
             "%02d", p_dvr_conf->dvr_date_param.hour); // Hour
    snprintf(ac_num[FILE_TEMPLATE_MIN], sizeof(ac_num[FILE_TEMPLATE_MIN]),
             "%02d", p_dvr_conf->dvr_date_param.min); // Minute
    snprintf(ac_num[FILE_TEMPLATE_SEC], sizeof(ac_num[FILE_TEMPLATE_SEC]),
             "%02d", p_dvr_conf->dvr_date_param.sec); // Second
    snprintf(ac_num[FILE_TEMPLATE_MS], sizeof(ac_num[FILE_TEMPLATE_MS]),
             "%08llu", p_dvr_conf->i_record_ms); // Millisecond

    // Generate path name according to template format
    memcpy(ac_template_path, p_dvr_conf->ac_template_path, strlen(p_dvr_conf->ac_template_path));
    strrep(ac_template_path, "%Y", ac_num[FILE_TEMPLATE_YEAR], ac_tmp);  // Replace %Y => Year
    strrep(ac_template_path, "%M", ac_num[FILE_TEMPLATE_MONTH], ac_tmp); // Replace %M => Month
    strrep(ac_template_path, "%D", ac_num[FILE_TEMPLATE_DAY], ac_tmp);   // Replace %D => Day
    strrep(ac_template_path, "%h", ac_num[FILE_TEMPLATE_HOUR], ac_tmp);  // Replace %h => Hour
    strrep(ac_template_path, "%m", ac_num[FILE_TEMPLATE_MIN], ac_tmp);   // Replace %m => Minute
    strrep(ac_template_path, "%s", ac_num[FILE_TEMPLATE_SEC], ac_tmp);   // Replace %s => Second
    strrep(ac_template_path, "%S", ac_num[FILE_TEMPLATE_MS], ac_tmp);    // Replace %S => Millisecond

    // Generate file name according to template format
    memcpy(ac_template_file, p_dvr_conf->ac_template_file, strlen(p_dvr_conf->ac_template_file));
    strrep(ac_template_file, "%Y", ac_num[FILE_TEMPLATE_YEAR], ac_tmp);  // Replace %Y => Year
    strrep(ac_template_file, "%M", ac_num[FILE_TEMPLATE_MONTH], ac_tmp); // Replace %M => Month
    strrep(ac_template_file, "%D", ac_num[FILE_TEMPLATE_DAY], ac_tmp);   // Replace %D => Day
    strrep(ac_template_file, "%h", ac_num[FILE_TEMPLATE_HOUR], ac_tmp);  // Replace %h => Hour
    strrep(ac_template_file, "%m", ac_num[FILE_TEMPLATE_MIN], ac_tmp);   // Replace %m => Minute
    strrep(ac_template_file, "%s", ac_num[FILE_TEMPLATE_SEC], ac_tmp);   // Replace %s => Second
    strrep(ac_template_file, "%S", ac_num[FILE_TEMPLATE_MS], ac_tmp);    // Replace %S => Millisecond

    // Generate final saved recording file name according to template
    switch (p_dvr_conf->stMuxConf.enMuxType)
    { // Concatenate file name + extension
    case MUX_TYPE_MP4:
        {
			if(gm_config.record_param.eis_enable == 1)
			{
				log_warn_app("eeeeeeeeeeeee");
				snprintf(ac_file, sizeof(ac_file), "%s%s", ac_template_file, FILE_EXT_MP4);
			}
			else
			{
				log_warn_app("eeeeeeeeeeeeeno");
				snprintf(ac_file, sizeof(ac_file), "%s_origin%s", ac_template_file, FILE_EXT_MP4);
			}
	}
        break;
    case MUX_TYPE_AVI:
		if(gm_config.record_param.eis_enable == 1)
	snprintf(ac_file, sizeof(ac_file), "%s%s", ac_template_file, FILE_EXT_AVI);
		else
			snprintf(ac_file, sizeof(ac_file), "%s_origin%s", ac_template_file, FILE_EXT_AVI);
        break;
    default:
        break;
    }


    if (strlen(ac_template_path) > 0)
    {
        // Concatenate according to the set recording path and template path
        i_len_full = snprintf(ac_fullpath_file, sizeof(ac_fullpath_file),
                              "%s/%s", p_dvr_conf->ac_path_save, ac_template_path);
    }
    else
    {
        i_len_full = snprintf(ac_fullpath_file, sizeof(ac_fullpath_file), "%s", p_dvr_conf->ac_path_save);
    }
    // Create directory
#ifdef AK_RTOS
    snprintf(ac_cmd, sizeof(ac_cmd), "mkdir %s", ac_fullpath_file);
#else
    snprintf(ac_cmd, sizeof(ac_cmd), "mkdir -p %s", ac_fullpath_file);
#endif
    system(ac_cmd);

	sprintf(g_eis_data_file_name, "%s/%s", ac_fullpath_file, ac_template_file);

    // Concatenate file name
    snprintf(ac_fullpath_file + i_len_full, sizeof(ac_fullpath_file) - i_len_full, "/%s", ac_file);

    AK_BASE_PrintNormal(MODULE_ID_APP, "%s => %s\n", p_dvr_conf->ac_file_video, ac_fullpath_file);

    rename(p_dvr_conf->ac_file_video, ac_fullpath_file); // Move file to corresponding location
    remove(p_dvr_conf->ac_file_idx);                     // Delete index temporary file
    strncpy(p_dvr_conf->final_file_video,ac_fullpath_file,strlen(ac_fullpath_file)); // Get final file name path
	printf("----video_file_name = %s ------\n",p_dvr_conf->final_file_video);
}
/* end of func */

// Initialize mux related interfaces
static int dvr_init_mux(struct dvr_conf *p_dvr_conf)
{
    if (AK_MPI_MUX_Open(&p_dvr_conf->stMuxConf, &p_dvr_conf->s32MuxHandle))
    {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "mux open error\n");
        return AK_FAILED;
    }
    else
    {
        AK_BASE_PrintNormal(MODULE_ID_APP, "mux open success\n");
    }

    // Initialize lock-free buffer (replace original linked list and lock)
    init_lockfree_venc_buffer(&p_dvr_conf->lockfree_venc_buffer);
    init_lockfree_aenc_buffer(&p_dvr_conf->lockfree_aenc_buffer);
    init_lockfree_ai_buffer(&p_dvr_conf->lockfree_ai_buffer);

    // Initialize atomic counters
    atomic_store(&p_dvr_conf->current_venc_count, 0);
    atomic_store(&p_dvr_conf->current_aenc_count, 0);
    atomic_store(&p_dvr_conf->current_ai_count, 0);

    // Preserve original list and lock initialization (for compatibility)
    ak_list_init(&p_dvr_conf->list_head_aenc);
    ak_list_init(&p_dvr_conf->list_head_venc);
    ak_list_init(&p_dvr_conf->list_head_ai);

    pthread_mutex_init(&p_dvr_conf->pthread_mutex_t_aenc, NULL);
    pthread_mutex_init(&p_dvr_conf->pthread_mutex_t_venc, NULL);
    pthread_mutex_init(&p_dvr_conf->pthread_mutex_t_ai, NULL);

    if (p_dvr_conf->s32BuffRecord > 0)
    { // Initialize recording buffer
        if ((p_dvr_conf->ps8BuffRecord = calloc(1, p_dvr_conf->s32BuffRecord)) == NULL)
        {
            p_dvr_conf->s32BuffRecord = 0;
        }
    }

    if (p_dvr_conf->s32BuffIndex > 0)
    { // Initialize index buffer
        if ((p_dvr_conf->ps8BuffIndex = calloc(1, p_dvr_conf->s32BuffIndex)) == NULL)
        {
            p_dvr_conf->s32BuffIndex = 0;
        }
    }

    AK_BASE_PrintNormal(MODULE_ID_APP, "Lockfree buffers initialized successfully\n");
    return AK_SUCCESS;
}
/* end of func */

/*
    timeval_mark: Assign current timestamp to a timeval structure
    @p_timeval[IN]: Pointer to the struct timeval structure
    return: Address of the passed structure pointer
*/
static struct timeval *timeval_mark(struct timeval *p_timeval)
{
    gettimeofday(p_timeval, 0);
    return p_timeval;
}
/* end of func */

/*
    timeval_count: Calculate the time difference between two timevals
    @p_timeval_begin[IN]: Pointer to the struct timeval structure, recording start value
    @p_timeval_end[IN]: Pointer to the struct timeval, recording end value
    return: Difference in usec between two struct timevals
*/
unsigned long long timeval_count(struct timeval *p_timeval_begin, struct timeval *p_timeval_end)
{
    if ((p_timeval_begin->tv_sec > p_timeval_end->tv_sec) ||
        ((p_timeval_begin->tv_sec == p_timeval_end->tv_sec) &&
         (p_timeval_begin->tv_usec >= p_timeval_end->tv_usec)))
    {
        return 0;
    }
    else
    {
        return (unsigned long long)(p_timeval_end->tv_sec - p_timeval_begin->tv_sec) * S2US +
               (unsigned long long)p_timeval_end->tv_usec - (unsigned long long)p_timeval_begin->tv_usec;
    }
}
/* end of func */

// AI acquisition thread (optimized using lock-free queue)
static void *thread_ai(void *pv_conf)
{
    struct dvr_conf *p_dvr_conf = (struct dvr_conf *)pv_conf;
    int ret;
    struct frame_entry *p_frame_entry;

    AK_BASE_ThreadSetName("thread_ai");
    while (p_dvr_conf->i_run == AK_TRUE)
    {
        // Create a video frame structure
        if ((p_frame_entry = (struct frame_entry *)calloc(1, sizeof(struct frame_entry))) == NULL)
        {
            AK_BASE_PrintErrorEx(MODULE_ID_APP, "calloc failure\n");
            continue;
        }
        // Get one frame of audio AI data
        if ((ret = AK_MPI_AI_GetFrame(p_dvr_conf->s32AiDevId, p_dvr_conf->s32AiChnId, &p_frame_entry->frame_stream,
                                      1000, 0)) == AK_SUCCESS)
        {
            static int ai = 0;
            if(ai++ == 0)
                printf("%s_%d ait=%" PRIu64 "",__FUNCTION__, __LINE__, p_frame_entry->frame_stream.u64Ts);
            // Try to push to lock-free queue
            if (lockfree_ai_push(&p_dvr_conf->lockfree_ai_buffer, p_frame_entry) != 0) {
                // Queue full, release data
                AK_MPI_AI_ReleaseFrame(p_dvr_conf->s32AiDevId, p_dvr_conf->s32AiChnId, &p_frame_entry->frame_stream, 0);
                FREE_POINT(p_frame_entry);
                printf("*** AI LOCKFREE BUFFER FULL ***\n");
            } else {
                // Successfully pushed, update count
                atomic_fetch_add(&p_dvr_conf->current_ai_count, 1);
            }
        }
        else
        {
            // Release structure if acquisition failed
            FREE_POINT(p_frame_entry);
            switch (ret)
            {
            case ERROR_AI_NO_DATA:
                AK_BASE_SleepMs(10);
                continue;
            default:
                AK_BASE_PrintErrorEx(MODULE_ID_APP, "AK_MPI_AI_GetFrame error ret=%d\n", ret);
                break;
            }
        }
    }

    AK_BASE_PrintNormal(MODULE_ID_APP, "%s exit\n", __func__);
    AK_BASE_ThreadExit();
    return NULL;
}
/* end of func */

// AENC acquisition thread (optimized using lock-free queue)
static void *thread_aenc(void *pv_conf)
{
    struct dvr_conf *p_dvr_conf = (struct dvr_conf *)pv_conf;
    struct frame_entry *p_frame_now;
    struct aenc_stream_entry *p_aenc_stream_entry = NULL;
    int ret;

    AK_BASE_ThreadSetName("thread_aenc");
    while (p_dvr_conf->i_run == AK_TRUE)
    {
        // Get data from lock-free AI queue
        p_frame_now = lockfree_ai_pop(&p_dvr_conf->lockfree_ai_buffer);
        if (p_frame_now != NULL) {
            atomic_fetch_sub(&p_dvr_conf->current_ai_count, 1);

            // Encode one frame of AI data into AENC data
            if (AK_MPI_AENC_SendFrame(p_dvr_conf->s32AencId, &p_frame_now->frame_stream, 0) != AK_SUCCESS)
            {
                AK_BASE_PrintErrorEx(MODULE_ID_APP, "send frame error\n");
            }
            // Release AI data
            AK_MPI_AI_ReleaseFrame(p_dvr_conf->s32AiDevId, p_dvr_conf->s32AiChnId, &p_frame_now->frame_stream, 0);
            FREE_POINT(p_frame_now);
        }

        // Allocate an AENC structure
        if ((p_aenc_stream_entry =
                 (struct aenc_stream_entry *)calloc(1, sizeof(struct aenc_stream_entry))) == NULL)
        {
            AK_BASE_PrintErrorEx(MODULE_ID_APP, "calloc failure\n");
            continue;
        }
        // Get one frame of AENC data
        if ((ret = AK_MPI_AENC_GetStream(p_dvr_conf->s32AencId, &p_aenc_stream_entry->stream, 0)) == AK_SUCCESS)
        {
            if(saving_flag)
            {
                // printf("Saving File... free new audio frame\n");
                AK_MPI_AENC_ReleaseStream(p_dvr_conf->s32AencId, &p_aenc_stream_entry->stream);
                FREE_POINT(p_aenc_stream_entry);
            }
            else
            {
                // Try to push to lock-free queue
                if (lockfree_aenc_push(&p_dvr_conf->lockfree_aenc_buffer, p_aenc_stream_entry) != 0) {
                    // Queue full, release data
                    AK_MPI_AENC_ReleaseStream(p_dvr_conf->s32AencId, &p_aenc_stream_entry->stream);
                    FREE_POINT(p_aenc_stream_entry);
                    printf("*** AENC LOCKFREE BUFFER FULL ***\n");
                } else {
                    // Successfully pushed, update count
                    atomic_fetch_add(&p_dvr_conf->current_aenc_count, 1);
                }
            }
        }
        else
        { // Release current AENC structure if data acquisition failed
            switch (ret)
            {
            case ERROR_AENC_NO_DATA:
                AK_BASE_SleepMs(10);
                break;
            default:
                AK_BASE_PrintErrorEx(MODULE_ID_APP, "AK_MPI_AENC_GetStream error ret=%d\n", ret);
                break;
            }
            FREE_POINT(p_aenc_stream_entry);
        }
    }

    AK_BASE_PrintNormal(MODULE_ID_APP, "%s exit\n", __func__);
    AK_BASE_ThreadExit();
    return NULL;
}
/* end of func */

#if 1
int ak_dvr_send_stream_to_record(struct dvr_conf *p_dvr_conf, int venc_handle_id, VIDEO_STREAM_S *video_stream_get)
{
    if(p_dvr_conf->i_run != AK_TRUE)
    {
        AK_BASE_PrintNoticeEx(MODULE_ID_MUX,"p_dvr_conf->i_run OFF, need ak_dvr_start to open!\n");
        return -1;
    }

    unsigned long long i_ts_prev = 0, i_us_split; // Previous frame timestamp, used for debugging
    //struct venc_stream_entry *p_venc_stream_entry = NULL;
	struct venc_stream_entry venc_stream_entry = {0};
	int frame_head_len = sizeof(struct venc_stream_entry);
    struct timeval timeval_begin, timeval_end;
    int i_times_venc = 0;
    char c_encode_frame;

    /*p_venc_stream_entry =(struct venc_stream_entry *)calloc(1, sizeof(struct venc_stream_entry));
    if(NULL == p_venc_stream_entry)
    {
        AK_BASE_PrintErrorEx(MODULE_ID_MUX,"p_venc_stream_entry calloc failed!\n");
        return -1;
    }

    p_venc_stream_entry->stream.pu8Data = (unsigned char *)malloc(video_stream_get->u32Len);
    if(NULL == p_venc_stream_entry->stream.pu8Data)
    {
        AK_BASE_PrintErrorEx(MODULE_ID_MUX,"p_venc_stream_entry->stream.pu8Data malloc failed!\n");
        FREE_POINT(p_venc_stream_entry);
        return -1;
    }*/

    venc_stream_entry.stream.u64Ts = video_stream_get->u64Ts;                    // Assign timestamp
    venc_stream_entry.stream.u64SeqNo = video_stream_get->u64SeqNo;             // Assign frame sequence number
    venc_stream_entry.stream.u32Len = video_stream_get->u32Len;                  // Get encoded frame length
    venc_stream_entry.stream.enFrameType = video_stream_get->enFrameType;        // Frame type

    // Frame data copy
    //memcpy(p_venc_stream_entry->stream.pu8Data, video_stream_get->pu8Data, video_stream_get->u32Len);




    // If it is a video stream, judge whether the current frame is an I frame. If it is JPEG encoding, judge whether the statistical frame count is reached.
    if ((venc_stream_entry.stream.enFrameType == FRAME_TYPE_I) ||
        ((p_dvr_conf->stVencConf.stVencAttrs.enOutputType == MJPEG_ENC_TYPE) &&
            ((venc_stream_entry.stream.u64SeqNo - 1) %
                p_dvr_conf->stVencConf.stVencAttrs.stH264Attr.u16GopLen ==
            0)))
    {
        // Get the current time to judge whether to truncate in the mux thread
        dvr_get_localdate(&venc_stream_entry.dvr_date_param);
        if (i_ts_prev != 0)
        { // Test code, judge the collection cycle of the sensor
            i_us_split = timeval_count(&timeval_begin, timeval_mark(&timeval_end));
            timeval_begin = timeval_end;
#if 0
            // Print the current mux corresponding information (use lock-free queue count)
            int current_venc_count = atomic_load(&p_dvr_conf->current_venc_count);
            int current_aenc_count = atomic_load(&p_dvr_conf->current_aenc_count);
            AK_BASE_PrintNormalEx( MODULE_ID_APP,
                                    "i_times_venc= %d i_ts_prev= %llu video_stream_get->u64Ts= %lu "
                                    "diff= %llu i_us_split= %llu lockfree_V= %d/%d lockfree_A= %d/%d fps= %.4f\n",
                                    i_times_venc, i_ts_prev, video_stream_get->u64Ts,
                                    video_stream_get->u64Ts - i_ts_prev, i_us_split,
                                    current_venc_count, LOCKFREE_BUFFER_SIZE,
                                    current_aenc_count, LOCKFREE_BUFFER_SIZE,
                                    ( double )i_times_venc / i_us_split * 1000000 );
#endif
            i_times_venc = 0;
        }
        else
        {
            timeval_mark(&timeval_begin);
        }
        i_ts_prev = video_stream_get->u64Ts;
    }

	char *buf_tmp = cirbuf_get_write_buf(p_dvr_conf->vring_id,video_stream_get->u32Len + frame_head_len);
	if(buf_tmp == NULL)
	{
		log_error_app("cirbuf_get_write_buf is null");
	}
	memcpy(buf_tmp, &venc_stream_entry, frame_head_len);
	memcpy(buf_tmp + sizeof(struct venc_stream_entry), video_stream_get->pu8Data, video_stream_get->u32Len);
	cirbuf_put_write_buf(p_dvr_conf->vring_id, video_stream_get->u32Len + frame_head_len);
    // Use lock-free queue to replace original list and lock
//    if (lockfree_venc_push(&p_dvr_conf->lockfree_venc_buffer, p_venc_stream_entry) != 0) {
//        // Queue full, release data
//        FREE_POINT(p_venc_stream_entry->stream.pu8Data);
//        FREE_POINT(p_venc_stream_entry);
//        printf("*** VENC LOCKFREE BUFFER FULL in ak_dvr_send_stream_to_record ***\n");
//        return -1;
//    } else {
        // Successfully pushed, update count
        atomic_fetch_add(&p_dvr_conf->current_venc_count, 1);

        // Monitor queue usage
        int current_count = atomic_load(&p_dvr_conf->current_venc_count);
        if (current_count > LOCKFREE_BUFFER_SIZE * 0.5) {
            printf("*** VENC BUFFER WARNING in ak_dvr_send_stream_to_record: %d/%d ***\n",
                   current_count, LOCKFREE_BUFFER_SIZE);
        }
    //}

    return 0;
}
#endif

static void *thread_vi_venc(void *pv_conf) // Video VENC acquisition thread
{   printf("%s_%d here\n",__FUNCTION__, __LINE__);
    VPSS_FRAME_INFO_S vpss_frame_get = {0}, vpss_frame_mark = {0};

    VIDEO_STREAM_S video_stream_get;
    struct venc_stream_entry venc_stream_entry = {0};
	int frame_head_len = sizeof(struct venc_stream_entry);
    struct dvr_conf *p_dvr_conf = (struct dvr_conf *)pv_conf;
    unsigned long long i_ts_prev = 0, i_us_split; // Previous frame timestamp, used for debugging
    struct timeval timeval_begin, timeval_end;
    int i_times_venc = 0;
    char c_encode_frame;

	int frame_time_ok = 0;
	int64_t prev_frame_time = 0;

    AK_BASE_ThreadSetName("thread_vi_venc");
    while (p_dvr_conf->i_run == AK_TRUE)
    {
        c_encode_frame = AK_FAILED;
        memset(&vpss_frame_get, 0x00, sizeof(vpss_frame_get));
        memset(&venc_stream_entry, 0x00, sizeof(struct venc_stream_entry));
        // Get a VI frame and check if encoding is successful
        int iRet = AK_MPI_VPSS_GetFrame(p_dvr_conf->s32VpssGrpId, p_dvr_conf->s32VpssChnId,&vpss_frame_get, 2000);
        if (iRet != AK_SUCCESS)
        {
			usleep(10*1000);
			continue;
        }

//		if( prev_frame_time == 0 || ( frame_time_ok == 0 && (vpss_frame_get.stFrame.u64Ts - prev_frame_time) > 40) ) // Drop frames with unstable frame time (>40ms)
//		{
//			prev_frame_time = vpss_frame_get.stFrame.u64Ts;
//			printf("%s_%d vit=%llu drop\n",__FUNCTION__, __LINE__, vpss_frame_get.stFrame.u64Ts);
//			AK_MPI_VPSS_ReleaseFrame(p_dvr_conf->s32ViDevId, p_dvr_conf->s32VpssChnId, &vpss_frame_get); // Release current VI frame
//			continue;
//		}
		frame_time_ok = 1;
        c_encode_frame = AK_MPI_VENC_EncodeFrame(p_dvr_conf->s32VencId, vpss_frame_get.stFrame.pu8Data,
						 vpss_frame_get.stFrame.u32Len, vpss_frame_get.pvMdInfo, &video_stream_get);
        if(c_encode_frame != AK_SUCCESS)
        {
			AK_MPI_VPSS_ReleaseFrame(p_dvr_conf->s32ViDevId, p_dvr_conf->s32VpssChnId, &vpss_frame_get); // Release current VI frame
			continue;
        }

//		p_venc_stream_entry = (struct venc_stream_entry *)calloc(1, sizeof(struct venc_stream_entry));
//        p_venc_stream_entry->stream.pu8Data = (unsigned char *)malloc(video_stream_get.u32Len);

            static int last_frame_ms = 0;
            static int vi = 0;
           if(vi++== 0)
            {
//				FILE *fdt = fopen("/tmp/t.yuv","w+");
//					fwrite(vpss_frame_get.stFrame.pu8Data,
//                                                       vpss_frame_get.stFrame.u32Len,1,fdt);
//					fclose(fdt);
                last_frame_ms = vpss_frame_get.stFrame.u64Ts;

				printf("%s_%d vit=%" PRIu64 " \n",__FUNCTION__, __LINE__, vpss_frame_get.stFrame.u64Ts);
            }

            vpss_frame_mark = vpss_frame_get;                                                            // Record ts and seq_no
            AK_MPI_VPSS_ReleaseFrame(p_dvr_conf->s32ViDevId, p_dvr_conf->s32VpssChnId, &vpss_frame_get); // Release current VI frame
            i_times_venc++;                                                                          // Encoding count +1
            venc_stream_entry.stream.u64Ts = vpss_frame_mark.stFrame.u64Ts;                           // Assign timestamp
            venc_stream_entry.stream.u64SeqNo = vpss_frame_mark.stFrame.u64SeqNo;                     // Assign frame sequence number
            venc_stream_entry.stream.u32Len = video_stream_get.u32Len;                                // Get encoded frame length
            venc_stream_entry.stream.enFrameType = video_stream_get.enFrameType;                      // Frame type
            // Frame data copy
            //memcpy(p_venc_stream_entry->stream.pu8Data, video_stream_get.pu8Data, video_stream_get.u32Len);



            // Embed ISO speed and exposure time information
            ISP_AE_RUN_INFO_S stAeRunInfo = {0};
		ISP_AE_ATTR_S stAeAttr = {0};
            AK_MPI_ISP_GetAeRunInfo(0, &stAeRunInfo);
            AK_MPI_ISP_GetAeAttr(0, &stAeAttr);

            //AK_BASE_PrintNormalEx(MODULE_ID_APP, "gain : %lu, %lu, %lu!\n", stAeRunInfo.u32CurrentAGain, stAeRunInfo.u32CurrentDGain, stAeRunInfo.u32CurrentIspDGain);
            AK_U32 u32Iso = 100 * stAeRunInfo.u32CurrentAGain / 256 * stAeRunInfo.u32CurrentDGain / 256 * stAeRunInfo.u32CurrentIspDGain / 256;
            //AK_BASE_PrintNormalEx(MODULE_ID_APP, "iso : %lu!\n", u32Iso);
            AK_S32 s32Fps = 10;
            AK_MPI_ISP_GetSensorFps(0, &s32Fps);
			static int ii = 0;
			ii++;
            if (0 == s32Fps)
            {
				if(ii==1)
				{
					printf("iiiiiiiiii p_dvr_conf->s32VpssGrpId=%d \n", p_dvr_conf->s32VpssGrpId);
				}
				s32Fps = 30;
            }

            if(venc_stream_entry.stream.u64Ts - last_frame_ms > ((1000 / s32Fps) + 10) )
            {
                // The timestamp from VI is incorrect
                //AK_BASE_PrintError(MODULE_ID_APP, "vi ts error. venc_stream_entry.stream.u64Ts=%d last_frame_ms=%d\n", venc_stream_entry.stream.u64Ts ,last_frame_ms);
            }
            last_frame_ms = venc_stream_entry.stream.u64Ts;

            //AK_BASE_PrintNormalEx(MODULE_ID_APP, "exptime : %lu, %lu, fps: %d!\n", stAeRunInfo.u32CurrentExpTime, stAeAttr.u32ExpTimeMax, s32Fps);
            AK_U32 u32ExptimeMs = 1000 * stAeRunInfo.u32CurrentExpTime / stAeAttr.u32ExpTimeMax / s32Fps;
            //AK_BASE_PrintNormalEx(MODULE_ID_APP, "u32ExptimeMs : %lu!\n", u32ExptimeMs);
            if (0 == u32ExptimeMs)
                u32ExptimeMs = 1;
            AK_U32 u32Shutter = 1000 / u32ExptimeMs;
			venc_stream_entry.exp_time = u32ExptimeMs;

			char *buf_tmp = cirbuf_get_write_buf(p_dvr_conf->vring_id,video_stream_get.u32Len + frame_head_len);
			if(buf_tmp == NULL)
			{
				log_error_app("cirbuf_get_write_buf is null");
			}
			memcpy(buf_tmp + sizeof(struct venc_stream_entry), video_stream_get.pu8Data, video_stream_get.u32Len);

            AK_MPI_VENC_ReleaseStream(p_dvr_conf->s32VencId, &video_stream_get); // Release encoded data


//			static FILE *fd = NULL;
//		if(fd == NULL)
//			fd = fopen("/mnt/s.264", "wb+");
//		if(fd != NULL)
//			fwrite(video_stream_get.pu8Data, video_stream_get.u32Len, 1, fd);

            // If it is a video stream, judge whether the current frame is an I frame. If it is JPEG encoding, judge whether the statistical frame count is reached.
            if ((venc_stream_entry.stream.enFrameType == FRAME_TYPE_I) ||
                ((p_dvr_conf->stVencConf.stVencAttrs.enOutputType == MJPEG_ENC_TYPE) &&
                 ((venc_stream_entry.stream.u64SeqNo - 1) %
                      p_dvr_conf->stVencConf.stVencAttrs.stH264Attr.u16GopLen ==
                  0)))
            {
                // Get the current time to judge whether to truncate in the mux thread
                dvr_get_localdate(&venc_stream_entry.dvr_date_param);
                if (i_ts_prev != 0)
                { // Test code, judge the collection cycle of the sensor
                    i_us_split = timeval_count(&timeval_begin, timeval_mark(&timeval_end));
                    timeval_begin = timeval_end;
#if 0
                    // Print the current mux corresponding information
                    AK_BASE_PrintNormalEx( MODULE_ID_APP,
                                          "i_times_venc= %d i_ts_prev= %llu vpss_frame_mark.stFrame.u64Ts= %lu "
                                          "diff= %llu i_us_split= %llu buff_V= %.2f%% buff_A= %.2f%% fps= %.4f\n",
                                          i_times_venc, i_ts_prev, vpss_frame_mark.stFrame.u64Ts,
                                          vpss_frame_mark.stFrame.u64Ts - i_ts_prev, i_us_split,
                                         ( double )p_dvr_conf->i_list_video / p_dvr_conf->i_buff_video * 100,
                                         ( double )p_dvr_conf->i_list_audio / p_dvr_conf->i_buff_audio * 100,
                                         ( double )i_times_venc / i_us_split * 1000000 );
#endif
                    i_times_venc = 0;
                }
                else
                {
                    timeval_mark(&timeval_begin);
                }
                i_ts_prev = vpss_frame_mark.stFrame.u64Ts;
            }


			memcpy(buf_tmp, &venc_stream_entry, frame_head_len);
			cirbuf_put_write_buf(p_dvr_conf->vring_id, video_stream_get.u32Len + frame_head_len);

//            MUTEX_LOCK_VENC;

//            if(saving_flag)
//            {
//                // printf("Saving File... free new video frame\n");
//                FREE_POINT(p_venc_stream_entry->stream.pu8Data);
//                FREE_POINT(p_venc_stream_entry);
//            }
//            else
//            {
//                // Try to push to lock-free queue
//                if (lockfree_venc_push(&p_dvr_conf->lockfree_venc_buffer, p_venc_stream_entry) != 0) {
//                    // Queue full, release data
//                    FREE_POINT(p_venc_stream_entry->stream.pu8Data);
//                    FREE_POINT(p_venc_stream_entry);
//                    printf("*** VENC LOCKFREE BUFFER FULL ***\n");
//                } else {
                    // Successfully pushed, update counter
                    atomic_fetch_add(&p_dvr_conf->current_venc_count, 1);

                    // Monitor queue usage
                    int current_count = atomic_load(&p_dvr_conf->current_venc_count);
                    if (current_count > LOCKFREE_BUFFER_SIZE * 0.5) {
                        printf("*** VENC BUFFER WARNING: %d/%d ***\n",
                               current_count, LOCKFREE_BUFFER_SIZE);
                    }
//                }
//            }
//            MUTEX_UNLOCK_VENC;
    }

	printf("%s_%d exit \n",__FUNCTION__, __LINE__);

    AK_BASE_PrintNormal(MODULE_ID_APP, "%s exit\n", __func__);
    AK_BASE_ThreadExit();
    return NULL;
}
/* end of func */

// Get current media library status, success: AK_SUCCESS, fail: AK_FAILED
static int set_mux_status(struct dvr_conf *p_dvr_conf)
{
    MUX_STATUS_E enMuxStatus;

    // Get current status of media library
    if (AK_MPI_MUX_Get_Status(p_dvr_conf->s32MuxHandle, &enMuxStatus) == AK_SUCCESS)
    {
        p_dvr_conf->enMuxStatus = enMuxStatus;
        // If in the following three states, the media library is currently unavailable
        switch (p_dvr_conf->enMuxStatus)
        {
        case MUX_STATUS_SYSERR:
        case MUX_STATUS_MEMFULL:
        case MUX_STATUS_SYNERR:
            p_dvr_conf->c_mux_enable = AK_FALSE;
            break;
        default:
            p_dvr_conf->c_mux_enable = AK_TRUE;
        }
        return AK_SUCCESS;
    }
    else
    {
        p_dvr_conf->c_mux_enable = AK_FALSE;
        return AK_FAILED;
    }
}
/* end of func */

// Before the recording ends, synthesize all audio frames before the timestamp of the first I-frame of the next video
static int add_aenc_less_than_ts(struct dvr_conf *p_dvr_conf, unsigned long long i_ts)
{
    char c_list_exist, c_add = AK_FALSE;
    int i_aenc_times;
    struct aenc_stream_entry *p_aenc_stream_now = NULL;
    struct ak_list *p_node, *p_next;

    i_aenc_times = 0;
    for (;;)
    {
        c_list_exist = AK_FALSE;
        // Traverse current aenc list
        ak_list_for_each_safe(p_node, p_next, &p_dvr_conf->list_head_aenc)
        {
            p_aenc_stream_now = ak_list_item(p_node, struct aenc_stream_entry, list);
            // Check if timestamp is before I-frame
            if (p_aenc_stream_now->stream.u64Ts < i_ts)
            {
                MUTEX_LOCK_AENC;
                ak_list_remove(&p_aenc_stream_now->list); // Unlink from list
                if (p_dvr_conf->i_buff_audio > 0)
                {
                    p_dvr_conf->i_list_audio -= p_aenc_stream_now->stream.u32Len;
                }
                MUTEX_UNLOCK_AENC;
                // Add audio
                if ((p_dvr_conf->c_mux_init == AK_TRUE) && (p_dvr_conf->c_mux_enable == AK_TRUE) &&
                    (AK_MPI_MUX_Add_Audio(p_dvr_conf->s32MuxHandle, &p_aenc_stream_now->stream) != AK_SUCCESS))
                {
                    set_mux_status(p_dvr_conf);
                    AK_BASE_PrintErrorEx(MODULE_ID_APP,
                                         "****** ADD AUDIO FRAME ERROR ****** p_dvr_conf->s32MuxHandle= %d "
                                         "p_aenc_stream_now->stream => pu8Data= %p u32Len= %d u64Ts= %lu u64SeqNo= %lu\n",
                                         p_dvr_conf->s32MuxHandle, p_aenc_stream_now->stream.pu8Data,
                                         p_aenc_stream_now->stream.u32Len, p_aenc_stream_now->stream.u64Ts,
                                         p_aenc_stream_now->stream.u64SeqNo);
                }
                c_add = AK_TRUE;
                AK_MPI_AENC_ReleaseStream(p_dvr_conf->s32AencId, &p_aenc_stream_now->stream);
                FREE_POINT(p_aenc_stream_now);
                if (p_dvr_conf->c_mux_enable == AK_FALSE)
                {
                    return c_add;
                }
            }
            else
            { // If the timestamp of the acquired audio frame is greater than or equal to the timestamp of the first I-frame of the next video, it means all audio frames have been synthesized
                return c_add;
            }
            c_list_exist = AK_TRUE;
        }
        if (c_list_exist == AK_FALSE)
        { // Exit if polled TIMES_CUT_LOOP times and all are empty
            i_aenc_times++;
            if (i_aenc_times > TIMES_CUT_LOOP)
            {
                return c_add;
            }
            AK_BASE_SleepMs(10);
        }
    }
    return c_add;
}
/* end of func */

static void *thread_mux(void *pv_conf) // Video muxing thread (optimized using lock-free queue)
{
    unsigned long long i_ts_start = 0;
    struct dvr_conf *p_dvr_conf = (struct dvr_conf *)pv_conf;
    //struct venc_stream_entry *p_venc_stream_now = NULL;
    struct aenc_stream_entry *p_aenc_stream_now = NULL, *p_aenc_stream_next = NULL;
    int i_num_file = 0;
    char c_gop, c_data_exist;

    // Batch process buffer
    #define BATCH_SIZE 8
    //struct venc_stream_entry *venc_batch[BATCH_SIZE];
	struct venc_stream_entry stream_head;

    int tly_frame_cnt = 0;

    AK_BASE_ThreadSetName("thread_mux");
    while (p_dvr_conf->i_run == AK_TRUE)
    {
        c_data_exist = AK_FALSE; // Set data exist flag to false first

		int data_len = 0;
		unsigned char *data = NULL;
		do
		{
			int ret = cirbuf_get_buff_data(p_dvr_conf->vring_id, p_dvr_conf->vreader_id, &data,&data_len, 0);
			if( ret < 0)
			{
				break;
			}

		memcpy(&stream_head, data, sizeof(stream_head));
			stream_head.stream.pu8Data = data+sizeof(stream_head);
            // Update atomic counter
            atomic_fetch_sub(&p_dvr_conf->current_venc_count, 1);

            c_data_exist = AK_TRUE;

                // If it is a video stream, judge whether the current frame is an I frame. If it is JPEG encoding, judge whether the statistical frame count is reached.
                if ((stream_head.stream.enFrameType == FRAME_TYPE_I) ||
                    ((p_dvr_conf->stMuxConf.enVencOutputType == MJPEG_ENC_TYPE) &&
                     ((stream_head.stream.u64SeqNo - 1) %
                          p_dvr_conf->stVencConf.stVencAttrs.stH264Attr.u16GopLen ==
                      0)))
                {
                    c_gop = AK_TRUE;
                }
                else
                {
                    c_gop = AK_FALSE;
                }

                // Manually stop recording
                if ((p_dvr_conf->c_mux_manual_stop_flag == AK_TRUE))
                {
					 printf("sssssssss l=%d t=%lld \n", __LINE__, get_sys_run_time64());
                    p_dvr_conf->i_record_ms = stream_head.stream.u64Ts - i_ts_start; // Get recording duration (ms)
                    // Add audio data before this timestamp before stopping recording
                    add_aenc_less_than_ts(p_dvr_conf, stream_head.stream.u64Ts);
                    saving_flag = AK_TRUE;
                    AK_MPI_MUX_Stop(p_dvr_conf->s32MuxHandle); // Stop current recording
                    //sync();
                    system("(sync;echo 3 > /proc/sys/vm/drop_caches;)&");
                    save_mux_file(p_dvr_conf); // Save current recording file
                    i_num_file++;
                    p_dvr_conf->c_mux_init = AK_FALSE;
                    p_dvr_conf->c_mux_enable = AK_FALSE;
                    p_dvr_conf->c_mux_manual_stop_flag = AK_FALSE;
                    gc_run = AK_FALSE;
					printf("sssssssss l=%d t=%lld \n", __LINE__, get_sys_run_time64());
                }

                // Judge whether recording has started, whether recording duration needs to be truncated and start new recording, whether error occurred during recording
                if ((c_gop == AK_TRUE) &&
                    ((p_dvr_conf->c_mux_init == AK_FALSE) || (p_dvr_conf->c_mux_enable == AK_FALSE) ||
                     ((stream_head.stream.u64Ts - i_ts_start + MS_CUT_ADD) >= p_dvr_conf->i_sec * S2MS)))
                {
                    // Save recording if started or error occurred during recording
                    if ((p_dvr_conf->c_mux_init == AK_TRUE) && ((p_dvr_conf->c_mux_enable == AK_FALSE) || ((stream_head.stream.u64Ts - i_ts_start + MS_CUT_ADD) >= p_dvr_conf->i_sec * S2MS)))
                    {
                        p_dvr_conf->i_record_ms = stream_head.stream.u64Ts - i_ts_start; // Get recording duration (ms)
                        // Add audio data before this timestamp before stopping recording
                        printf("sssssssss l=%d t=%lld \n", __LINE__, get_sys_run_time64());
                        add_aenc_less_than_ts(p_dvr_conf, stream_head.stream.u64Ts);
						printf("sssssssss l=%d t=%lld \n", __LINE__, get_sys_run_time64());
                        saving_flag = AK_TRUE;
                        AK_MPI_MUX_Stop(p_dvr_conf->s32MuxHandle); // Stop current recording
                        printf("sssssssss l=%d t=%lld \n", __LINE__, get_sys_run_time64());
                        //sync();
                        system("(sync;echo 3 > /proc/sys/vm/drop_caches;)&");
                        save_mux_file(p_dvr_conf); // Save current recording file
                        printf("sssssssss l=%d t=%lld \n", __LINE__, get_sys_run_time64());
                        i_num_file++;
                        p_dvr_conf->c_mux_init = AK_FALSE;
                        p_dvr_conf->c_mux_enable = AK_FALSE;
                    }
                    // Exit if muxed file count reaches specified number
                    if ((gi_record_num > 0) && (i_num_file >= gi_record_num))
                    {
                        gc_run = AK_FALSE;
                    }
                    else
                    {                                                                   // Initialize next recording file
                        i_ts_start = stream_head.stream.u64Ts;                   // Get start recording timestamp
                        p_dvr_conf->dvr_date_param = stream_head.dvr_date_param; // Assign dvr_date_param
                        if (create_mux_file(p_dvr_conf) == AK_SUCCESS)
                        { // Create recording file
                            p_dvr_conf->c_mux_init = AK_TRUE;
                            set_mux_status(p_dvr_conf); // Re-get media library status
                        }
                        else
                        { // Failed to create recording file
                            p_dvr_conf->c_mux_init = AK_FALSE;
                            p_dvr_conf->c_mux_enable = AK_FALSE;
                        }
                    }
                }

                int bAddVideoOK = -1;
                // Judge whether current media library status can add venc, if yes then add
                // if ((p_dvr_conf->c_mux_init == AK_TRUE) && (p_dvr_conf->c_mux_enable == AK_TRUE) &&
                //     ( (bAddVideoOK=AK_MPI_MUX_Add_Video(p_dvr_conf->s32MuxHandle, &stream_head.stream)) != AK_SUCCESS))
                // {
                //     // If adding one frame of venc data fails, get media library status and print error
                //     set_mux_status(p_dvr_conf);
                //     AK_BASE_PrintErrorEx(MODULE_ID_APP,
                //                          "****** ADD VIDEO FRAME ERROR ****** p_dvr_conf->s32MuxHandle= %d "
                //                          "stream_head.stream => pu8Data= %p u32Len= %d u64Ts= %lu u64SeqNo= %lu enFrameType= %d\n",
                //                          p_dvr_conf->s32MuxHandle, stream_head.stream.pu8Data,
                //                          stream_head.stream.u32Len, stream_head.stream.u64Ts,
                //                          stream_head.stream.u64SeqNo, stream_head.stream.enFrameType);
                // }

                // Judge whether current media library status can add venc, if yes then add
                if ((p_dvr_conf->c_mux_init == AK_TRUE) && (p_dvr_conf->c_mux_enable == AK_TRUE) /*&&
                    ( (bAddVideoOK=AK_MPI_MUX_Add_Video(p_dvr_conf->s32MuxHandle, &stream_head.stream)) != AK_SUCCESS)*/)
                {
					static int vtotal = 0;
					long long llstart = get_sys_run_time64();
					bAddVideoOK = AK_MPI_MUX_Add_Video(p_dvr_conf->s32MuxHandle, &stream_head.stream);
//		static FILE *fd = NULL;
//		if(fd == NULL)
//			fd = fopen("/mnt/r.264", "wb+");
//		if(fd != NULL)
//			fwrite(stream_head.stream.pu8Data, stream_head.stream.u32Len, 1, fd);
					vtotal += stream_head.stream.u32Len;
					//AK_BASE_PrintErrorEx(MODULE_ID_APP,"addvend vtotal=%d\n", vtotal);
					if(get_sys_run_time64()-llstart > 50)
					{
						int current_count = atomic_load(&p_dvr_conf->current_venc_count);
						AK_BASE_PrintErrorEx(MODULE_ID_APP,"---write video too slow, total_len=%d cur_len=%d t=%lldms buf_framecnt=%d ---\n\n",
							vtotal, stream_head.stream.u32Len, get_sys_run_time64()-llstart, current_count);
					}

					if( bAddVideoOK != AK_SUCCESS)
                    // If adding one frame of venc data fails, get media library status and print error
                    {
						set_mux_status(p_dvr_conf);
	AK_BASE_PrintErrorEx(MODULE_ID_APP,
                                         "****** ADD VIDEO FRAME ERROR ****** p_dvr_conf->s32MuxHandle= %d "
                                         "stream_head.stream => pu8Data= %p u32Len= %d u64Ts= %lu u64SeqNo= %lu enFrameType= %d\n",
                                         p_dvr_conf->s32MuxHandle, stream_head.stream.pu8Data,
                                         stream_head.stream.u32Len, stream_head.stream.u64Ts,
                                         stream_head.stream.u64SeqNo, stream_head.stream.enFrameType);
                    }
                }

                if(bAddVideoOK == AK_SUCCESS)
                {
					if(gm_config.record_param.eis_enable == 0)
						ak_frame_info_store_write(stream_head.stream.u64Ts + (EXPOSE_OFFSET_TIME - stream_head.exp_time),stream_head.exp_time);
                    tly_frame_cnt++;
                    //printf("tly_frame_cnt=%d \n", tly_frame_cnt);
                }
            }while(1);

        // Get AENC data from lock-free queue
        p_aenc_stream_now = lockfree_aenc_pop(&p_dvr_conf->lockfree_aenc_buffer);
        if (p_aenc_stream_now != NULL) {
            atomic_fetch_sub(&p_dvr_conf->current_aenc_count, 1);
            c_data_exist = AK_TRUE;

            // Determine whether the current media library status can add AENC, if so, add it
            if ((p_dvr_conf->c_mux_init == AK_TRUE) && (p_dvr_conf->c_mux_enable == AK_TRUE) &&
                (AK_MPI_MUX_Add_Audio(p_dvr_conf->s32MuxHandle, &p_aenc_stream_now->stream) != AK_SUCCESS))
            {
                // If adding an AENC frame fails, get the media library status and print the error
                set_mux_status(p_dvr_conf);
                AK_BASE_PrintErrorEx(MODULE_ID_APP,
                                     "****** ADD AUDIO FRAME ERROR ****** p_dvr_conf->s32MuxHandle= %d "
                                     "p_aenc_stream_now->stream => pu8Data= %p u32Len= %d u64Ts= %lu u64SeqNo= %lu\n",
                                     p_dvr_conf->s32MuxHandle, p_aenc_stream_now->stream.pu8Data,
                                     p_aenc_stream_now->stream.u32Len, p_aenc_stream_now->stream.u64Ts,
                                     p_aenc_stream_now->stream.u64SeqNo);
            }
            AK_MPI_AENC_ReleaseStream(p_dvr_conf->s32AencId, &p_aenc_stream_now->stream);
            FREE_POINT(p_aenc_stream_now);
        }

        if (c_data_exist == AK_FALSE)
        {
            AK_BASE_SleepMs(10);
        }
    }
    if (p_dvr_conf->c_mux_init == AK_TRUE)
    {
        AK_MPI_MUX_Stop(p_dvr_conf->s32MuxHandle);
    }

    printf("%d:tly_frame_cnt=%d \n", __LINE__, tly_frame_cnt);

    sync();
    AK_BASE_PrintNormal(MODULE_ID_APP, "%s exit\n", __func__);
    AK_BASE_ThreadExit();
    return NULL;
}
/* end of func */

// Initialize AENC audio encoding interface
static int dvr_init_aenc(struct dvr_conf *p_dvr_conf)
{
    AENC_PARAM_S stAencConf = {0};
    AENC_ENCODE_S stEncode = {0};

    stAencConf.enType = p_dvr_conf->stMuxConf.enAudioType;
    stAencConf.enSampleBits = p_dvr_conf->stMuxConf.stAudioDataAttr.enSampleBits;
    stAencConf.enSampleRate = p_dvr_conf->stMuxConf.stAudioDataAttr.enSampleRate;
    stAencConf.enSoundMode = p_dvr_conf->stMuxConf.stAudioDataAttr.enSoundMode;

    stEncode.enType = p_dvr_conf->stMuxConf.enAudioType;

    switch (p_dvr_conf->stMuxConf.enAudioType)
    {
    case AK_AUDIO_TYPE_MP3:
        stEncode.pfnEncOpen = AENC_OpenMp3;
        if (AK_MPI_AENC_RegisterEncoder(&stEncode))
        {
            AK_BASE_PrintErrorEx(MODULE_ID_APP, "AK_MPI_AENC_RegisterEncoder failed\n");
            return AK_FAILED;
        }
        break;
    case AK_AUDIO_TYPE_AAC:
        stEncode.pfnEncOpen = AENC_OpenAac;
        stAencConf.unAencAttr.stAacParam.enAacFrmHead = AENC_AAC_SAVE_FRAME_HEAD;
        if (AK_MPI_AENC_RegisterEncoder(&stEncode))
        {
            AK_BASE_PrintErrorEx(MODULE_ID_APP, "AK_MPI_AENC_RegisterEncoder failed\n");
            return AK_FAILED;
        }
        /*
        stAencAttr.unAencAttr.stAacParam.enAacFrmHead = AENC_AAC_SAVE_FRAME_HEAD;
        if( AK_MPI_AENC_SetAttr( p_dvr_conf->s32VencId, &stAencAttr ) ){
            AK_BASE_PrintErrorEx( MODULE_ID_APP, "AK_MPI_AENC_SetAttr failed\n");
            return AK_FAILED;
        }
        */
        break;
    case AK_AUDIO_TYPE_AMR:
        stEncode.pfnEncOpen = AENC_OpenAmr;
        if (AK_MPI_AENC_RegisterEncoder(&stEncode))
        {
            AK_BASE_PrintErrorEx(MODULE_ID_APP, "AK_MPI_AENC_RegisterEncoder failed\n");
            return AK_FAILED;
        }
        break;
    case AK_AUDIO_TYPE_PCM_ALAW:
    case AK_AUDIO_TYPE_PCM_ULAW:
        break;
    default:
        break;
    }
    if (AK_MPI_AENC_Open(p_dvr_conf->s32AencId, &stAencConf) != AK_SUCCESS)
    {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "AK_MPI_AENC_Open failure\n");
        return AK_FAILED;
    }
    AK_BASE_PrintNormal(MODULE_ID_APP,
                        "stAencConf=> enSampleRate=%d enSoundMode=%d enSampleBits=%d type=%d \n",
                        stAencConf.enSampleRate, stAencConf.enSoundMode,
                        stAencConf.enSampleBits, stAencConf.enType);
    return AK_SUCCESS;
}
/* end of func */

// Save fixed file
static void save_fix_file(struct dvr_conf *p_dvr_conf, char *pc_file, int i_ms, int i_type)
{
    char ac_file[LEN_FILE], ac_fullpath_tmp[LEN_FILE_UNION], ac_fullpath_file[LEN_FILE], ac_cmd[LEN_CMD];
    char ac_template_path[LEN_TEMPLATE] = {0}, ac_template_file[LEN_TEMPLATE] = {0}, ac_tmp[LEN_TEMPLATE] = {0};
    char ac_num[FILE_TEMPLATE_NUM][LEN_NUM];
    int i_len_full;

    if (strlen(pc_file) < LEN_FIX_NAME)
    {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "strlen( pc_file )= %lu too short.\n", strlen(pc_file));
    }
    memcpy(ac_num[FILE_TEMPLATE_YEAR], pc_file + 0, 4); // Copy the passed file name to year, month, day, hour, minute, second respectively
    memcpy(ac_num[FILE_TEMPLATE_MONTH], pc_file + 4, 2);
    memcpy(ac_num[FILE_TEMPLATE_DAY], pc_file + 6, 2);
    memcpy(ac_num[FILE_TEMPLATE_HOUR], pc_file + 8, 2);
    memcpy(ac_num[FILE_TEMPLATE_MIN], pc_file + 10, 2);
    memcpy(ac_num[FILE_TEMPLATE_SEC], pc_file + 12, 2);
    snprintf(ac_num[FILE_TEMPLATE_MS], sizeof(ac_num[FILE_TEMPLATE_MS]), "%08d", i_ms); // Video duration obtained from the repaired video

    // Generate path name according to template format
    memcpy(ac_template_path, p_dvr_conf->ac_template_path, strlen(p_dvr_conf->ac_template_path));
    strrep(ac_template_path, "%Y", ac_num[FILE_TEMPLATE_YEAR], ac_tmp);  // Replace %Y => Year
    strrep(ac_template_path, "%M", ac_num[FILE_TEMPLATE_MONTH], ac_tmp); // Replace %M => Month
    strrep(ac_template_path, "%D", ac_num[FILE_TEMPLATE_DAY], ac_tmp);   // Replace %D => Day
    strrep(ac_template_path, "%h", ac_num[FILE_TEMPLATE_HOUR], ac_tmp);  // Replace %h => Hour
    strrep(ac_template_path, "%m", ac_num[FILE_TEMPLATE_MIN], ac_tmp);   // Replace %m => Minute
    strrep(ac_template_path, "%s", ac_num[FILE_TEMPLATE_SEC], ac_tmp);   // Replace %s => Second
    strrep(ac_template_path, "%S", ac_num[FILE_TEMPLATE_MS], ac_tmp);    // Replace %S => Millisecond

    // Generate file name according to template format
    memcpy(ac_template_file, p_dvr_conf->ac_template_file, strlen(p_dvr_conf->ac_template_file));
    strrep(ac_template_file, "%Y", ac_num[FILE_TEMPLATE_YEAR], ac_tmp);  // Replace %Y => Year
    strrep(ac_template_file, "%M", ac_num[FILE_TEMPLATE_MONTH], ac_tmp); // Replace %M => Month
    strrep(ac_template_file, "%D", ac_num[FILE_TEMPLATE_DAY], ac_tmp);   // Replace %D => Day
    strrep(ac_template_file, "%h", ac_num[FILE_TEMPLATE_HOUR], ac_tmp);  // Replace %h => Hour
    strrep(ac_template_file, "%m", ac_num[FILE_TEMPLATE_MIN], ac_tmp);   // Replace %m => Minute
    strrep(ac_template_file, "%s", ac_num[FILE_TEMPLATE_SEC], ac_tmp);   // Replace %s => Second
    strrep(ac_template_file, "%S", ac_num[FILE_TEMPLATE_MS], ac_tmp);    // Replace %S => Millisecond

    switch (i_type)
    { // Concatenate file name + extension
    case MUX_TYPE_MP4:
        snprintf(ac_file, sizeof(ac_file), "%s%s", ac_template_file, FILE_EXT_MP4);
        break;
    case MUX_TYPE_AVI:
        snprintf(ac_file, sizeof(ac_file), "%s%s", ac_template_file, FILE_EXT_AVI);
        break;
    default:
        break;
    }
    if (strlen(ac_template_path) > 0)
    { // Concatenate according to the set recording path and template path
        i_len_full = snprintf(ac_fullpath_file, sizeof(ac_fullpath_file),
                              "%s/%s", p_dvr_conf->ac_path_save, ac_template_path);
    }
    else
    {
        i_len_full = snprintf(ac_fullpath_file, sizeof(ac_fullpath_file), "%s", p_dvr_conf->ac_path_save);
    }
#ifdef AK_RTOS
    snprintf(ac_cmd, sizeof ac_cmd, "mkdir %s", ac_fullpath_file);
#else
    snprintf(ac_cmd, sizeof ac_cmd, "mkdir -p %s", ac_fullpath_file);
#endif
    system(ac_cmd); // Create directory

    // Concatenate written file name
    snprintf(ac_fullpath_file + i_len_full, sizeof(ac_fullpath_file) - i_len_full, "/%s", ac_file);
    // Concatenate temporary directory and file name
    snprintf(ac_fullpath_tmp, sizeof(ac_fullpath_tmp), "%s/%s", p_dvr_conf->ac_path_tmp, pc_file);
    PRINTD(CM_BOLD, CB_BLACK, CF_BLUE, "****** REPAIR FILE '%s' => '%s' ******\n",
           ac_fullpath_tmp, ac_fullpath_file);
    rename(ac_fullpath_tmp, ac_fullpath_file); // Move file to corresponding location
}
/* end of func */

// Check temporary directory to see if there are video files that need repair
int ak_dvr_fix(struct dvr_conf *p_dvr_conf)
{
    struct dirent **pp_dirent_list = NULL;
    int i_num_scandir, i, m;
    MUX_FILEINFO_S mux_fileinfo_param;

    char ac_file_name[MUX_NODE_NUM][LEN_FILE_UNION];
    char ac_mux_ext[MUX_NODE_NUM][LEN_EXT] = {
        "",
        ".idx",
        ".moov",
        ".stbl",
    };
    FILE *pFILE_mux[MUX_NODE_NUM];

    i_num_scandir = scandir(p_dvr_conf->ac_path_tmp, &pp_dirent_list, 0, alphasort);
    for (m = 0; m < MUX_NODE_NUM; m++)
    {
        pFILE_mux[m] = NULL;
    }
    // Traverse directory
    for (i = 0; i < i_num_scandir; i++)
    {
        if ((strcmp(pp_dirent_list[i]->d_name, ".") == 0) ||
            (strcmp(pp_dirent_list[i]->d_name, "..") == 0) ||
            (pp_dirent_list[i]->d_type == DT_DIR))
        { // Skip . .. directories
            FREE_POINT(pp_dirent_list[i]);
            continue;
        }
        // Whether there is AVI file to be repaired
        if (strcmp(pp_dirent_list[i]->d_name + strlen(pp_dirent_list[i]->d_name) -
                       strlen(FILE_EXT_AVI),
                   FILE_EXT_AVI) == 0)
        {
            PRINTD(CM_BOLD, CB_BLACK, CF_BLUE, "****** TRY REPAIR FILE: '%s' ******\n", pp_dirent_list[i]->d_name);
            // Concatenate string to create corresponding index file name
            snprintf(ac_file_name[MUX_NODE_RECORD], sizeof(ac_file_name[MUX_NODE_RECORD]), "%s/%s",
                     p_dvr_conf->ac_path_tmp, pp_dirent_list[i]->d_name);
            snprintf(ac_file_name[MUX_NODE_IDX], sizeof(ac_file_name[MUX_NODE_IDX]), "%s%s",
                     ac_file_name[MUX_NODE_RECORD], ac_mux_ext[MUX_NODE_IDX]);
            // Open corresponding index file and start repair
            if (((pFILE_mux[MUX_NODE_RECORD] = fopen(ac_file_name[MUX_NODE_RECORD], "rb+")) != NULL) &&
                ((pFILE_mux[MUX_NODE_IDX] = fopen(ac_file_name[MUX_NODE_IDX], "rb")) != NULL) &&
                (AK_MPI_MUX_Fix_File(pFILE_mux[MUX_NODE_RECORD], pFILE_mux[MUX_NODE_IDX], NULL, NULL,
                                     MUX_TYPE_AVI, &mux_fileinfo_param) == AK_SUCCESS) &&
                (mux_fileinfo_param.u64TotalMs > 0))
            {
                // Save recording file
                save_fix_file(p_dvr_conf, pp_dirent_list[i]->d_name, mux_fileinfo_param.u64TotalMs, MUX_TYPE_AVI);
            }
        }
        // Whether there is MP4 file to be repaired
        else if (strcmp(pp_dirent_list[i]->d_name +
                            strlen(pp_dirent_list[i]->d_name) - strlen(FILE_EXT_MP4),
                        FILE_EXT_MP4) == 0)
        {
            PRINTD(CM_BOLD, CB_BLACK, CF_BLUE, "****** TRY REPAIR FILE: '%s' ******\n", pp_dirent_list[i]->d_name);
            // Concatenate string to create corresponding index file name
            snprintf(ac_file_name[MUX_NODE_RECORD], sizeof(ac_file_name[MUX_NODE_RECORD]), "%s/%s",
                     p_dvr_conf->ac_path_tmp, pp_dirent_list[i]->d_name);
            snprintf(ac_file_name[MUX_NODE_IDX], sizeof(ac_file_name[MUX_NODE_IDX]), "%s%s",
                     ac_file_name[MUX_NODE_RECORD], ac_mux_ext[MUX_NODE_IDX]);
            snprintf(ac_file_name[MUX_NODE_MP4_MOOV], sizeof(ac_file_name[MUX_NODE_MP4_MOOV]), "%s%s",
                     ac_file_name[MUX_NODE_RECORD], ac_mux_ext[MUX_NODE_MP4_MOOV]);
            snprintf(ac_file_name[MUX_NODE_MP4_STBL], sizeof(ac_file_name[MUX_NODE_MP4_STBL]), "%s%s",
                     ac_file_name[MUX_NODE_RECORD], ac_mux_ext[MUX_NODE_MP4_STBL]);
            pFILE_mux[MUX_NODE_IDX] = fopen(ac_file_name[MUX_NODE_IDX], "rb");
            // Open corresponding index file and start repair
            if (((pFILE_mux[MUX_NODE_RECORD] = fopen(ac_file_name[MUX_NODE_RECORD], "rb+")) != NULL) &&
                ((pFILE_mux[MUX_NODE_MP4_MOOV] = fopen(ac_file_name[MUX_NODE_MP4_MOOV], "rb")) != NULL) &&
                ((pFILE_mux[MUX_NODE_MP4_STBL] = fopen(ac_file_name[MUX_NODE_MP4_STBL], "rb")) != NULL) &&
                (AK_MPI_MUX_Fix_File(pFILE_mux[MUX_NODE_RECORD], NULL, pFILE_mux[MUX_NODE_MP4_MOOV],
                                     pFILE_mux[MUX_NODE_MP4_STBL],
                                     MUX_TYPE_MP4, &mux_fileinfo_param) == AK_SUCCESS) &&
                (mux_fileinfo_param.u64TotalMs > 0))
            {
                // Save recording file
                save_fix_file(p_dvr_conf, pp_dirent_list[i]->d_name, mux_fileinfo_param.u64TotalMs, MUX_TYPE_MP4);
            }
        }
        // Close and release corresponding recording file resources
        for (m = 0; m < MUX_NODE_NUM; m++)
        {
            if (pFILE_mux[m] != NULL)
            {
                fflush(pFILE_mux[m]);
                fclose(pFILE_mux[m]);
                sync();
                pFILE_mux[m] = NULL;
                remove(ac_file_name[m]);
                PRINTD(CM_BOLD, CB_BLACK, CF_BLUE, "****** REMOVE FILE: %s ******\n", ac_file_name[m]);
            }
        }
        // Release traversed file name structure
        FREE_POINT(pp_dirent_list[i]);
    }
    FREE_POINT(pp_dirent_list);
    return AK_SUCCESS;
}
/* end of func */

// Initialization function for AI, AENC, VI, VENC, and MUX
int ak_dvr_init(struct dvr_conf *p_dvr_conf)
{
    if (dvr_init_ai(p_dvr_conf))
    {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "dvr_init_ai failure\n");
        return AK_FAILED;
    }
    if (dvr_init_aenc(p_dvr_conf))
    {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "dvr_init_aenc failure\n");
        return AK_FAILED;
    }
    if (dvr_init_vi_vpss(p_dvr_conf))
    {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "dvr_init_vi_vpss failure\n");
        return AK_FAILED;
    }
    if (dvr_init_venc(p_dvr_conf))
    {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "dvr_init_venc failure\n");
        return AK_FAILED;
    }
    if (dvr_init_mux(p_dvr_conf))
    {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "dvr_init_mux failure\n");
        return AK_FAILED;
    }

	p_dvr_conf->vring_id = cirbuf_new(8* 1024 *1024);
	p_dvr_conf->vreader_id = cirbuf_reader_new(p_dvr_conf->vring_id, "vreader");

    return AK_SUCCESS;
}
/* end of func */

// Start AI, AENC, VI/VENC, MUX threads to start recording
int ak_dvr_start(struct dvr_conf *p_dvr_conf, int video_thread_run_flag)
{
    printf("%s_%d here\n",__FUNCTION__, __LINE__);

    p_dvr_conf->i_run = AK_TRUE;

    // thread_mux
   // AK_BASE_ThreadCreate(&p_dvr_conf->a_pthread_t[THREAD_MUX], thread_mux,(void *)p_dvr_conf, ANYKA_THREAD_MIN_STACK_SIZE, 20);
	AK_BASE_ThreadCreate(&p_dvr_conf->a_pthread_t[THREAD_MUX], thread_mux,(void *)p_dvr_conf, ANYKA_THREAD_MIN_STACK_SIZE, -1);

    if (p_dvr_conf->stMuxConf.u8CaptureAudio == AK_TRUE)
    {printf("%s_%d here\n",__FUNCTION__, __LINE__);
        // thread_ai
        AK_BASE_ThreadCreate(&p_dvr_conf->a_pthread_t[THREAD_AI], thread_ai,
                             (void *)p_dvr_conf, ANYKA_THREAD_MIN_STACK_SIZE, -1);
        // thread_aenc
        AK_BASE_ThreadCreate(&p_dvr_conf->a_pthread_t[THREAD_AENC], thread_aenc,
                             (void *)p_dvr_conf, ANYKA_THREAD_MIN_STACK_SIZE, -1);
    }

    g_video_thread_run_flag = video_thread_run_flag;

    if(g_video_thread_run_flag)
    {printf("%s_%d here\n",__FUNCTION__, __LINE__);
        // thread_vi_venc
        //AK_BASE_ThreadCreate(&p_dvr_conf->a_pthread_t[THREAD_VI_VENC], thread_vi_venc,(void *)p_dvr_conf, ANYKA_THREAD_MIN_STACK_SIZE, 90);
	AK_BASE_ThreadCreate(&p_dvr_conf->a_pthread_t[THREAD_VI_VENC], thread_vi_venc,(void *)p_dvr_conf, ANYKA_THREAD_MIN_STACK_SIZE, -1);
    }


    return AK_SUCCESS; // Do not check whether the thread starts successfully for now
}
/* end of func */

// Stop threads to end recording
int ak_dvr_stop(struct dvr_conf *p_dvr_conf)
{
    p_dvr_conf->i_run = AK_FALSE;

    AK_BASE_ThreadJoin(p_dvr_conf->a_pthread_t[THREAD_MUX]);
    if (p_dvr_conf->stMuxConf.u8CaptureAudio == AK_TRUE)
    {
        AK_BASE_ThreadJoin(p_dvr_conf->a_pthread_t[THREAD_AI]);
        AK_BASE_ThreadJoin(p_dvr_conf->a_pthread_t[THREAD_AENC]);
    }

    // AK_MPI_AI_DisableChn(p_dvr_conf->s32AiDevId, p_dvr_conf->s32AiChnId);
    // AK_MPI_AI_Disable(p_dvr_conf->s32AiDevId);

    if(g_video_thread_run_flag)
    {
        AK_BASE_ThreadJoin(p_dvr_conf->a_pthread_t[THREAD_VI_VENC]);
    }
    AK_BASE_PrintNormal(MODULE_ID_APP, "p_dvr_conf->i_run= %d\n", p_dvr_conf->i_run);

    return AK_SUCCESS;
}

int ak_dvr_manual_stop(struct dvr_conf *p_dvr_conf)
{
    p_dvr_conf->c_mux_manual_stop_flag = AK_TRUE;
	log_info_app("here");

    // while (1)
    // {
    //     if( AK_TRUE != gc_run ) {
    //         printf("%s %d get_gcrun_flag break!\n",__func__,__LINE__);
    //         break;
    //     }
    //     usleep(100*1000);
    //     printf("waiting.......\n");
    // }
    // p_dvr_conf->c_mux_manual_stop_flag = AK_FALSE;
    // p_dvr_conf->i_run = AK_FALSE;
    // AK_MPI_AI_DisableChn( p_dvr_conf->s32AiDevId, p_dvr_conf->s32AiChnId );
    // AK_BASE_ThreadJoin( p_dvr_conf->a_pthread_t[ THREAD_MUX ] );
    // if ( p_dvr_conf->stMuxConf.u8CaptureAudio == AK_TRUE ) {
    //     AK_BASE_ThreadJoin( p_dvr_conf->a_pthread_t[ THREAD_AI ] );
    //     AK_BASE_ThreadJoin( p_dvr_conf->a_pthread_t[ THREAD_AENC ] );
    // }
    // AK_BASE_ThreadJoin( p_dvr_conf->a_pthread_t[ THREAD_VI_VENC ] );
    // AK_BASE_PrintNormal(MODULE_ID_APP, "p_dvr_conf->i_run= %d\n", p_dvr_conf->i_run );

    return AK_SUCCESS;
}

/* end of func */

// Exit recording, release data existing in the linked list
int ak_dvr_exit(struct dvr_conf *p_dvr_conf)
{
    int ret = 0;
    struct frame_entry *p_frame_now = NULL;
    struct aenc_stream_entry *p_aenc_stream_now = NULL;
    struct venc_stream_entry *p_venc_stream_now = NULL;
    struct ak_list *p_node, *p_next;

    if (p_dvr_conf->s32MuxHandle != HANDLE_NOT_INIT)
    {
        // Release AI list content when exiting
        ak_list_for_each_safe(p_node, p_next, &p_dvr_conf->list_head_ai)
        {
            p_frame_now = ak_list_item(p_node, struct frame_entry, list);
            if (p_frame_now != NULL)
            {
                MUTEX_LOCK_AI;
                ak_list_remove(&p_frame_now->list);
                MUTEX_UNLOCK_AI;
                AK_MPI_AI_ReleaseFrame(p_dvr_conf->s32AiDevId, p_dvr_conf->s32AiChnId,
                                       &p_frame_now->frame_stream, 0);
                FREE_POINT(p_frame_now);
            }
        }
        // Release AENC list content when exiting
        ak_list_for_each_safe(p_node, p_next, &p_dvr_conf->list_head_aenc)
        {
            p_aenc_stream_now = ak_list_item(p_node, struct aenc_stream_entry, list);
            if (p_aenc_stream_now != NULL)
            {
                MUTEX_LOCK_AENC;
                ak_list_remove(&p_aenc_stream_now->list);
                if (p_dvr_conf->i_buff_audio > 0)
                {
                    p_dvr_conf->i_list_audio -= p_aenc_stream_now->stream.u32Len;
                }
                MUTEX_UNLOCK_AENC;
                AK_MPI_AENC_ReleaseStream(p_dvr_conf->s32AencId, &p_aenc_stream_now->stream);
                FREE_POINT(p_aenc_stream_now);
            }
        }
        // Release VENC list content when exiting
        ak_list_for_each_safe(p_node, p_next, &p_dvr_conf->list_head_venc)
        {
            p_venc_stream_now = ak_list_item(p_node, struct venc_stream_entry, list);
            if (p_venc_stream_now != NULL)
            {
                MUTEX_LOCK_VENC;
                ak_list_remove(&p_venc_stream_now->list);
                if (p_dvr_conf->i_buff_video > 0)
                {
                    p_dvr_conf->i_list_video -= p_venc_stream_now->stream.u32Len;
                }
                MUTEX_UNLOCK_VENC;
                FREE_POINT(p_venc_stream_now->stream.pu8Data);
                FREE_POINT(p_venc_stream_now);
            }
        }
    }
    // close aenc
    if (p_dvr_conf->s32AencId != HANDLE_NOT_INIT)
    {
        AK_MPI_AENC_Close(p_dvr_conf->s32AencId);
        p_dvr_conf->s32AencId = HANDLE_NOT_INIT;
    }

    // close ai chn
    if (p_dvr_conf->s32AiChnId != HANDLE_NOT_INIT)
    {
        AK_MPI_AI_DisableChn(p_dvr_conf->s32AiDevId, p_dvr_conf->s32AiChnId);
        p_dvr_conf->s32AiChnId = HANDLE_NOT_INIT;
    }

    // close ai dev
    if (p_dvr_conf->s32AiDevId != HANDLE_NOT_INIT)
    {
        AK_MPI_AI_Disable(p_dvr_conf->s32AiDevId);
        p_dvr_conf->s32AiDevId = HANDLE_NOT_INIT;
    }

    // close venc
    if (p_dvr_conf->s32VencId != HANDLE_NOT_INIT)
    {
        AK_MPI_VENC_Close(p_dvr_conf->s32VencId);
        p_dvr_conf->s32VencId = HANDLE_NOT_INIT;
    }

    // close vpss
    if (p_dvr_conf->s32VpssGrpId != HANDLE_NOT_INIT)
    {
        printf("p_dvr_conf->s32VpssGrpId:%d, p_dvr_conf->s32VpssChnId:%d \n", p_dvr_conf->s32VpssGrpId, p_dvr_conf->s32VpssChnId);
        ret = AK_MPI_VPSS_DisableChn(p_dvr_conf->s32VpssGrpId, p_dvr_conf->s32VpssChnId);
        ret = AK_MPI_VPSS_StopGrp(p_dvr_conf->s32VpssGrpId);
        // (There is a bug in AK_MPI_VPSS_DestroyGrp deinitialization here, currently calling it will cause a segfault directly, cannot deinitialize normally, wait for Anyka to fix it later)
        // ret = AK_MPI_VPSS_DestroyGrp(p_dvr_conf->s32VpssGrpId);
        p_dvr_conf->s32VpssGrpId = HANDLE_NOT_INIT;
        p_dvr_conf->s32VpssChnId = HANDLE_NOT_INIT;
    }

    // close vi
    if (p_dvr_conf->s32ViDevId != HANDLE_NOT_INIT)
    {
        AK_MPI_VI_DisableChn(p_dvr_conf->s32ViDevId, p_dvr_conf->s32ViChnId);
        AK_MPI_VI_DisableDev(p_dvr_conf->s32ViDevId);
        AK_MPI_VI_DestroyDev(p_dvr_conf->s32ViDevId);
        p_dvr_conf->s32ViDevId = HANDLE_NOT_INIT;
    }

#ifdef AI_DENOISE_ENABLE
    DestroyAIDenoise();
#endif

    // close mux
    if (p_dvr_conf->s32MuxHandle != HANDLE_NOT_INIT)
    {
        AK_MPI_MUX_Close(p_dvr_conf->s32MuxHandle);
        p_dvr_conf->s32MuxHandle = HANDLE_NOT_INIT;
    }
    // Release buffer
    FREE_POINT(p_dvr_conf->ps8BuffRecord);
    FREE_POINT(p_dvr_conf->ps8BuffIndex);
    return AK_SUCCESS;
}
/* end of func */

// Initialize AI module
static int dvr_init_ai(struct dvr_conf *p_dvr_conf)
{
#ifdef AI_DENOISE_ENABLE
    if (InitAIDenoise()) {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "init AIDenoise failed\n");
        return AK_FAILED;
    }
#endif

    if (AK_MPI_AI_SetPubAttr(p_dvr_conf->s32AiDevId, &p_dvr_conf->stAiConf) != AK_SUCCESS)
    {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "AK_MPI_AI_SetPubAttr failure \n");
        return AK_FAILED;
    }
    if (AK_MPI_AI_Enable(p_dvr_conf->s32AiDevId) != AK_SUCCESS)
    {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "AK_MPI_AI_Enable failure\n");
        return AK_FAILED;
    }

    if (AK_MPI_AI_EnableChn(p_dvr_conf->s32AiDevId, p_dvr_conf->s32AiChnId))
    {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "AK_MPI_AI_EnableChn failure\n");
        return AK_FAILED;
    }

    // Check if frame length value is valid, if not, get default frame length based on sample rate
#if 0
    if( p_dvr_conf->stMuxConf.s32FrameLen == 0 ) {
        // Get minimum frame length based on sample rate
        p_dvr_conf->stMuxConf.s32FrameLen = ( p_dvr_conf->stAiConf.enSoundMode == AUDIO_SOUND_MODE_MONO ) ?
                                           ( AudioGetDevBufLen( p_dvr_conf->stAiConf.enSampleRate ) ) :
                                           ( AudioGetDevBufLen( p_dvr_conf->stAiConf.enSampleRate ) << 1 );
    }
    if( AK_MPI_AI_SetFrmLen( p_dvr_conf->s32AiDevId, p_dvr_conf->s32AiChnId, p_dvr_conf->stMuxConf.s32FrameLen ) ) {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "AK_MPI_AI_SetFrmLen failure\n");
        return AK_FAILED;
    }
#else
    if (p_dvr_conf->stMuxConf.s32FrameLen != 0)
    { // Set if ai frame length is not 0
        if (AK_MPI_AI_SetFrmLen(p_dvr_conf->s32AiDevId, p_dvr_conf->s32AiChnId, p_dvr_conf->stMuxConf.s32FrameLen))
        {
            AK_BASE_PrintErrorEx(MODULE_ID_APP, "AK_MPI_AI_SetFrmLen failure\n");
            return AK_FAILED;
        }
    }
    else
    { // Get default frame rate if ai frame length is 0
        if (AK_MPI_AI_GetFrmLen(p_dvr_conf->s32AiDevId, p_dvr_conf->s32AiChnId, &p_dvr_conf->stMuxConf.s32FrameLen))
        {
            AK_BASE_PrintErrorEx(MODULE_ID_APP, "AK_MPI_AI_GetFrmLen failure\n");
            return AK_FAILED;
        }
    }
#endif
    if (AK_MPI_AI_LoadConf(p_dvr_conf->s32AiDevId, p_dvr_conf->s32AiChnId,
                           p_dvr_conf->pc_ai_conf_file, p_dvr_conf->pc_ai_conf_section))
    {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "AK_MPI_AI_LoadConf failed \n");
        return AK_FAILED;
    }
    // Print corresponding parameters of ai
    AK_BASE_PrintNormal(MODULE_ID_APP,
                        "enSampleRate= %d enSampleBits=%d enSoundMode= %d "
                        "s32FrameLen= %d\n",
                        p_dvr_conf->stAiConf.enSampleRate,
                        p_dvr_conf->stAiConf.enSampleBits,
                        p_dvr_conf->stAiConf.enSoundMode,
                        p_dvr_conf->stMuxConf.s32FrameLen);
    return AK_SUCCESS;
}
/* end of func */

static int dvr_init_vi_vpss(struct dvr_conf *p_dvr_conf)
{
    VI_RECTANGLE_S stViRectangle = {0};
    VI_DEV_ATTR_S stViDevConf = {0};
    VI_CHN_ATTR_S stViChnConf = {0};
    VPSS_GRP_ATTR_S stGrpAttr = {0};
    VPSS_CHN_ATTR_S stVpssChnAttr = {0};

#if 0
    stViDevConf.u32Width = p_dvr_conf->i_width;
    stViDevConf.u32Height = p_dvr_conf->i_height;
    stViDevConf.stCropInfo.u32Width = p_dvr_conf->i_width;;
    stViDevConf.stCropInfo.u32Height = p_dvr_conf->i_height;
#else
    stViDevConf.u32Width = 320;
    stViDevConf.u32Height = 184;
    stViDevConf.stCropInfo.u32Width = stViDevConf.u32Width;
    stViDevConf.stCropInfo.u32Height = stViDevConf.u32Height;
#endif

    stViDevConf.bFlip = 0;
    stViDevConf.enIntfMode = (VI_INTF_MODE_E)VI_INTF_MIPI_2;
    stViDevConf.bMirror = 0;
    stViDevConf.u32Fps = p_dvr_conf->i_fps;
    stViDevConf.stCropInfo.u32PointX = 0;
    stViDevConf.stCropInfo.u32PointY = 0;

    // Open VI device
    if (AK_MPI_VI_CreateDev(p_dvr_conf->s32ViDevId, &stViDevConf) != AK_SUCCESS)
    {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "AK_MPI_VI_CreateDev failure\n");
        goto dvr_init_vi_vpss_failure;
    }

    AK_MPI_VI_GetSensorRes(p_dvr_conf->s32ViDevId, &stViRectangle);
    stViDevConf.u32Width = stViRectangle.u32Width;
    stViDevConf.u32Height = stViRectangle.u32Height;
    stViDevConf.stCropInfo.u32Width = stViRectangle.u32Width;
    stViDevConf.stCropInfo.u32Height = stViRectangle.u32Height;
    AK_MPI_VI_SetDevAttr(p_dvr_conf->s32ViDevId, &stViDevConf);

    stViChnConf.enMode = VI_CHN_MODE_AUTO; // VI_CHN_MODE_AUTO; //only user mode can get raw data.
    stViChnConf.enOutputPixFmt = PIXEL_FORMAT_YUV420SP;
    stViChnConf.u32Fps = p_dvr_conf->i_fps; // no framerate ctrl.
    stViChnConf.u32FrameDepth = p_dvr_conf->i_frame_depth;
    stViChnConf.u32Width = stViRectangle.u32Width; // the same res to device.
    stViChnConf.u32Height = stViRectangle.u32Height;
    stViChnConf.u32MaxPerFrameSize = 0; // use default value.
    AK_BASE_PrintErrorEx(MODULE_ID_APP, "stViChnConf.u32FrameDepth:%d\n",stViChnConf.u32FrameDepth);
    if (AK_MPI_VI_SetChnAttr(p_dvr_conf->s32ViDevId, p_dvr_conf->s32ViChnId, &stViChnConf))
    {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "AK_MPI_VI_SetChnAttr failure \n");
        goto dvr_init_vi_vpss_failure;
    }

    stGrpAttr.u32Width = stViRectangle.u32Width;
    stGrpAttr.u32Height = stViRectangle.u32Height;
    stGrpAttr.bEnGrpBypass = 0;
    stGrpAttr.bFlip = 0;
    stGrpAttr.bMirror = 0;
    stGrpAttr.enInputPixFmt = PIXEL_FORMAT_YUV420SP;
    stGrpAttr.enWorkMode = VPSS_WORK_AUTO;
    strcpy((char *)stGrpAttr.pu8IspCfgPath, p_dvr_conf->ac_isp_cfg);
    AK_U32 u32SensorId = 0;
    AK_MPI_VI_GetSensorId(p_dvr_conf->s32VpssGrpId, &u32SensorId);
    stGrpAttr.s32AttachSensorId = u32SensorId;
    if (AK_MPI_VPSS_CreateGrp(p_dvr_conf->s32VpssGrpId, &stGrpAttr) != AK_SUCCESS)
    {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "Create vpss Grp fail\n");
        goto dvr_init_vi_vpss_failure;
    }

    /* Main channel */
    stVpssChnAttr.u32Width = p_dvr_conf->i_width;
    stVpssChnAttr.u32Height = p_dvr_conf->i_height;
    stVpssChnAttr.enChnMode = VPSS_CHN_FRAME_MODE;
    stVpssChnAttr.enOutputPixFmt = PIXEL_FORMAT_YUV420SP;

    // Use adaptive cropping algorithm to calculate optimal cropping parameters
    calculate_adaptive_crop(
        stViRectangle.u32Width,     // Input frame width
        stViRectangle.u32Height,    // Input frame height
        stVpssChnAttr.u32Width,     // Output frame width
        stVpssChnAttr.u32Height,    // Output frame height
        &stVpssChnAttr.stCropInfo.u32Width,   // Output: Crop width
        &stVpssChnAttr.stCropInfo.u32Height,  // Output: Crop height
        &stVpssChnAttr.stCropInfo.u32PointX,  // Output: Crop start X coordinate
        &stVpssChnAttr.stCropInfo.u32PointY   // Output: Crop start Y coordinate
    );

	stVpssChnAttr.stCropInfo.u32Width  = stViRectangle.u32Width, 	// Input frame width
	stVpssChnAttr.stCropInfo.u32Height = stViRectangle.u32Height;
	stVpssChnAttr.stCropInfo.u32PointX = 0;  // Output: Crop start X coordinate
	stVpssChnAttr.stCropInfo.u32PointY = 0;  // Output: Crop start Y coordinate



    stVpssChnAttr.u32MaxBufSize = 0;
    stVpssChnAttr.s32Fps = p_dvr_conf->i_fps;
    stVpssChnAttr.u32FrameDepth = p_dvr_conf->i_frame_depth;
    AK_BASE_PrintErrorEx(MODULE_ID_APP, "stVpssChnAttr.u32FrameDepth:%d\n",stVpssChnAttr.u32FrameDepth);

    if (AK_MPI_VPSS_SetChnAttr(p_dvr_conf->s32VpssGrpId, p_dvr_conf->s32VpssChnId, &stVpssChnAttr) != AK_SUCCESS)
    {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "set vpss chn attr fail \n");
        return AK_FAILED;
    }
    /* Sub channel */
    /*
    stVpssChnAttr.u32Width = p_dvr_conf->i_sub_width;;
    stVpssChnAttr.u32Height = p_dvr_conf->i_sub_height;
    if( AK_MPI_VPSS_SetChnAttr( p_dvr_conf->s32VpssGrpId, p_dvr_conf->s32VpssChnSubId, &stVpssChnAttr) != AK_SUCCESS) {
        AK_BASE_PrintErrorEx(MODULE_ID_APP,"set vpss sub chn attr fail \n");
        goto dvr_init_vi_vpss_failure;
    }
    */

    if (AK_MPI_VI_BindVpssGrp(p_dvr_conf->s32ViDevId, p_dvr_conf->s32ViChnId, p_dvr_conf->s32VpssGrpId) != AK_SUCCESS)
    {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "bind vpss grp fail \n");
        goto dvr_init_vi_vpss_failure;
    }
    if (AK_MPI_VPSS_BindChn(p_dvr_conf->s32VpssGrpId, p_dvr_conf->s32ViChnId) != AK_SUCCESS)
    {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "bind vi device fail \n");
        goto dvr_init_vi_vpss_failure;
    }

    if (AK_MPI_VI_EnableDev(p_dvr_conf->s32ViDevId) != AK_SUCCESS)
    {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "vi device enable device failure!\n");
        goto dvr_init_vi_vpss_failure;
    }

    if (AK_MPI_VI_EnableChn(p_dvr_conf->s32ViDevId, p_dvr_conf->s32ViChnId) != AK_SUCCESS)
    {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "vi channel[0] enable failure!\n");
        goto dvr_init_vi_vpss_failure;
    }

    if (AK_MPI_VPSS_StartGrp(p_dvr_conf->s32VpssGrpId) != AK_SUCCESS)
    {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "start vpss grp fail \n");
        goto dvr_init_vi_vpss_failure;
    }
    if (AK_MPI_VPSS_EnableChn(p_dvr_conf->s32VpssGrpId, p_dvr_conf->s32VpssChnId) != AK_SUCCESS)
    {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "enable vpss chn fail \n");
        goto dvr_init_vi_vpss_failure;
    }
    /*
    if( AK_MPI_VPSS_EnableChn( p_dvr_conf->s32VpssGrpId, p_dvr_conf->s32VpssChnSubId ) != AK_SUCCESS ) {
        AK_BASE_PrintErrorEx(MODULE_ID_APP,"enable vpss chn fail \n");
        goto dvr_init_vi_vpss_failure;
    }
    */

    /*
    stViChnConf.u32Width = stViRectangle.u32Width;
    stViChnConf.u32Height = stViRectangle.u32Height;
    stViChnConf.u32Fps = p_dvr_conf->i_fps;
    stViChnConf.u32FrameDepth = p_dvr_conf->i_frame_depth;
    if ( AK_MPI_VI_SetChnAttr( p_dvr_conf->s32ViDevId, p_dvr_conf->s32ViChnId, &stViChnConf) != AK_SUCCESS ) {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "vi device set channel attribute failure!\n" );
        goto dvr_init_vi_vpss_failure;
    }
    // Enable device
    if ( AK_MPI_VI_EnableDev( p_dvr_conf->s32ViDevId ) != AK_SUCCESS ) {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "vi device enable device failure!\n" );
        goto dvr_init_vi_vpss_failure;
    }
    // Enable main channel
    if ( AK_MPI_VI_EnableChn( p_dvr_conf->s32ViDevId, p_dvr_conf->s32ViChnId ) != AK_SUCCESS ) {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "vi channel[0] enable failure!\n" );
        goto dvr_init_vi_vpss_failure;
    }
    // Enable sub channel
    if ( AK_MPI_VI_EnableChn( p_dvr_conf->s32ViDevId, p_dvr_conf->s32ViChnId ) != AK_SUCCESS ) {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "vi channel[1] enable failure!\n" );
        goto dvr_init_vi_vpss_failure;
    }
    */

    p_dvr_conf->s32ViDevId = HANDLE_INIT;
    return AK_SUCCESS;

dvr_init_vi_vpss_failure:
    p_dvr_conf->s32ViDevId = HANDLE_NOT_INIT;
    return AK_FAILED;
}
/* end of func */

// Open VENC interface
static int dvr_init_venc(struct dvr_conf *p_dvr_conf)
{
    /*
    VENC_ATTR_S *pstVencAttrs       = &p_dvr_conf->stVencConf.stVencAttrs;   //venc attr
    VENC_RC_ATTRS_S *pstRcAttrs     = &p_dvr_conf->stVencConf.stRcAttrs;     //rc att
    VENC_EXT_ATTRS_S *pstExtAttrs   = &p_dvr_conf->stVencConf.stExtAttrs;    //ext attr
    */
    AK_U8 u8BrMode = p_dvr_conf->i_br_mode;
    AK_S32 s32TargetKbps = p_dvr_conf->i_kbps;

    p_dvr_conf->stVencConf.stExtAttrs.stAdvance.s16IPDelta = 0;         // Delta
    p_dvr_conf->stVencConf.stExtAttrs.stAdvance.u32MaxPictureISize = 0; // MAX I Frame size (bits)
    p_dvr_conf->stVencConf.stExtAttrs.stAdvance.u32MaxPicturePSize = 0; // MAX P Frame size (bits)
    p_dvr_conf->stVencConf.stExtAttrs.stAdvance.u32MaxPictureBSize = 0; // Max B Frame size (bits)
    p_dvr_conf->stVencConf.stExtAttrs.stAdvance.bEnableSRD = AK_FALSE;  // SRD
    p_dvr_conf->stVencConf.stExtAttrs.stAdvance.u8SRDThreshold = 0;     // SRDThreshold

    switch (u8BrMode)
    {
    case BR_MODE_VBR:
        p_dvr_conf->stVencConf.stRcAttrs.enRcMode = VENC_RC_MODE_H264_VBR;        // vbr
        p_dvr_conf->stVencConf.stRcAttrs.stH264Vbr.u16MaxBitrate = s32TargetKbps; // target kbps
        p_dvr_conf->stVencConf.stRcAttrs.stH264Vbr.u16Quality = 50;               // quality
        break;
    case BR_MODE_CBR:
        p_dvr_conf->stVencConf.stRcAttrs.enRcMode = VENC_RC_MODE_H264_CBR;           // cbr
        p_dvr_conf->stVencConf.stRcAttrs.stH264Cbr.u16TargetBitrate = s32TargetKbps; // target kbps
        break;
    case BR_MODE_AVBR:
        p_dvr_conf->stVencConf.stRcAttrs.enRcMode = VENC_RC_MODE_H264_AVBR;        // VBR
        p_dvr_conf->stVencConf.stRcAttrs.stH264Avbr.u16MinQP = 20;                 // min qp
        p_dvr_conf->stVencConf.stRcAttrs.stH264Avbr.u16MaxQP = 48;                 // max qp
        p_dvr_conf->stVencConf.stRcAttrs.stH264Avbr.u16MaxBitrate = s32TargetKbps; // target Kbits
        p_dvr_conf->stVencConf.stRcAttrs.stH264Avbr.u16ChangePos = 0;              // targetBitrate = maxBitrate*changePos/100
        break;
    case BR_MODE_CONST_QP:
        p_dvr_conf->stVencConf.stRcAttrs.enRcMode = VENC_RC_MODE_H264_CONST_QP; // const qp
        p_dvr_conf->stVencConf.stRcAttrs.stH264ConstQp.u16QP = 50;              // qp
        break;
    }

    switch (p_dvr_conf->stVencConf.stVencAttrs.enOutputType)
    {
    case H264_ENC_TYPE:
        p_dvr_conf->stVencConf.stVencAttrs.stH264Attr.enProfileMode = VENC_PROFILE_MAIN;
        break;
    case HEVC_ENC_TYPE:
        p_dvr_conf->stVencConf.stVencAttrs.stH265Attr.enProfileMode = VENC_PROFILE_HEVC_MAIN;
        break;
    case MJPEG_ENC_TYPE:
        // p_dvr_conf->stVencConf.profile = PROFILE_JPEG; // JPEG has no profile, this enum is defined but not used
        break;
    default:
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "enOutputType= %d error.\n", p_dvr_conf->stVencConf.stVencAttrs.enOutputType);
        return AK_FAILED;
    }
    return AK_MPI_VENC_Open(p_dvr_conf->s32VencId, &p_dvr_conf->stVencConf);
}

int get_gcrun_flag(char *gc_run_out, char *saving_flag_out)
{
    *gc_run_out = gc_run;
    *saving_flag_out = saving_flag;
    return 0;
}
