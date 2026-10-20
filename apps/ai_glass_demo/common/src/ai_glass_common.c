/* SPDX-License-Identifier: Apache-2.0 */
/**
 * @file ai_glass_common.c
 * @brief AI glasses common API: LED control, log save, config apply, etc.
 * @copyright Copyright (c) 2025 Guangzhou Anyka Microelectronics Co., Ltd.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include "fun_log.h"
#include "ai_glass_common.h"

/**
 * @brief Set LED brightness.
 * @param led_name LED device name (e.g. leds subdir under sysfs)
 * @param brightness Brightness value
 * @return 0 on success, -1 on failure
 */
int set_led_brightness(const char *led_name, int brightness)
{
    char path[128];
    FILE *fp;

    snprintf(path, sizeof(path), "/sys/devices/platform/leds/leds/%s/brightness", led_name);

    fp = fopen(path, "w");
    if (!fp) {
        perror("fopen failed");
        return -1;
    }

    fprintf(fp, "%d", brightness);
    fclose(fp);
    return 0;
}

/**
 * @brief Set LED flash (on/off timing).
 * @param led_name LED device name
 * @param on_time  On time (units match kernel implementation)
 * @param off_time Off time
 * @return 0 on success, -1 on failure
 */
int set_led_flash(const char *led_name, int on_time, int off_time)
{
    char path[256];
    FILE *fp;

    snprintf(path, sizeof(path), "/sys/devices/platform/leds/leds/%s/flash", led_name);

    fp = fopen(path, "w");
    if (!fp) {
        perror("fopen failed");
        return -1;
    }

    fprintf(fp, "on %d %d", on_time, off_time);
    fclose(fp);

    return 0;
}

/**
 * @brief Save fastsys RTT log to /mnt/Log (if enabled in config).
 * @return 0
 */
int save_fasylog_to_file()
{
    char cmd[256] = {0};
    DATE_S t;
    AK_BASE_GetLocalDate(&t);
    if(g_config.common_param.rtt_log != 0)
    {
        sprintf(cmd, "cat /dev/fastsys-log > /mnt/Log/fastsys_%4d%02d%02d_%02d%02d%02d.log",  t.s32Year,t.s32Month,t.s32Day,t.s32Hour,t.s32Minute,t.s32Second);
        system(cmd);
    }
   //printf("app_main_config:%d,%d,%d,%d %s\n", g_config.common_param.dmesg, g_config.common_param.middle_log, g_config.common_param.rtt_log, g_config.common_param.app_log, cmd);

    return 0;
}

/**
 * @brief Save dmesg to /mnt/Log (if enabled in config).
 * @return 0
 */
int save_dmesg_to_file()
{
    char cmd[256] = {0};
    DATE_S t;
    AK_BASE_GetLocalDate(&t);
    if(g_config.common_param.dmesg != 0)
    {
        sprintf(cmd, "dmesg > /mnt/Log/dmesg_%4d%02d%02d_%02d%02d%02d.log",  t.s32Year,t.s32Month,t.s32Day,t.s32Hour,t.s32Minute,t.s32Second);
        system(cmd);
    }
    //printf("app_main_config:%d,%d,%d,%d %s\n", g_config.common_param.dmesg, g_config.common_param.middle_log, g_config.common_param.rtt_log, g_config.common_param.app_log,cmd);
    return 0;
}

/**
 * @brief Copy app log to /mnt/Log (if enabled in config).
 * @return 0
 */
int save_app_log_to_file()
{
    char cmd[256] = {0};
    DATE_S t;
    AK_BASE_GetLocalDate(&t);
    if(g_config.common_param.app_log_save != 0)
    {
        fflush(stdout);
        fclose(stdout);
        sprintf(cmd, "cp /tmp/app_log.txt /mnt/Log/app_log_%4d%02d%02d_%02d%02d%02d.log",  t.s32Year,t.s32Month,t.s32Day,t.s32Hour,t.s32Minute,t.s32Second);
        system(cmd);
    }
    //printf("app_main_config:%d,%d,%d,%d %s\n", g_config.common_param.dmesg, g_config.common_param.middle_log, g_config.common_param.rtt_log, g_config.common_param.app_log,cmd);
    return 0;

}

/**
 * @brief Apply config for app log, RTT, dmesg, etc.
 */
void set_configuration(void)
{
    /* output rtt log */
    if(g_config.common_param.rtt_log)
    {
        //system("cat /dev/fastsys-log");
    }

    if(g_config.common_param.middle_log)
    {
        int level = g_config.common_param.middle_log;
        if(level < LOG_LEVEL_ERROR)
            level = LOG_LEVEL_ERROR;
        else if(level > LOG_APP_DEBUG)
            level =  LOG_LEVEL_DEBUG;
        AK_BASE_PrintSetLevel(MODULE_ID_ALL, level);
    }

    //app log level
    if(g_config.common_param.app_log)
    {
        int level = g_config.common_param.app_log;
        if(level < LOG_APP_FATAL)
            level = LOG_APP_FATAL;
        else if(level > LOG_APP_DEBUG)
            level =  LOG_APP_DEBUG;
        log_print_app_set_level(level);
    }

    if(g_config.common_param.app_log_save)
    {
        setlinebuf(stdout);
        freopen("/tmp/app_log.txt","a",stdout);
        printf("app_log_save__[syst=%lld]\n",get_sys_run_time64());
        printf("app_main_config:dmesg=%d,middle_log=%d,rtt_log=%d,app_log=%d,app_log_save=%d,power_off_time=%d\n", g_config.common_param.dmesg, g_config.common_param.middle_log, g_config.common_param.rtt_log, g_config.common_param.app_log,g_config.common_param.app_log_save,g_config.common_param.power_off_time);
        ai_glass_print_sw_version();
    }
}

/**
 * @brief ak sdk destory
 *
 * @return int
 */
int ak_sdk_env_destory()
{
    return AK_BASE_SdkExit();
}

int km02_work_mode = WORK_MODE_IDLE;

int get_work_mode(void)
{

    return km02_work_mode;
}

void set_work_mode(work_mode_t work_mode)
{
     km02_work_mode = work_mode;
}
int global_cmd_id = 0;

int is_bit7_set(unsigned char hex_value)
{
    return (hex_value & 0x80) ? 1 : 0;
}

/**
 * Append data to file
 * @param filename: target file name
 * @param pdata: pointer to data to append
 * @param len: length of data to append
 * @return: 0 on success, -1 on failure
 */
int append_write_file(char *filename, char* pdata, int len)
{
    // Parameter validation
    if (filename == NULL || pdata == NULL || len < 0) {
        fprintf(stderr, "Invalid parameters\n");
        return -1;
    }

    // Open file in binary append mode
    FILE *fp = fopen(filename, "ab");
    if (fp == NULL) {
        perror("Failed to open file");
        return -1;
    }

    // Write data to file
    size_t written = fwrite(pdata, 1, len, fp);
    if (written != (size_t)len) {
        perror("Failed to write all data");
        fclose(fp);
        return -1;
    }

    // Flush data to ensure it's written to disk
    if (fflush(fp) != 0) {
        perror("Failed to flush file");
        fclose(fp);
        return -1;
    }

    // Close file
    if (fclose(fp) != 0) {
        perror("Failed to close file");
        return -1;
    }

    return 0;  // Success
}

int ensure_directory_exists(const char *path)
{
    struct stat st;

    // Check if directory already exists
    if (stat(path, &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            // printf("Directory already exists: %s\n", path);
            return 0;
        } else {
            // Path exists but is not a directory
            fprintf(stderr, "Error: %s exists but is not a directory\n", path);
            errno = ENOTDIR;
            return -1;
        }
    }

    // Directory doesn't exist, try to create it
    if (mkdir(path, 0755) == 0) {
        printf("Directory created successfully: %s\n", path);
        return 0;
    }

    // Handle creation failure
    fprintf(stderr, "Failed to create directory: %s (%s)\n", path, strerror(errno));
    return -1;
}



int file_exists(const char *path) {
    return access(path, F_OK) == 0;
}
// Get MD5 value of file
int get_md5(const char *file, char *md5_buf, size_t buf_len) {
    char cmd[512];
    FILE *fp;
    snprintf(cmd, sizeof(cmd), "md5sum \"%s\"", file);

    fp = popen(cmd, "r");
    if (fp == NULL) {
        return -1;
    }

    if (fscanf(fp, "%50s", md5_buf) != 1) {
        pclose(fp);
        return -1;
    }

    pclose(fp);
    return 0;
}

// Compare MD5 values of two files
int compare_md5(const char *file1, const char *file2) {
    char md5_1[64] = {0};
    char md5_2[64] = {0};

    if (get_md5(file1, md5_1, sizeof(md5_1)) != 0 ||
        get_md5(file2, md5_2, sizeof(md5_2)) != 0) {
        return -1; // Get failed
    }

    return strcmp(md5_1, md5_2) == 0 ? 0 : 1;
}

int replace_with_mv(const char *src, const char *dest ,const char *back_file) {
    char cmd[512];

    // Backup original file
    snprintf(cmd, sizeof(cmd), "cp \"%s\" \"%s\"", dest, back_file);
    if (system(cmd) != 0) {
        perror("Backup failed");
        return -1;
    }

    // Move new file to overwrite old file
    snprintf(cmd, sizeof(cmd), "mv \"%s\" \"%s\"", src, dest);
    if (system(cmd) != 0) {
        // Rollback
        snprintf(cmd, sizeof(cmd), "mv \"%s\" \"%s\"", back_file, dest);
        system(cmd);
        return -1;
    }

    return 0;
}

int check_camera_ini(void)
{
    if (!file_exists(CAMERA_SRC_FILE)) {
        log_error_app("camera Source file does not exist: %s", CAMERA_SRC_FILE);
        return 0;
    }

    if (compare_md5(CAMERA_SRC_FILE, CAMERA_DEST_FILE) != 0) {
        log_info_app("camera MD5 differs. Replacing with mv...");
        if (replace_with_mv(CAMERA_SRC_FILE, CAMERA_DEST_FILE ,CAMERA_BACKUP_FILE) == 0) {
            log_info_app("Replace success.");
        } else {
            log_info_app("Replace failed.");
        }
    } else {
        log_info_app("Files are identical (by MD5). No action taken.");
    }

    return 0;
}

int check_app_ini(void)
{
    if (!file_exists(APP_SRC_FILE)) {
        log_error_app("app Source file does not exist: %s", APP_SRC_FILE);
        return 0;
    }

    if (compare_md5(APP_SRC_FILE, APP_DEST_FILE) != 0) {
        log_info_app("app MD5 differs. Replacing with mv...");
        if (replace_with_mv(APP_SRC_FILE, APP_DEST_FILE ,APP_BACKUP_FILE) == 0) {
            log_info_app("Replace success.");
        } else {
            log_info_app("Replace failed.");
        }
    } else {
        log_info_app("Files are identical (by MD5). No action taken.");
    }

    return 0;
}


int ak_rename_file(const char *src, const char *dst)
{
    char cmd[512] = {0};
    snprintf(cmd, sizeof(cmd), "mv %s %s", src, dst);
    system(cmd);
    printf("fun=%s line=%d cmd:%s \n",__FUNCTION__,__LINE__,cmd);
    return 0;
}
