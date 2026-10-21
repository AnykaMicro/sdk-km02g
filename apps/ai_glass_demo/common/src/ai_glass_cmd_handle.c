/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd. */
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "ak_vpss.h"


#include "fun_log.h"
#include "ai_glass_cmd_handle.h"
#include "ak_snap_demo.h"

#include "sdnand_manager.h"
#include "wifi_manager.h"
#include "ak_gyro_app.h"
#include "ai_glass_isp_quick.h"
#include "ai_glass_cmd_ipi.h"
#include "ak_eis.h"

#ifdef RECORD_SUPPORT_ENABLE
#include "ak_record_demo.h"
#endif

#ifdef RTSP_SUPPORT_ENABLE
#include "ak_rtsp_demo.h"
#endif

#ifdef EIS_ENABLED
#include "video_eis.h"
#endif

#ifdef SOUND_REC_SUPPORT_ENABLE
#include "ak_sound_demo.h"
#endif


#define REC_SEC_MAX 600
static pthread_t mount_thread;

static pthread_t rtsp_thread;
static pthread_t record_thread;
static pthread_t audio_record_thread;

// RTSP and recording thread control flags
static volatile int rtsp_thread_running = 0;
static volatile int record_thread_running = 0;
static volatile int audio_record_thread_running = 0;

static volatile int rtsp_thread_exit = 0;
static volatile int record_thread_exit = 0;
static volatile int audio_record_thread_exit = 0;

qs_demo_dev_t qs_dev;
static qs_demo_dev_t *pdev = &qs_dev;
static AK_S32 g_s32DevCnt = 1;


int handle_get_num_img_video(void)
{
    int timeout = 200;
    int i= 0;
    int ret = 0;
    uint8_t glass_info_param[256] = {0};
    uint16_t glass_info_param_len = 0;
    int result = 0;

    const char picture_extensions[][10] = {
        "jpg", "JPG",           // JPEG format
        "jpeg", "JPEG",         // JPEG format
        "png", "PNG",           // PNG format
        "bmp", "BMP",           // BMP format
        "raw", "RAW"           // RAW original image
    };
    const int picture_ext_count = sizeof(picture_extensions) / sizeof(picture_extensions[0]);

    // Video file extension (case sensitive)
    char video_extensions[][10] = {
        "mp4", "MP4",           // MPEG-4 video
        "avi", "AVI"           // Audio Video Interleave
    };
    const int video_ext_count = sizeof(video_extensions) / sizeof(video_extensions[0]);

    wait_for_sdnand(BT_CMD_NO_CMD);

    // Loop to detect mount status, exit if timeout
    while(!sd_nand_is_mounted(SD_NAND_DEVICE_PATH))
    {
        usleep(100000);
        timeout--;

        if(timeout < 0) break;
    }

    // Mount timeout
    if(timeout <= 0 && !sd_nand_is_mounted(SD_NAND_DEVICE_PATH))
    {
        log_info_app("mount detect timeout: %d\n",timeout);
        result = RETURN_CODE_SYSTEM_NOT_MOUNTED;
        memset(glass_info_param, 0, sizeof(glass_info_param));
        glass_info_param_len = ai_glass_info_pack_data(result, glass_info_param, NULL, 0);
        set_glass_info_v4(APP_CMD_P2P, (char *)glass_info_param, glass_info_param_len);
        return 0;
    }

    // If mounted, report picture and video count
    unsigned short picture_count = 0;

    for(i = 0; i < picture_ext_count; i++)
    {
        ret = sd_nand_count_files_by_extension(PICTURES_PATH, picture_extensions[i], 0);
        if(ret > 0)
        {
            picture_count += ret;
            log_info_app("Found %d .%s files\n", ret, picture_extensions[i]);
        }
        else if(ret < 0)
        {
            log_error_app("Error counting .%s files: %d\n", picture_extensions[i], ret);
        }
    }

    unsigned short video_count = 0;

    for(i = 0; i < video_ext_count; i++)
    {
        ret = sd_nand_count_files_by_extension(VIDEOS_PATH, video_extensions[i], 1);
        if(ret > 0)
        {
            video_count += ret;
            log_info_app("Found %d .%s files\n", ret, video_extensions[i]);
        }
        else if(ret < 0)
        {
            log_error_app("Error counting .%s files: %d\n", video_extensions[i], ret);
        }
    }

    // Reply
    log_debug_app("  -----File scan complete------");
    log_debug_app("  Pictures: %d files", picture_count);
    log_debug_app("  Videos: %d files", video_count);
    printf("  Total: %d files\n", picture_count + video_count);
    log_debug_app("  -----------------------------\n");

    if(picture_count > 0 || video_count > 0){
        result = RETURN_CODE_SUCCESS;
    }else{
        result = RETURN_CODE_SD_NAND_NO_FILES;
    }

    memset(glass_info_param, 0, sizeof(glass_info_param));
    glass_info_param_len = ai_glass_info_pack_data(result, glass_info_param, NULL, 0);
    set_glass_info_v4(APP_CMD_P2P, (char *)glass_info_param, glass_info_param_len);
    return result;
}



#ifdef RTSP_SUPPORT_ENABLE

void *rtsp_thread_func(void *arg)
{
    log_info_app("RTSP thread started\n");
    rtsp_thread_running = 1;
    rtsp_thread_exit = 0;
    set_work_mode(WORK_MODE_RTSP);
    run_rtsp_live_stream_with_eis(gm_config.video_isp_config);

    rtsp_thread_running = 0;
    log_info_app("RTSP thread exited\n");

    return NULL;
}

int start_rtsp_live_stream(void)
{
    int result = RETURN_CODE_SUCCESS;

    log_info_app("STA: LIVE_STREAM ON\n");

    if (rtsp_thread_running) {
        result = RETURN_CODE_DEVICE_BUSY;
        log_info_app("RTSP thread is already running\n");
    } else {
        int ret = pthread_create(&rtsp_thread, NULL, rtsp_thread_func, NULL);
        if (ret == 0) {
            log_info_app("RTSP thread created successfully\n");
        } else {
            result = RETURN_CODE_CREATE_FAIL;
            log_error_app("Failed to create RTSP thread: %d\n", ret);
        }
    }
    return result;
}

int stop_rtsp_live_stream(void)
{
    int result = RETURN_CODE_SUCCESS;

    log_info_app("Worker thread: STOP_LIVE_STREAM\n");

    if (rtsp_thread_running) {
        rtsp_thread_exit = 1;
        set_work_mode(WORK_MODE_IDLE);
        pthread_join(rtsp_thread, NULL);
        log_info_app("RTSP thread stopped\n");
    } else {
        result = RETURN_CODE_DEVICE_NO_USE;
        log_error_app("RTSP thread is not running\n");
    }
    return result;
}

#endif


/**
 * @brief Take photo sample
 *
 * @param isp_conf_path
 * @return int
 */
static int run_take_photo(char *isp_conf_path, int fps, int frame_depth)
{
    int ret = 0;

    AK_Vpss_Isp_Set_AwbParamAccess(0);log_info_app("here\n");

    ret = ak_snap_env_init(isp_conf_path, fps, frame_depth);
    if(ret != 0)
    {
        log_error_app("ak_snap_env_init failed! ret:%d\n", ret);
        return ret;
    }

    ret = ak_snap_photo(PICTURES_PATH);
    if(ret != 0)
    {
        log_error_app("ak_snap_photo failed! ret:%d\n", ret);
        return ret;
    }

    ret = ak_snap_env_destory();
    if(ret != 0)
    {
        log_error_app("ak_snap_env_destory failed! ret:%d\n", ret);
        return ret;
    }

	log_info_app("here\n");

    return 0;
}

int start_take_photo(void)
{
    int result = RETURN_CODE_SUCCESS;
    for (AK_U32 u32devid = 0; u32devid < g_s32DevCnt; u32devid++)
    {
        get_vi_dev_info(u32devid, &qs_dev.dev_info[u32devid]);
        get_venc_attr(u32devid, &qs_dev.venc_chn[u32devid]);
    }

    unsigned int i_fps = pdev->dev_info[VIDEO_DEV0].frame_rate;
    unsigned int i_frame_depth = pdev->dev_info[VIDEO_DEV0].main_chn_buf;
    log_info_app("photo i_fps:%u i_frame_depth:%u\n", i_fps, i_frame_depth);
    int ret = run_take_photo(gm_config.snap_isp_config, i_fps, i_frame_depth);
    if(ret != 0){
     // Take photo failed
        result = RETURN_CODE_TIMEOUT;
    }

    return result;
}


#ifdef RTSP_SUPPORT_ENABLE

/**
 * @brief RTSP event callback function
 *
 * @param event RTSP event type
 * @param client_ip Client IP address
 */
static void rtsp_event_callback(rtsp_event_type_t event, const char* client_ip)
{
    int result = 0;
    switch (event) {
        case RTSP_EVENT_LOGIN:
            log_info_app("=== RTSP client login ===\n");
            log_info_app("Client IP: %s\n", client_ip);
            log_info_app("login time: %s\n", ctime(&(time_t){time(NULL)}));
            log_info_app("==================\n");
            result = 1;
            //set_glass_info_v4_notify(CMD_RTSP_LOGIN_NOTIFY,(char *)&result,sizeof(result));
            break;

        case RTSP_EVENT_LOGOUT:
            log_info_app("=== RTSP client logout ===\n");
            // log_info_app("Client IP: %s", client_ip);
            log_info_app("logout time: %s\n", ctime(&(time_t){time(NULL)}));
            log_info_app("==================\n");
            result = 1;
            //set_glass_info_v4_notify(CMD_RTSP_LOGOUT_NOTIFY,(char *)&result,sizeof(result));
            break;

        default:
            log_error_app("FTP Monitor: Unknown event type: %d\n", event);
            break;
    }
}

/**
 * @brief RTSP Live video stream demo
 *
 * @param isp_conf_path
 * @return int
 */
int run_rtsp_live_stream_with_eis(char *isp_conf_path)
{
    int ret = 0;
    // Initialize video environment
	ret = ak_rtsp_demo_env_init(isp_conf_path, (gm_config.rtsp_param.eis_enable == 0), rtsp_event_callback);
    if (ret != 0)
    {
        log_error_app("Failed to initialize video environment");
        ak_sdk_env_destory();
        return -1;
    }

	if(gm_config.rtsp_param.eis_enable == 1)
	{
	// Eis init
	    ret = eis_init(0, 0,gm_config.rtsp_param.main_vi_width,gm_config.rtsp_param.main_vi_height,
	    gm_config.rtsp_param.main_chn_width, gm_config.rtsp_param.main_chn_height, 4, send_stream_to_rtsp);
	    if (ret != 0)
	    {
	        log_error_app("Failed to initialize EIS system");
	        ak_rtsp_demo_env_destory(0);
	        ak_sdk_env_destory();
	        return -1;
	    }

	    // Eis run
	    ret = eis_start_threads();
	    if (ret != 0)
	    {
	        log_error_app("Failed to start EIS threads");
	        eis_cleanup();
	        ak_rtsp_demo_env_destory(0);
	        ak_sdk_env_destory();
	        return -1;
	    }
	}

    ak_rtsp_demo_audio_env_init();

    // run for a while
    log_info_app("rtsp server running... Press Ctrl+C to stop");
    while (!rtsp_thread_exit)
    {
        sleep(1); // Check exit flag, not fixed 120 seconds run
    }

	if(gm_config.rtsp_param.eis_enable == 1)
	{
	    log_info_app("EIS system OUT!!!");
	    // stop
	    eis_stop_threads();

	    // clean
	    eis_cleanup();
	}

	ak_rtsp_demo_env_destory(gm_config.rtsp_param.eis_enable == 0);

    return 0;
}
#endif



#ifdef RECORD_SUPPORT_ENABLE


/**
 * @brief
 *
 * @param isp_conf_path
 * @param record_direction
 * @param record_sec
 * @param record_min_sec
 * @param frame_depth
 * @param record_file_name_out
 * @return int
 */
static int run_record(char *isp_conf_path, int record_direction,int record_sec, int record_min_sec, int frame_depth,char **record_file_name_out)
{
		// Check path
    ensure_directory_exists(VIDEOS_PATH);

    int already_record_sec = 0;
    int ret = 0;

	if(gm_config.record_param.eis_enable == 1)
	{
	    // Eis init
	    ret = eis_init(0, 0, gm_config.record_param.main_vi_width, gm_config.record_param.main_vi_height,
		gm_config.record_param.main_chn_width, gm_config.record_param.main_chn_height, frame_depth, send_stream_to_record);
	    if (ret != 0)
	    {
	        log_error_app("Failed to initialize EIS system");
	        ak_sdk_env_destory();
	        return -1;
	    }
	}
	else
	{
		ak_frame_info_store_open();
		gyro_init(0);
	}

    ret = ak_record_start(isp_conf_path, record_sec, frame_depth, (gm_config.record_param.eis_enable == 0) );

    if(ret != 0)
    {
        log_error_app("ak_record_start failed ret:%d",ret);
        return ret;
    }

	if(gm_config.record_param.eis_enable == 1)
    {
		// Eis start
	    ret = eis_start_threads();
	    if (ret != 0)
	    {
	        log_error_app("Failed to start EIS threads");
	        eis_cleanup();
	        ak_sdk_env_destory();
	        return -1;
	    }
	}

    already_record_sec = ak_record_wait_stop();


	if(gm_config.record_param.eis_enable == 1)
	{
	    // stop
	    eis_stop_threads();

	    // clean
	    eis_cleanup();
	}

    ret = ak_record_stop_and_exit(record_file_name_out);
    if(ret != 0)
    {
        log_error_app("ak_record_stop_and_exit failed ret:%d",ret);
        return ret;
    }

    if(gm_config.record_param.eis_enable == 0)
	{
		char dst_name[512] = {0};
//		snprintf(dst_name, sizeof(dst_name), "%s_frame.csv", g_eis_data_file_name);
		ak_frame_info_store_close(dst_name);
        snprintf(dst_name, sizeof(dst_name), "%s_eis.bin", g_eis_data_file_name);

		gyro_deinit();
        eis_mux_file((double*)gm_config.eis_param.data,sizeof(gm_config.eis_param.data),GYRO_DATA_PATH,dst_name);

        //ak_rename_file(GYRO_FRAME_MUX_FILE,dst_name);

    }

    return already_record_sec;
}
#endif


#ifdef SOUND_REC_SUPPORT_ENABLE
static void *audio_record_thread_func(void *arg)
{
    record_thread_params_t *params = (record_thread_params_t *)arg;
    int ret = 0;
    int already_record_sec = 0;
    unsigned int cmd_bk = 0;
    uint8_t result = 0;

    uint8_t glass_info_param[256] = {0};
    uint16_t glass_info_param_len = 0;

    log_info_app("Audio record thread started\n");
    audio_record_thread_running = 1;
    audio_record_thread_exit = 0;

    log_info_app("Audio recording params: duration=%u sec, min_duration=%u sec\n",
           params->record_duration, params->record_min_duration);

    // Start recording
    ret = ak_sound_demo_start_record(AUDIO_PATH,
                                    params->record_duration,
                                    params->record_min_duration);
    if (ret != 0) {
        log_error_app("Failed to start audio recording: %d\n", ret);
        audio_record_thread_running = 0;
        return NULL;
    }

    log_info_app("Audio recording started successfully\n");

    // Monitor recording status
    while (!audio_record_thread_exit)
    {
        already_record_sec = ak_sound_demo_get_already_record_times();

        // Check if recording is finished (auto stop or duration reached)
        if (ak_sound_demo_is_recording_finished()) {
            log_info_app("Audio recording finished, exiting monitoring loop\n");
            break;
        }

        // Check for manual stop
        if (audio_record_thread_exit) {
            log_info_app("Audio recording manual stop requested\n");
            break;
        }

        // Print recording status every second
        log_info_app("Audio recording progress: %u/%u seconds\n",
               already_record_sec, params->record_duration);

        sleep(1);
    }

    // Stop audio recording
    log_info_app("Stopping audio recording...\n");
    ret = ak_sound_demo_stop_record();

    if (ret == 0) {
        log_info_app("Audio recording completed successfully\n");
        if(audio_record_thread_exit)
        {
            // Manually stopped recording
            result = RETURN_CODE_SUCCESS;
        }
        else
        {
            // Duration reached recording
            result = RETURN_CODE_SUCCESS;
        }
    } else if (ret == -1) {
        log_error_app("Audio recording failed: duration too short, file deleted");
        result = RETURN_CODE_RECORD_TIME_SHORT;
    }

    memset(glass_info_param, 0, sizeof(glass_info_param));
    glass_info_param_len = ai_glass_info_pack_data(result, glass_info_param, NULL, 0);
    set_glass_info_v4(BT_CMD_RECORD_AUDIO, (char *)glass_info_param, glass_info_param_len);

    ret = ak_sound_demo_destroy();
    audio_record_thread_running = 0;
    log_info_app("Audio record thread exited\n");

    return NULL;
}
#endif

#ifdef RECORD_SUPPORT_ENABLE
/**
 * @brief Record thread function
 *
 * @param arg
 * @return void*
 */
static void *record_thread_func(void *arg)
{
    record_thread_params_t *params = (record_thread_params_t *)arg;
    int ret = 0;
    unsigned int cmd_bk = 0;
    uint8_t result = RETURN_CODE_SUCCESS;
    uint8_t glass_info_param[256] = {0};
    uint16_t glass_info_param_len = 0;

    char *record_file_name = NULL;

    log_info_app(" Record thread started\n");
    AK_Vpss_Isp_Set_AwbParamAccess(0);
    record_thread_running = 1;
    record_thread_exit = 0;

    log_info_app("isp_conf_path:%s record_direction:%u record_duration:%u record_min_duration:%u frame_depth:%d\n",
                    params->isp_conf_path,
                    params->record_direction,
                    params->record_duration,
                    params->record_min_duration,
                    params->frame_depth);

    // Call record function
    ret = run_record(params->isp_conf_path,
                    params->record_direction,
                    params->record_duration,
                    params->record_min_duration,
                    params->frame_depth,
                    &record_file_name);
    log_info_app("run_record ret:%d\n",ret);
    if (ret >= (int)params->record_duration)
    {
        // Confirm that the recording duration exceeds the configured duration, then judge as recording complete.
        log_info_app("ret(%d) >= params->record_duration(%d) record done!\n",ret,params->record_duration);
        if(is_bit7_set(global_cmd_id)){
            memset(glass_info_param, 0, sizeof(glass_info_param));
            glass_info_param_len = ai_glass_info_pack_data(result, glass_info_param, NULL, 0);
            set_glass_info_v4(APP_CMD_RECORD, (char *)glass_info_param, glass_info_param_len);
        }else{
            memset(glass_info_param, 0, sizeof(glass_info_param));
            glass_info_param_len = ai_glass_info_pack_data(result, glass_info_param, NULL, 0);
            set_glass_info_v4(BT_CMD_RECORD, (char *)glass_info_param, glass_info_param_len);
        }
    }
    else if (ret > (int)params->record_min_duration && ret < (int)params->record_duration)
    {
        // Recording time is between minimum time and maximum time, judge as manual stop recording
        log_info_app("record manually stop\n");
        if(is_bit7_set(global_cmd_id)){
            memset(glass_info_param, 0, sizeof(glass_info_param));
            glass_info_param_len = ai_glass_info_pack_data(result, glass_info_param, NULL, 0);
            set_glass_info_v4(APP_CMD_RECORD, (char *)glass_info_param, glass_info_param_len);
        }else{
            memset(glass_info_param, 0, sizeof(glass_info_param));
            glass_info_param_len = ai_glass_info_pack_data(result, glass_info_param, NULL, 0);
            set_glass_info_v4(BT_CMD_RECORD, (char *)glass_info_param, glass_info_param_len);
        }
    }
    else if(ret >= 0 && ret <= (int)params->record_min_duration)
    {
        // Recording time is less than or equal to record_min_duration, judge as recording too short, then delete
        result = RETURN_CODE_RECORD_TIME_SHORT;
        if(record_file_name)
        {
            log_info_app("delete file: %s\n",record_file_name);
            remove(record_file_name);
        }
        if(is_bit7_set(global_cmd_id)){
            memset(glass_info_param, 0, sizeof(glass_info_param));
            glass_info_param_len = ai_glass_info_pack_data(result, glass_info_param, NULL, 0);
            set_glass_info_v4(APP_CMD_RECORD, (char *)glass_info_param, glass_info_param_len);
        }else{
            memset(glass_info_param, 0, sizeof(glass_info_param));
            glass_info_param_len = ai_glass_info_pack_data(result, glass_info_param, NULL, 0);
            set_glass_info_v4(BT_CMD_RECORD, (char *)glass_info_param, glass_info_param_len);
        }
    }
    else //if(ret == -1)
    {
        // Recording start failed
        result = RETURN_CODE_CREATE_FAIL;
        log_error_app("start record failed!\n");
        if(is_bit7_set(global_cmd_id)){
            memset(glass_info_param, 0, sizeof(glass_info_param));
            glass_info_param_len = ai_glass_info_pack_data(result, glass_info_param, NULL, 0);
            set_glass_info_v4(APP_CMD_RECORD, (char *)glass_info_param, glass_info_param_len);
        }else{
            memset(glass_info_param, 0, sizeof(glass_info_param));
            glass_info_param_len = ai_glass_info_pack_data(result, glass_info_param, NULL, 0);
            set_glass_info_v4(BT_CMD_RECORD, (char *)glass_info_param, glass_info_param_len);
        }
    }


    sync();
    //record_thread_running = 0;
    set_led_brightness("state_led", 0);
    log_info_app("Record thread exited\n");
    stop_storage_monitor();
    return NULL;
}
#endif



int start_video_record(int record_time)
{
    uint8_t result = RETURN_CODE_SUCCESS;
    uint8_t glass_info_param[256] = {0};
    uint16_t glass_info_param_len = 0;

	static record_thread_params_t record_params = {0};

    log_info_app("Worker thread: RECORD\n");
    if (record_thread_running){
        log_info_app("Record thread is already running\n");
        result = RETURN_CODE_DEVICE_BUSY; // Already running
        if(is_bit7_set(global_cmd_id)){
            memset(glass_info_param, 0, sizeof(glass_info_param));
            glass_info_param_len = ai_glass_info_pack_data(result, glass_info_param, NULL, 0);
            set_glass_info_v4(APP_CMD_RECORD, (char *)glass_info_param, glass_info_param_len);
        }else{
            memset(glass_info_param, 0, sizeof(glass_info_param));
            glass_info_param_len = ai_glass_info_pack_data(result, glass_info_param, NULL, 0);
            set_glass_info_v4(BT_CMD_RECORD, (char *)glass_info_param, glass_info_param_len);
        }
    }else{
        if(sd_nand_is_full(SD_NAND_MOUNT_PATH,IS_VIDEO_FULL_PERSENT))
    {
        // Not enough space, disable photo taking
        result = RETURN_CODE_SD_NAND_SPACE_FULL;
        if(is_bit7_set(global_cmd_id)){
            memset(glass_info_param, 0, sizeof(glass_info_param));
            glass_info_param_len = ai_glass_info_pack_data(result, glass_info_param, NULL, 0);
            set_glass_info_v4(APP_CMD_RECORD, (char *)glass_info_param, glass_info_param_len);
        }else{
            memset(glass_info_param, 0, sizeof(glass_info_param));
            glass_info_param_len = ai_glass_info_pack_data(result, glass_info_param, NULL, 0);
            set_glass_info_v4(BT_CMD_RECORD, (char *)glass_info_param, glass_info_param_len);
        }
    }else{
        for (AK_U32 u32devid = 0; u32devid < g_s32DevCnt; u32devid++)
        {
            get_vi_dev_info(u32devid, &qs_dev.dev_info[u32devid]);
            get_venc_attr(u32devid, &qs_dev.venc_chn[u32devid]);
        }
        unsigned int i_fps = pdev->dev_info[VIDEO_DEV0].frame_rate;
        unsigned int i_frame_depth = pdev->dev_info[VIDEO_DEV0].main_chn_buf;
        log_info_app("record i_fps:%u i_frame_depth:%u\n", i_fps, i_frame_depth);

        strcpy(record_params.isp_conf_path, gm_config.video_isp_config);
        //int *p_parameter = (int*)glass_info->parameter;
        record_params.record_direction = 0;
        record_params.record_duration = record_time;
        record_params.record_min_duration = 3;
        record_params.frame_depth = i_frame_depth;
        log_info_app("isp_conf_path:%s record_direction:%u record_duration:%u record_min_duration:%u frame_depth:%d\n",
                record_params.isp_conf_path,
                record_params.record_direction,
                record_params.record_duration,
                record_params.record_min_duration,
                record_params.frame_depth);

        // Create recording thread
        int ret = pthread_create(&record_thread, NULL, record_thread_func, &record_params);
        if (ret == 0)
        {
            log_info_app("Record thread created successfully\n");
            set_led_flash("state_led", 500, 500);
        }else{
                log_error_app("Failed to create record thread: %d\n", ret);
                result = RETURN_CODE_CREATE_FAIL;
                if(is_bit7_set(global_cmd_id)){
                    memset(glass_info_param, 0, sizeof(glass_info_param));
                    glass_info_param_len = ai_glass_info_pack_data(result, glass_info_param, NULL, 0);
                    set_glass_info_v4(APP_CMD_RECORD, (char *)glass_info_param, glass_info_param_len);
                }else{
                    memset(glass_info_param, 0, sizeof(glass_info_param));
                    glass_info_param_len = ai_glass_info_pack_data(result, glass_info_param, NULL, 0);
                    set_glass_info_v4(BT_CMD_RECORD, (char *)glass_info_param, glass_info_param_len);
                }
            }
        }
    }

    return result;
}


int stop_video_record(void)
{
   int result = RETURN_CODE_SUCCESS;

   log_info_app("Worker thread: STOP_RECORD\n");
   if (record_thread_running){
       ak_record_stop();
       record_thread_running = 0;
       record_thread_exit = 1;
       pthread_join(record_thread, NULL);
       log_info_app("Record thread stopped\n");
   }else{
       log_info_app("Record thread is not running\n");
        result = RETURN_CODE_DEVICE_NO_USE;//fail
   }
   return result;
}
#ifdef SOUND_REC_SUPPORT_ENABLE

int start_audio_record(uint8_t record_duration, uint8_t record_min_duration)
{
    int result = RETURN_CODE_SUCCESS;

    record_thread_params_t record_params = {0};

    log_info_app("CMD_AUDIO_RECORD start audio record\n");
    // Start audio recording
    ak_sound_demo_init();

    record_params.record_duration = record_duration;
    record_params.record_min_duration = record_min_duration;
    log_info_app("[AUDIO]record_duration:%u record_min_duration:%u\n",
            record_params.record_duration,
            record_params.record_min_duration);

    // ak_sound_demo_start_record(AUDIO_PATH);
    int ret = pthread_create(&audio_record_thread, NULL, audio_record_thread_func, &record_params);
    if (ret == 0)
    {
        log_info_app("Audio record thread created successfully\n");
    }else{
        log_error_app("Failed to create Audio record thread: %d\n", ret);
        result = RETURN_CODE_FILE_CREATE_FAILED;
    }

    return result;

}
int stop_audio_record(void)
{
    int result = RETURN_CODE_SUCCESS;
    log_info_app("CMD_AUDIO_RECORD_STOP stop audio record\n");
    if (audio_record_thread_running)
    {
        // Set manual stop flag to let the recording thread detect and stop
        audio_record_thread_exit = 1;
        // Wait for recording thread to finish
        pthread_join(audio_record_thread, NULL);
        log_info_app("Audio Record thread stopped\n");
    }else{
        log_info_app("Audio record thread is not running\n");
        result = RETURN_CODE_DEVICE_NO_USE;
    }
    return result;
}

#endif


void handle_set_timezone(const quick_glass_info_t *glass_info)
{
    uint8_t result = 0;
    uint8_t glass_info_param[256] = {0};
    uint16_t glass_info_param_len = 0;
    ak_glass_timezone_t *tz = (ak_glass_timezone_t *)glass_info->parameter;

    log_info_app("handle_set_timezone:%d\n", tz->minutes);

    if (set_system_timezone(tz) == 0) {
        result = 0; // success
    }else{
        result = RETURN_CODE_PARAM_CONFIG_FAILED; // fail
    }
    memset(glass_info_param, 0, sizeof(glass_info_param));
    glass_info_param_len = ai_glass_info_pack_data(result, glass_info_param, NULL, 0);
    set_glass_info_v4(glass_info->cmd, (char *)glass_info_param, glass_info_param_len);
}

void handle_set_local_time(const quick_glass_info_t *glass_info)
{
    uint8_t result = RETURN_CODE_SUCCESS ;
    uint8_t glass_info_param[256] = {0};
    uint16_t glass_info_param_len = 0;
    ak_glass_localtime_t *local_time = (ak_glass_localtime_t *)glass_info->parameter;

    log_info_app("handle_set_local_time timezone.minutes:%d", local_time->timezone.minutes);
    log_info_app("handle_set_local_time datetime.year:%u", local_time->datetime.year);
    log_info_app("handle_set_local_time datetime.month:%u", local_time->datetime.month);
    log_info_app("handle_set_local_time datetime.day:%u", local_time->datetime.day);
    log_info_app("handle_set_local_time datetime.hour:%u", local_time->datetime.hour);
    log_info_app("handle_set_local_time datetime.min:%u", local_time->datetime.min);
    log_info_app("handle_set_local_time datetime.sec:%u", local_time->datetime.sec);

    ak_glass_timezone_t *tz  =  &local_time->timezone;

    if (set_system_time(&local_time->datetime) != 0){
        result = RETURN_CODE_PARAM_CONFIG_FAILED;// fail
    }

    if (set_system_timezone(tz) != 0) {
       result = RETURN_CODE_PARAM_CONFIG_FAILED; // fail
    }
    get_system_time();

    memset(glass_info_param, 0, sizeof(glass_info_param));
    glass_info_param_len = ai_glass_info_pack_data(result, glass_info_param, NULL, 0);
    set_glass_info_v4(glass_info->cmd, (char *)glass_info_param, glass_info_param_len);
}


void handle_set_time(const quick_glass_info_t *glass_info)
{
    uint8_t result = RETURN_CODE_SUCCESS ;
    uint8_t glass_info_param[256] = {0};
    uint16_t glass_info_param_len = 0;

    ak_glass_datetime_t *datetime = (ak_glass_datetime_t *)glass_info->parameter;

    log_info_app("handle_set_time: %u/%u/%u %u:%u:%u\n",
            datetime->year, datetime->month, datetime->day,
            datetime->hour, datetime->min, datetime->sec);

    if (set_system_time(datetime) != 0)
    {
        result = RETURN_CODE_PARAM_INVALID;
    }
    get_system_time();

    memset(glass_info_param, 0, sizeof(glass_info_param));
    glass_info_param_len = ai_glass_info_pack_data(result, glass_info_param, NULL, 0);
    set_glass_info_v4(glass_info->cmd, (char *)glass_info_param, glass_info_param_len);
}

void handle_get_storage_file_count(const quick_glass_info_t *glass_info)
{
    // File count statistics
    uint8_t result = RETURN_CODE_SUCCESS ;
    uint8_t glass_info_param[256] = {0};
    uint16_t glass_info_param_len = 0;

    file_count_t file_count_stu = {0};
    file_count_stu.file_count_type = glass_info->parameter[0];
    if (file_count_stu.file_count_type < SYNC_FILE_TYPE_ALL || file_count_stu.file_count_type > SYNC_FILE_TYPE_END){
        log_error_app("CMD_CHK_STORAGE_FILE_COUNT file_count_stu.file_count_type[%u] out if range[%d %d]\n",
                file_count_stu.file_count_type, SYNC_FILE_TYPE_ALL, SYNC_FILE_TYPE_END);
        file_count_stu.file_count = 0;
        return;
    }

    // Wait for SD NAND to be ready
    wait_for_sdnand(BT_CMD_NO_CMD);

    switch (file_count_stu.file_count_type)
    {
    case SYNC_FILE_TYPE_ALL:
        // Total statistics
        file_count_stu.file_count = sd_nand_get_dir_file_count(VIDEOS_PATH);
        file_count_stu.file_count += sd_nand_get_dir_file_count(PICTURES_PATH);
        file_count_stu.file_count += sd_nand_get_dir_file_count(AUDIO_PATH);
        break;
    case SYNC_FILE_TYPE_VIDEOS:
        // Video statistics
        file_count_stu.file_count = sd_nand_get_dir_file_count(VIDEOS_PATH);
        break;
    case SYNC_FILE_TYPE_PICTURES:
        // Image statistics
        file_count_stu.file_count = sd_nand_get_dir_file_count(PICTURES_PATH);
        break;
    case SYNC_FILE_TYPE_AUDIO:
        // Audio statistics
        file_count_stu.file_count = sd_nand_get_dir_file_count(AUDIO_PATH);
        break;
    default:
        log_error_app("unknow file_count_stu.file_count_type[%u]", file_count_stu.file_count_type);
        file_count_stu.file_count = 0;
        break;
    }

    memset(glass_info_param, 0, sizeof(glass_info_param));
    glass_info_param_len = ai_glass_info_pack_data(result, glass_info_param, (uint8_t*) &file_count_stu, sizeof(file_count_t));
    set_glass_info_v4(glass_info->cmd, (char *)glass_info_param, glass_info_param_len);
}

void handle_recovert_factory(const quick_glass_info_t *glass_info)
{
    uint8_t result = RETURN_CODE_SUCCESS ;
    uint8_t glass_info_param[256] = {0};
    uint16_t glass_info_param_len = 0;

    wait_for_sdnand(BT_CMD_NO_CMD);

    // SD NAND formatting
    result = sd_nand_format_to_fat32(SD_NAND_DEVICE_PATH);
    if (result == 0){
        sd_nand_file_path_init(SD_NAND_DEVICE_PATH);
        log_info_app("sd_nand_format_to_fat32 successfully\n");
    }else{
        result = RETURN_CODE_SD_NAND_FORMART_FAIL;
        log_error_app("sd_nand_format_to_fat32 failed\n");
        memset(glass_info_param, 0, sizeof(glass_info_param));
        glass_info_param_len = ai_glass_info_pack_data(result, glass_info_param, NULL, 0);
        set_glass_info_v4(glass_info->cmd, (char *)glass_info_param, glass_info_param_len);
        return;
    }

    // P2P configuration reset
    result = system("cp /etc/config/wpa_p2p_group_bak.conf /etc/config/wpa_p2p_group.conf");
    if (result == 0){
        log_info_app("RECOVERY_FACTORY successfully\n");
    }else{
        log_error_app("RECOVERY_FACTORY failed\n");
    }

    memset(glass_info_param, 0, sizeof(glass_info_param));
    glass_info_param_len = ai_glass_info_pack_data(result, glass_info_param, NULL, 0);
    set_glass_info_v4(glass_info->cmd, (char *)glass_info_param, glass_info_param_len);
}

void handle_get_wifi_ip(const quick_glass_info_t *glass_info)
{
    uint8_t result = RETURN_CODE_SUCCESS ;
    uint8_t glass_info_param[256] = {0};
    uint16_t glass_info_param_len = 0;

    log_info_app("handle_get_wifi_ip\n");
    // Return IP
    ak_glass_wifi_ip_t wifi_ip_stu = {0};

    wifi_get_ip(wifi_ip_stu.wifi_ip);

    memset(glass_info_param, 0, sizeof(glass_info_param));
    glass_info_param_len = ai_glass_info_pack_data(result, glass_info_param,(char *)&wifi_ip_stu, sizeof(ak_glass_wifi_ip_t));
    set_glass_info_v4(glass_info->cmd, (char *)glass_info_param, glass_info_param_len);
}

void handle_get_sta_wifi_status(const quick_glass_info_t *glass_info)
{
    uint8_t result = RETURN_CODE_SUCCESS ;
    uint8_t glass_info_param[256] = {0};
    uint16_t glass_info_param_len = 0;

    log_info_app("handle_get_sta_wifi_status\n");
    // Check connection status, return WiFi name and password
    ak_glass_wifi_status_t wifi_status_stu = {0};

    wifi_get_status_ssid_password(&wifi_status_stu.connnect_status, wifi_status_stu.wifi_ssid, wifi_status_stu.wifi_password);

    memset(glass_info_param, 0, sizeof(glass_info_param));
    glass_info_param_len = ai_glass_info_pack_data(result, glass_info_param,(char *)&wifi_status_stu,sizeof(ak_glass_wifi_status_t));
    set_glass_info_v4(glass_info->cmd, (char *)glass_info_param, glass_info_param_len);
}

void handle_wifi_stop(const quick_glass_info_t *glass_info)
{
    uint8_t result = RETURN_CODE_SUCCESS ;
    uint8_t glass_info_param[256] = {0};
    uint16_t glass_info_param_len = 0;

    log_info_app("Stop Wifi\n");
    result = stop_wifi();
    if(result != 0)
    {
        result = RETURN_CODE_DEVICE_BUSY;
    }
    stop_rtsp_live_stream();

    memset(glass_info_param, 0, sizeof(glass_info_param));
    glass_info_param_len = ai_glass_info_pack_data(result, glass_info_param, NULL, 0);
    set_glass_info_v4(glass_info->cmd, (char *)glass_info_param, glass_info_param_len);
}
void handle_wifi_sta_connect(const quick_glass_info_t *glass_info)
{
    uint8_t result = RETURN_CODE_SUCCESS ;
    uint8_t glass_info_param[256] = {0};
    uint16_t glass_info_param_len = 0;

    ak_glass_wifi_ap_t *wifi_sta = (ak_glass_wifi_ap_t *)glass_info->parameter;

    log_info_app("CMD_WIFI_STA_CONNECT STA ssid:%s\n", wifi_sta->ap_ssid);
    if (set_wifi_sta_conf(wifi_sta->ap_ssid, wifi_sta->ap_password) != 0) {
        result = RETURN_CODE_PARAM_INVALID; // rejected credentials: do not start with the stored config
    } else {
        result = start_sta();
#ifdef RTSP_SUPPORT_ENABLE
        if(result == 0){
            start_rtsp_live_stream();
        }else{
            result = RETURN_CODE_DEVICE_BUSY;
            log_error_app("result(%d) error! rtsp no run\n",result);
        }
#endif
    }

    memset(glass_info_param, 0, sizeof(glass_info_param));
    glass_info_param_len = ai_glass_info_pack_data(result, glass_info_param, NULL, 0);
    set_glass_info_v4(glass_info->cmd, (char *)glass_info_param, glass_info_param_len);
}

void handle_start_rtsp(const quick_glass_info_t *glass_info)
{
    uint8_t result = RETURN_CODE_SUCCESS ;
    uint8_t glass_info_param[256] = {0};
    uint16_t glass_info_param_len = 0;

    log_info_app("handle_start_rtsp\n");
    result = start_rtsp_live_stream();

    memset(glass_info_param, 0, sizeof(glass_info_param));
    glass_info_param_len = ai_glass_info_pack_data(result, glass_info_param, NULL, 0);
    set_glass_info_v4(glass_info->cmd, (char *)glass_info_param, glass_info_param_len);
}


void handle_stop_rtsp(const quick_glass_info_t *glass_info)
{
    uint8_t result = RETURN_CODE_SUCCESS ;
    uint8_t glass_info_param[256] = {0};
    uint16_t glass_info_param_len = 0;

    log_info_app("handle_stop_rtsp\n");
    set_work_mode(WORK_MODE_IDLE);

    result = stop_rtsp_live_stream();

    memset(glass_info_param, 0, sizeof(glass_info_param));
    glass_info_param_len = ai_glass_info_pack_data(result, glass_info_param, NULL, 0);
    set_glass_info_v4(glass_info->cmd, (char *)glass_info_param, glass_info_param_len);
}

void handle_stop_video_record(const quick_glass_info_t *glass_info)
{
    uint8_t result = RETURN_CODE_SUCCESS ;
    uint8_t glass_info_param[256] = {0};
    uint16_t glass_info_param_len = 0;

    log_info_app("handle_stop_video_record\n");
    stop_storage_monitor();
    set_work_mode(WORK_MODE_IDLE);
    set_led_brightness("state_led", 0);
    result = stop_video_record();

    memset(glass_info_param, 0, sizeof(glass_info_param));
    glass_info_param_len = ai_glass_info_pack_data(result, glass_info_param, NULL, 0);
    set_glass_info_v4(glass_info->cmd, (char *)glass_info_param, glass_info_param_len);

}

/* Debug Wi-Fi AP of the image-debug and UVC photo modes, from app_main_config.ini [DebugAp] */
static uint8_t start_debug_ap(void)
{
    if (g_config.common_param.debug_ap_password[0] == '\0')
    {
        log_error_app("debug AP disabled: set [DebugAp] password (8-63 chars) in /etc/config/app_main_config.ini\n");
        return RETURN_CODE_PARAM_CONFIG_FAILED;
    }
    if (set_wifi_ap_conf(g_config.common_param.debug_ap_ssid, g_config.common_param.debug_ap_password) != 0)
        return RETURN_CODE_PARAM_CONFIG_FAILED;
    return start_ap();
}

void handle_start_video_record(const quick_glass_info_t *glass_info)
{
    uint8_t result = RETURN_CODE_SUCCESS ;
    uint8_t glass_info_param[256] = {0};
    uint16_t glass_info_param_len = 0;

    int rec_sec_max = REC_SEC_MAX;
    if(get_work_mode() != WORK_MODE_IDLE)
    {
        result = RETURN_CODE_DEVICE_BUSY;

        memset(glass_info_param, 0, sizeof(glass_info_param));
        glass_info_param_len = ai_glass_info_pack_data(result, glass_info_param, NULL, 0);
        set_glass_info_v4(glass_info->cmd, (char *)glass_info_param, glass_info_param_len);
        return;
    }
    global_cmd_id = glass_info->cmd;

    if(gm_config.record_param.image_debug_mode)
    {
        // Start AP; ssid and password from app_main_config.ini [DebugAp], no built-in password
        sleep(1);
        ai_glass_log_info("Start AP\n");
        result = start_debug_ap();

            // Start RTSP
        #ifdef RTSP_SUPPORT_ENABLE
            if(result == 0){
                start_rtsp_live_stream();
            }else{
                ai_glass_log_error("result(%d) error! rtsp no run\n",result);
            }
        #endif
    }
    else
    {
        int* p_parameter = (int*)glass_info->parameter;
        if(*p_parameter > 0)
        {
            rec_sec_max = *p_parameter;
        }

    // Time synchronization
    #if 1
        // Treat parameter as byte array, get time info starting from 4th byte
        unsigned char* param_bytes = (unsigned char*)glass_info->parameter;
        ak_glass_localtime_t* local_time = (ak_glass_localtime_t*)&param_bytes[4];

        log_info_app("handle_set_local_time timezone.minutes:%d\n", local_time->timezone.minutes);
        log_info_app("handle_set_local_time datetime.year:%u\n", local_time->datetime.year);
        log_info_app("handle_set_local_time datetime.month:%u\n", local_time->datetime.month);
        log_info_app("handle_set_local_time datetime.day:%u\n", local_time->datetime.day);
        log_info_app("handle_set_local_time datetime.hour:%u\n", local_time->datetime.hour);
        log_info_app("handle_set_local_time datetime.min:%u\n", local_time->datetime.min);
        log_info_app("handle_set_local_time datetime.sec:%u\n", local_time->datetime.sec);

        ak_glass_timezone_t *tz = &local_time->timezone;

        if (set_system_time(&local_time->datetime) != 0){
            //result = RETURN_CODE_PARAM_CONFIG_FAILED;// fail
        }

        if (set_system_timezone(tz) != 0) {
            //result = RETURN_CODE_PARAM_CONFIG_FAILED; // fail
        }
        get_system_time();
    #endif

        start_video_record(rec_sec_max);
        start_storage_monitor(SD_NAND_MOUNT_PATH);
        log_info_app("handle_start_video_record, %d\n", rec_sec_max);
    }
}


void handle_take_photo(const quick_glass_info_t *glass_info)
{
    uint8_t result = RETURN_CODE_SUCCESS ;
    uint8_t glass_info_param[256] = {0};
    uint16_t glass_info_param_len = 0;

    log_info_app("handle_take_photo\n");
    if(get_work_mode() != WORK_MODE_IDLE)
    {
        result = RETURN_CODE_DEVICE_BUSY;

        memset(glass_info_param, 0, sizeof(glass_info_param));
        glass_info_param_len = ai_glass_info_pack_data(result, glass_info_param, NULL, 0);
        set_glass_info_v4(glass_info->cmd, (char *)glass_info_param, glass_info_param_len);
        return;
    }
    global_cmd_id = glass_info->cmd;
    if(sd_nand_is_full(SD_NAND_MOUNT_PATH,IS_PHOTO_FULL_PERSENT))
    {
        // Not enough space, disable photo taking
        result = RETURN_CODE_SD_NAND_SPACE_FULL;
    }else{
        // Time synchronization
#if 1
        ak_glass_localtime_t *local_time = (ak_glass_localtime_t *)glass_info->parameter;

        log_info_app("handle_set_local_time timezone.minutes:%d", local_time->timezone.minutes);
        log_info_app("handle_set_local_time datetime.year:%u", local_time->datetime.year);
        log_info_app("handle_set_local_time datetime.month:%u", local_time->datetime.month);
        log_info_app("handle_set_local_time datetime.day:%u", local_time->datetime.day);
        log_info_app("handle_set_local_time datetime.hour:%u", local_time->datetime.hour);
        log_info_app("handle_set_local_time datetime.min:%u", local_time->datetime.min);
        log_info_app("handle_set_local_time datetime.sec:%u", local_time->datetime.sec);

        ak_glass_timezone_t *tz  =  &local_time->timezone;

        if (set_system_time(&local_time->datetime) != 0){
            //result = RETURN_CODE_PARAM_CONFIG_FAILED;// fail
        }

        if (set_system_timezone(tz) != 0) {
            //result = RETURN_CODE_PARAM_CONFIG_FAILED; // fail
        }
        get_system_time();
#endif

        set_led_brightness("state_led", 1);
        set_work_mode(WORK_MODE_PHOTO);
        result = start_take_photo();
        set_led_brightness("state_led", 0);
    }
    if(result != 0)// Only send photo failure reply, because success reply has been sent early in the capture function
    {
		memset(glass_info_param, 0, sizeof(glass_info_param));
        glass_info_param_len = ai_glass_info_pack_data(result, glass_info_param, NULL, 0);
        set_glass_info_v4(glass_info->cmd, (char *)glass_info_param, glass_info_param_len);
    }

}

void handle_power_off(const quick_glass_info_t *glass_info)
{
    uint8_t result = RETURN_CODE_SUCCESS ;
    uint8_t glass_info_param[256] = {0};
    uint16_t glass_info_param_len = 0;

    if(is_bit7_set(glass_info->cmd)){
        log_info_app("APP Wants to power off\n");
    }else{
        log_info_app("BT Wants to power off\n");
    }
    set_work_mode(WORK_MODE_IDLE);
    save_fasylog_to_file();
    save_dmesg_to_file();
    save_app_log_to_file();

    // umount SD nand
    result = sd_nand_umount("/mnt");
    if(result == 0)
    {
        result = RETURN_CODE_SUCCESS;
    }else{
        result = RETURN_CODE_UMOUNT_FAILED;
    }


    uint16_t power_off_time = g_config.common_param.power_off_time;

    memset(glass_info_param, 0, sizeof(glass_info_param));
    glass_info_param_len = ai_glass_info_pack_data(result, glass_info_param, (char*)&power_off_time,sizeof(uint16_t));
    set_glass_info_v4(glass_info->cmd, (char *)glass_info_param, glass_info_param_len);
}

void handle_wifi_ap_mode(const quick_glass_info_t *glass_info)
{
    uint8_t result = RETURN_CODE_SUCCESS ;
    uint8_t glass_info_param[256] = {0};
    uint16_t glass_info_param_len = 0;

    log_info_app("handle_wifi_ap_mode\n");
    ak_glass_wifi_ap_t *wifi_ap = (ak_glass_wifi_ap_t *)glass_info->parameter;
    log_info_app("AP ssid:%s\n", wifi_ap->ap_ssid);
    if (set_wifi_ap_conf(wifi_ap->ap_ssid, wifi_ap->ap_password) != 0) {
        result = RETURN_CODE_PARAM_INVALID; // rejected credentials: do not start with the stored config
    } else {
        result = start_ap();
#ifdef RTSP_SUPPORT_ENABLE
        if(result == 0){
            start_rtsp_live_stream();
        }else{
            result = RETURN_CODE_DEVICE_BUSY;
            log_error_app("result(%d) error! rtsp no run\n",result);
        }
#endif
    }

    memset(glass_info_param, 0, sizeof(glass_info_param));
    glass_info_param_len = ai_glass_info_pack_data(result, glass_info_param, NULL, 0);
    set_glass_info_v4(glass_info->cmd, (char *)glass_info_param, glass_info_param_len);
}

void handle_wifi_p2p_mode(const quick_glass_info_t *glass_info)
{
    uint8_t result = RETURN_CODE_SUCCESS ;
    uint8_t glass_info_param[256] = {0};
    uint16_t glass_info_param_len = 0;

    char p2p_name[64] = {0};
	int sub_cmd = 2; //P2P function: 0x00 bind device, 0x01 photo library (media transfer), 0x02 live video. Default is live video (capture encoding + RTSP)

    set_led_flash("state_led", 1000, 1000);

    if(glass_info->param_len > 0)
    {
		log_info_app("p2p cmd paramlen=%d\n", glass_info->param_len);
		if(glass_info->param_len == 24) //Old protocol without state
        {
			strncpy(p2p_name,glass_info->parameter,glass_info->param_len);
			log_info_app("p2p old cmd state=%d, p2p_name=%s\n", sub_cmd, p2p_name);
		}else {//New protocol
			ak_glass_p2p_config_t *p2p_param = (ak_glass_p2p_config_t *)glass_info->parameter;
			strncpy(p2p_name, p2p_param->p2p_dev_name, glass_info->param_len-1);// Remove one byte state data
			sub_cmd = p2p_param->state;
			log_info_app("p2p new cmd state=%d, p2p_name=%s\n", sub_cmd, p2p_name);
		}

    }else{
        log_error_app("param_len error! len:%d\n",glass_info->param_len);
        result = RETURN_CODE_PARAM_CONFIG_FAILED;
    }

    if(sub_cmd == 1)
    {
        if(handle_get_num_img_video() == 0)
        {
        printf("mode import\n");
        set_work_mode(WORK_MODE_RTSP);
        result = start_p2p(p2p_name);
        }
    }else if (sub_cmd == 2){
            printf("mode rtsp\n");
            set_work_mode(WORK_MODE_RTSP);
            result = start_p2p(p2p_name);
            start_rtsp_live_stream();
            memset(glass_info_param, 0, sizeof(glass_info_param));
            glass_info_param_len = ai_glass_info_pack_data(result, glass_info_param, NULL, 0);
            set_glass_info_v4(glass_info->cmd, (char *)glass_info_param, glass_info_param_len);
    }else{
            printf("mode p2p\n");
            set_work_mode(WORK_MODE_P2P);
            result = start_p2p(p2p_name);
            memset(glass_info_param, 0, sizeof(glass_info_param));
            glass_info_param_len = ai_glass_info_pack_data(result, glass_info_param, NULL, 0);
            set_glass_info_v4(glass_info->cmd, (char *)glass_info_param, glass_info_param_len);
    }
}
#ifdef SOUND_REC_SUPPORT_ENABLE

void handle_start_audio_record(const quick_glass_info_t *glass_info)
{
    uint8_t result = RETURN_CODE_SUCCESS ;
    uint8_t glass_info_param[256] = {0};
    uint16_t glass_info_param_len = 0;

    uint8_t record_duration,record_min_duration;

    log_info_app("handle_start_audio_record\n");
    int *p_parameter = (int*)glass_info->parameter;

    record_duration = p_parameter[0];
    record_min_duration = p_parameter[1];
    set_work_mode(WORK_MODE_AUDIO);
    result = start_audio_record(record_duration,record_min_duration);

    memset(glass_info_param, 0, sizeof(glass_info_param));
    glass_info_param_len = ai_glass_info_pack_data(result, glass_info_param, NULL, 0);
    set_glass_info_v4(glass_info->cmd, (char *)glass_info_param, glass_info_param_len);
}
void handle_stop_audio_record(const quick_glass_info_t *glass_info)
{
    uint8_t result = RETURN_CODE_SUCCESS ;
    uint8_t glass_info_param[256] = {0};
    uint16_t glass_info_param_len = 0;

    set_work_mode(WORK_MODE_IDLE);
    result = stop_audio_record();

    memset(glass_info_param, 0, sizeof(glass_info_param));
    glass_info_param_len = ai_glass_info_pack_data(result, glass_info_param, NULL, 0);
    set_glass_info_v4(glass_info->cmd, (char *)glass_info_param, glass_info_param_len);
}
#endif


/*
 * OTA images come from the phone (IPI) or the SD card, so the path is untrusted:
 * it must name a file under /mnt with plain characters, and ota_flasher runs
 * without a shell.
 */
static int ota_path_ok(const char *path)
{
    static const char allowed[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789._-/";

    if (strncmp(path, SD_NAND_MOUNT_PATH "/", sizeof(SD_NAND_MOUNT_PATH)) != 0 ||
        strstr(path, "..") != NULL || path[strspn(path, allowed)] != '\0')
    {
        log_error_app("OTA file path rejected: it must be under %s/ and use only [A-Za-z0-9._-]\n", SD_NAND_MOUNT_PATH);
        return 0;
    }
    return 1;
}

/* Not waited for: this app exits while ota_flasher runs, so detach it into its own session */
static void start_ota_flasher(const char *path)
{
    pid_t pid = fork();

    if (pid < 0)
    {
        log_error_app("OTA: fork failed: %s\n", strerror(errno));
    }
    else if (pid == 0)
    {
        setsid();
        execlp("ota_flasher", "ota_flasher", "-f", path,
               "-p", "ENV,ENVBAK,ISPCONF,FASTSYS,DTB,KERNEL,INITFS,RAMFS,RAMFS2,CONFIG,APP,ARGS", "-y", (char *)NULL);
        _exit(127);
    }
}

void handle_ota(const quick_glass_info_t *glass_info)
{
    uint8_t result = RETURN_CODE_SUCCESS ;
    uint8_t glass_info_param[256] = {0};
    uint16_t glass_info_param_len = 0;

    ak_ota_info_t *ota_info = (ak_ota_info_t *)glass_info->parameter;
    char ota_file_path[sizeof(ota_info->ota_file_path) + 1];

    // The IPI field is not guaranteed to be NUL-terminated
    memcpy(ota_file_path, ota_info->ota_file_path, sizeof(ota_info->ota_file_path));
    ota_file_path[sizeof(ota_info->ota_file_path)] = '\0';

    log_info_app("APP OTA KM02\n");

    // Check if the firmware package exists
    if(!ota_path_ok(ota_file_path))
    {
        result = RETURN_CODE_PARAM_INVALID;
    }
    else if(sd_nand_file_exists(ota_file_path))
    {
        log_info_app("file[%s] exist! start ota\n",ota_file_path);
		log_info_app("close glass_fd\n");
		ai_glass_ipi_cmd_fd_close();

        set_work_mode(WORK_MODE_OTA);
        set_led_flash("state_led", 300, 300);
        //Initiate full firmware flashing
        start_ota_flasher(ota_file_path);

        //Program quit unexpectedly
        ota_run_flag = 1;
        ai_glass_demo_exit();
    }
    else
    {
        //fail
        log_error_app("ota file No exists!\n");
        result = RETURN_CODE_FILE_READ_FAILED;
    }

    memset(glass_info_param, 0, sizeof(glass_info_param));
    glass_info_param_len = ai_glass_info_pack_data(result, glass_info_param, NULL, 0);
    set_glass_info_v4(glass_info->cmd, (char *)glass_info_param, glass_info_param_len);
}

void ota_update(const char *file_path)
{
    log_info_app("U DISK ota_update\n");

    // Check if the firmware package exists
    if(ota_path_ok(file_path) && sd_nand_file_exists(file_path))
    {
        log_debug_app("file[%s] exist! start ota\n",file_path);
        log_info_app("close glass_fd\n");
		ai_glass_ipi_cmd_fd_close();
        set_work_mode(WORK_MODE_OTA);
        set_led_flash("state_led", 300, 300);
        //Initiate full firmware flashing
        start_ota_flasher(file_path);

        //Program quit unexpectedly
        ota_run_flag = 1;
        ai_glass_demo_exit();

    }else{
        //fail
        log_error_app("ota file No exists!\n");
    }

}

static void *wait_and_ota_update(void *arg)
{
    int result = 0;
    printf("enter wait_and_ota_update\n");
    result = get_usb_remove();
    if(result == 1)
    {
        wait_for_sdnand(BT_CMD_NO_CMD);
        system("sync");
        system("echo 3 > /proc/sys/vm/drop_caches");
        printf("waiting wait_and_ota_update\n");
        check_camera_ini();
        check_app_ini();

        while(1){
            if (access(OTA_PATH_NAME, F_OK) == 0) {
                ota_update(OTA_PATH_NAME);
                break;
            }
            sleep(5);
        }

    }
    return NULL;
}


void u_pan_auto_update(void)
{
    int ret = 0;
    pthread_t ota_bin_thread;

    ret = pthread_create(&ota_bin_thread, NULL, wait_and_ota_update, NULL);
     if (ret != 0) {
         log_error_app("u_pan_auto_update fail\n");
     }
}

void handle_get_work_mode(const quick_glass_info_t *glass_info)
{
    uint8_t result = RETURN_CODE_SUCCESS ;
    uint8_t glass_info_param[256] = {0};
    uint16_t glass_info_param_len = 0;

    result = get_work_mode();

    memset(glass_info_param, 0, sizeof(glass_info_param));
    glass_info_param_len = ai_glass_info_pack_data(result, glass_info_param, NULL, 0);
    set_glass_info_v4(glass_info->cmd, (char *)glass_info_param, glass_info_param_len);
}

int handle_usb_mode_set(const quick_glass_info_t *glass_info)
{
	//int result = 0;
    uint8_t result = RETURN_CODE_SUCCESS ;
    uint8_t glass_info_param[256] = {0};
    uint16_t glass_info_param_len = 0;

    uint8_t mode = *(char*)glass_info->parameter;
    log_info_app("handle_usb_mode_set glass_info.param_len=%d req mode=%d\n", glass_info->param_len, mode);
	if(usbdetect_fun() == USB_MODE_CHARGE)
	{
		mode = 0; // Modify to charging mode;
	}
	log_info_app("handle_usb_mode_set aws mode=%d\n", mode);


    memset(glass_info_param, 0, sizeof(glass_info_param));
    glass_info_param_len = ai_glass_info_pack_data(result, glass_info_param, (uint8_t *)&mode,sizeof(uint8_t));
    set_glass_info_v4(glass_info->cmd, (char *)glass_info_param, glass_info_param_len);

	if(mode == 0)
	{
        log_info_app("handle_usb_mode_set charge mode\n");
		return 0;
	}


	if(mode == 1)
	{

        log_info_app("handle_usb_mode_set usb storge mode\n");
        sd_nand_file_path_init(SD_NAND_DEVICE_PATH);
		//u_pan_auto_update();//for test
	}
	else if(mode == 2) // uvc debug photo
	{

        log_info_app("handle_usb_mode_set uvc photo mode\n");
		sleep(2);// Wait for file system to start
		//system("cat /dev/fastsys-log");

		wait_for_script();
		// From app_main_config.ini [DebugAp]; the AP stays off if no password is set
		start_debug_ap();

		system("/usr/sbin/usb_udc_g.sh uninstall");
		system("/usr/sbin/uvc.sh");
		sleep(2);

		log_info_app("ak_snap_env_init start!\n");
		int ret = ak_snap_env_init(gm_config.snap_isp_config, 10, 1);
	    if(ret != 0)
	    {
	        log_error_app("ak_snap_env_init failed! ret:%d\n", ret);
	        return ret;
	    }
		log_info_app("ak_snap_env_init ok!\n");
	    ret = ak_snap_env_enable(1);
	    if(ret != 0)
	    {
	        log_error_app("USB webcam start failed! ret:%d\n", ret);
	        return ret;
	    }

	}
	else if(mode == 3) // uvc debug recording
	{
		log_info_app("handle_usb_mode_set uvc record mode\n");
	}

	return 0;
}

int handle_get_storage(const quick_glass_info_t *glass_info)
{
    uint8_t result = RETURN_CODE_SUCCESS ;
    uint8_t glass_info_param[256] = {0};
    uint16_t glass_info_param_len = 0;

    StorageInfo storage_info;

    log_info_app("handle_get_storage\n");
    if (sd_nand_get_storage_info("/mnt", &storage_info) == 0) {
        log_info_app("Used: %u MB, Left: %u MB\n", storage_info.used, storage_info.left);
    }

    memset(glass_info_param, 0, sizeof(glass_info_param));
    glass_info_param_len = ai_glass_info_pack_data(result, glass_info_param, (uint8_t *)&storage_info, sizeof(StorageInfo));
    set_glass_info_v4(glass_info->cmd, (char *)glass_info_param, glass_info_param_len);
}

int handle_cmd_init()
{



}


int handle_cmd_deinit()
{
#ifdef RTSP_SUPPORT_ENABLE
    // Stop RTSP and recording threads
    if (rtsp_thread_running)
    {
        rtsp_thread_exit = 1;
        pthread_join(rtsp_thread, NULL);
        log_info_app("RTSP thread joined\n");
    }
#endif

#ifdef RECORD_SUPPORT_ENABLE
	    if (record_thread_running)
	    {
	        record_thread_exit = 1;
	        pthread_join(record_thread, NULL);
	        log_info_app("Record thread joined\n");
	    }
#endif

}
