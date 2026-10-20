/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "fun_log.h"
#include "ini_parser.h"
#include "media_configs.h"

// Global config instance
GlobalConfig_User g_config;
GLASS_GlobalConfig_User gm_config;

/* [DebugAp]: copy only values that fit, so a long passphrase is rejected instead of cut short */
static void load_debug_ap(INI_CONFIG *config)
{
    const char *ssid = ini_get_string(config, "DebugAp", "ssid", "AI_GLASS_DEBUG");
    const char *pass = ini_get_string(config, "DebugAp", "password", "");

    if (strlen(ssid) < 1 || strlen(ssid) >= sizeof(g_config.common_param.debug_ap_ssid))
    {
        log_error_app("app_main_config.ini [DebugAp] ssid must be 1-32 characters; using AI_GLASS_DEBUG\n");
        ssid = "AI_GLASS_DEBUG";
    }
    strcpy(g_config.common_param.debug_ap_ssid, ssid);

    if (strlen(pass) >= sizeof(g_config.common_param.debug_ap_password))
    {
        log_error_app("app_main_config.ini [DebugAp] password is longer than 63 characters; debug AP disabled\n");
        pass = "";
    }
    strcpy(g_config.common_param.debug_ap_password, pass);
}

// Load config parameters from INI file
void load_configuration(void)
{
    INI_CONFIG *config = ini_create();
    if (config == NULL) {
        log_error_app("Failed to create INI config\n");
        return;
    }

    // Load config file
    if (ini_load(config, "/etc/config/app_main_config.ini") != 0) {
        log_error_app("Failed to load config.ini, using default values\n");
        ini_destroy(config);
        return;
    }

    // Read global config and save to global struct
    g_config.common_param.dmesg = ini_get_int(config, "Common", "dmesg", 0);
    g_config.common_param.middle_log = ini_get_int(config, "Common", "middle_log", 0);
    g_config.common_param.rtt_log = ini_get_int(config, "Common", "rtt_log", 0);
    g_config.common_param.app_log = ini_get_int(config, "Common", "app_log", 0);
    g_config.common_param.app_log_save = ini_get_int(config, "Common", "app_log_save", 0);
    g_config.common_param.power_off_time = ini_get_int(config, "Common", "power_off_time", 0);
    load_debug_ap(config);

    printf("app_main_config:dmesg=%d,middle_log=%d,rtt_log=%d,app_log=%d,app_log_save=%d,power_off_time=%d\n", g_config.common_param.dmesg, g_config.common_param.middle_log, g_config.common_param.rtt_log, g_config.common_param.app_log,g_config.common_param.app_log_save,g_config.common_param.power_off_time);

    // Free config
    ini_destroy(config);
}

void load_camera_configuration(void)
{
    INI_CONFIG *config = ini_create();
    if (config == NULL) {
        log_error_app("Failed to create INI config\n");
        return;
    }

    // Load config file
    if (ini_load(config, "/etc/config/camera.ini") != 0) {
        log_error_app("Failed to load config.ini, using default values\n");
        ini_destroy(config);
        return;
    }

    // Read global config and save to global struct
    strncpy(gm_config.snap_isp_config, ini_get_string(config, "Camera", "snap_isp_config", "/fast/isp_gc13b0_mipi_2lane_av200.conf"), MAX_VALUE_LEN - 1);
    gm_config.snap_isp_config[MAX_VALUE_LEN - 1] = '\0';
    strncpy(gm_config.video_isp_config, ini_get_string(config, "Camera", "video_isp_config", "/fast/isp_gc13b0_mipi_2lane_av200_30fps.conf"), MAX_VALUE_LEN - 1);
    gm_config.video_isp_config[MAX_VALUE_LEN - 1] = '\0';
    //
    gm_config.record_param.main_chn_width = ini_get_int(config, "Record", "main_chn_width", 2080);
    gm_config.record_param.main_chn_height = ini_get_int(config, "Record", "main_chn_height", 1536);
    gm_config.record_param.main_vi_width = ini_get_int(config, "Record", "main_vi_width", 2080);
    gm_config.record_param.main_vi_height = ini_get_int(config, "Record", "main_vi_height", 1536);
    gm_config.record_param.main_chn_fps = ini_get_int(config, "Record", "main_chn_fps", 30);
    gm_config.record_param.main_chn_target_kbps = ini_get_int(config, "Record", "main_chn_target_kbps", 20480);
    gm_config.record_param.main_chn_max_kbps = ini_get_int(config, "Record", "main_chn_max_kbps", 20480);
    gm_config.record_param.main_chn_gop_len = ini_get_int(config, "Record", "main_chn_gop_len", 90);
    gm_config.record_param.main_chn_enc_type = ini_get_int(config, "Record", "main_chn_enc_type", 0);
    gm_config.record_param.main_chn_br_mode = ini_get_int(config, "Record", "main_chn_br_mode", 5);
	gm_config.record_param.eis_enable = ini_get_int(config, "Record", "eis_enable", 0);
    // Recording RTSP+AP debug
    gm_config.record_param.image_debug_mode = ini_get_int(config, "Record", "image_debug_mode", 0);

    //
    gm_config.rtsp_param.main_chn_width = ini_get_int(config, "Rtsp", "main_chn_width", 1600);
    gm_config.rtsp_param.main_chn_height = ini_get_int(config, "Rtsp", "main_chn_height", 1200);
    gm_config.rtsp_param.main_vi_width = ini_get_int(config, "Rtsp", "main_vi_width", 2080);
    gm_config.rtsp_param.main_vi_height = ini_get_int(config, "Rtsp", "main_vi_height", 1536);
    gm_config.rtsp_param.main_chn_fps = ini_get_int(config, "Rtsp", "main_chn_fps", 30);
    gm_config.rtsp_param.main_chn_target_kbps = ini_get_int(config, "Rtsp", "main_chn_target_kbps", 2048);
    gm_config.rtsp_param.main_chn_max_kbps = ini_get_int(config, "Rtsp", "main_chn_max_kbps", 3072);
    gm_config.rtsp_param.main_chn_gop_len = ini_get_int(config, "Rtsp", "main_chn_gop_len", 60);
    gm_config.rtsp_param.main_chn_enc_type = ini_get_int(config, "Rtsp", "main_chn_enc_type", 0);
    gm_config.rtsp_param.main_chn_br_mode = ini_get_int(config, "Rtsp", "main_chn_br_mode", 5);
	gm_config.rtsp_param.eis_enable = ini_get_int(config, "Rtsp", "eis_enable", 1);

    //
    gm_config.photo_param.main_chn_fps = ini_get_int(config, "Photo", "main_chn_fps",10);
    gm_config.photo_param.main_org_width = ini_get_int(config, "Photo", "main_org_width",3840);
    gm_config.photo_param.main_org_height = ini_get_int(config, "Photo", "main_org_height",2880);
    gm_config.photo_param.main_res_width = ini_get_int(config, "Photo", "main_res_width",4032);
    gm_config.photo_param.main_res_height = ini_get_int(config, "Photo", "main_res_height",3024);
    gm_config.photo_param.main_jpg_slice_num = ini_get_int(config, "Photo", "main_jpg_slice_num",10);
    gm_config.photo_param.main_org_slice_one_height = ini_get_int(config, "Photo", "main_org_slice_one_height",960);
    gm_config.photo_param.main_jpg_slice_one_height = ini_get_int(config, "Photo", "main_jpg_slice_one_height",1008);

    if(g_config.common_param.app_log == 3)
    {
        printf("=== Camera Config ===\n");
        printf("snap_isp_config: %s\n", gm_config.snap_isp_config);
        printf("video_isp_config: %s\n", gm_config.video_isp_config);

        printf("=== Camera record ===\n");
        printf("main_chn_width: %d\n", gm_config.record_param.main_chn_width);
        printf("main_chn_height: %d\n", gm_config.record_param.main_chn_height);
        printf("main_vi_width: %d\n", gm_config.record_param.main_vi_width);
        printf("main_vi_height: %d\n", gm_config.record_param.main_vi_height);
        printf("main_chn_fps: %d\n", gm_config.record_param.main_chn_fps);
        printf("main_chn_target_kbps: %d\n", gm_config.record_param.main_chn_target_kbps);
        printf("main_chn_max_kbps: %d\n", gm_config.record_param.main_chn_max_kbps);
        printf("main_chn_gop_len: %d\n", gm_config.record_param.main_chn_gop_len);
        printf("main_chn_enc_type: %d\n", gm_config.record_param.main_chn_enc_type);
        printf("main_chn_br_mode: %d\n", gm_config.record_param.main_chn_br_mode);
        printf("eis_enable: %d\n", gm_config.record_param.eis_enable);
        printf("image_debug_mode: %d\n", gm_config.record_param.image_debug_mode);

        printf("=== Camera rtsp ===\n");
        printf("main_chn_width: %d\n", gm_config.rtsp_param.main_chn_width);
        printf("main_chn_height: %d\n", gm_config.rtsp_param.main_chn_height);
        printf("main_vi_width: %d\n", gm_config.rtsp_param.main_vi_width);
        printf("main_vi_height: %d\n", gm_config.rtsp_param.main_vi_height);
        printf("main_chn_fps: %d\n", gm_config.rtsp_param.main_chn_fps);
        printf("main_chn_target_kbps: %d\n", gm_config.rtsp_param.main_chn_target_kbps);
        printf("main_chn_max_kbps: %d\n", gm_config.rtsp_param.main_chn_max_kbps);
        printf("main_chn_gop_len: %d\n", gm_config.rtsp_param.main_chn_gop_len);
        printf("main_chn_enc_type: %d\n", gm_config.rtsp_param.main_chn_enc_type);
        printf("main_chn_br_mode: %d\n", gm_config.rtsp_param.main_chn_br_mode);
        printf("eis_enable: %d\n", gm_config.rtsp_param.eis_enable);

        printf("=== Camera photo ===\n");
        printf("main_chn_fps: %d\n", gm_config.photo_param.main_chn_fps);
        printf("main_org_width: %d\n", gm_config.photo_param.main_org_width);
        printf("main_org_height: %d\n", gm_config.photo_param.main_org_height);
        printf("main_res_width: %d\n", gm_config.photo_param.main_res_width);
        printf("main_res_height: %d\n", gm_config.photo_param.main_res_height);
        printf("main_jpg_slice_num: %d\n", gm_config.photo_param.main_jpg_slice_num);
        printf("main_org_slice_one_height: %d\n", gm_config.photo_param.main_org_slice_one_height);
        printf("main_jpg_slice_one_height: %d\n", gm_config.photo_param.main_jpg_slice_one_height);
    }

    // Free config
    ini_destroy(config);
}
int load_eis_configuration(void)
{
    INI_CONFIG *config = ini_create();
    if (config == NULL) {
        log_error_app("Failed to create INI config\n");
        return -1;
    }

    // Load config file
    if (ini_load(config, "/etc/config/eis_config.ini") != 0) {
        log_error_app("Failed to load config.ini, using default values\n");
        ini_destroy(config);
        return -1;
    }

    const char *eis_param = NULL;
    eis_param = ini_get_string(config, "main", "eis_param", "eis_param_80");
    // Read global config and save to global struct
    gm_config.eis_param.data[0][0] = ini_get_double(config, eis_param, "data0_0", 0);
    gm_config.eis_param.data[0][1] = ini_get_double(config, eis_param, "data0_1", 0);
    gm_config.eis_param.data[0][2] = ini_get_double(config, eis_param, "data0_2", 0);

    gm_config.eis_param.data[1][0] = ini_get_double(config, eis_param, "data1_0", 0);
    gm_config.eis_param.data[1][1] = ini_get_double(config, eis_param, "data1_1", 0);
    gm_config.eis_param.data[1][2] = ini_get_double(config, eis_param, "data1_2", 0);

    gm_config.eis_param.data[2][0] = ini_get_double(config, eis_param, "data2_0", 0);
    gm_config.eis_param.data[2][1] = ini_get_double(config, eis_param, "data2_1", 0);
    gm_config.eis_param.data[2][2] = ini_get_double(config, eis_param, "data2_2", 0);

    log_warn_app("=== eis param Config ===\n");
    log_warn_app("[0][0-2]: %0.16lf %.16lf %.16lf", gm_config.eis_param.data[0][0],gm_config.eis_param.data[0][1],gm_config.eis_param.data[0][2]);
    log_warn_app("[1][0-2]: %.16lf %.16lf %.16lf", gm_config.eis_param.data[1][0],gm_config.eis_param.data[1][1],gm_config.eis_param.data[1][2]);
    log_warn_app("[2][0-2]: %.16f %.16lf %.16lf", gm_config.eis_param.data[2][0],gm_config.eis_param.data[2][1],gm_config.eis_param.data[2][2]);

    // Free config
    ini_destroy(config);

    return 0;
}
