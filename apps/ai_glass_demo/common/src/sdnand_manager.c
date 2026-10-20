/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <sys/statvfs.h>
#include <dirent.h>

#include "fun_log.h"
#include "sdnand_manager.h"
#include "media_configs.h"
#include "ai_glass_common.h"
#include "ai_glass_log.h"


int sd_nand_is_full(const char *mount_path, int is_full_percent)
{
    struct statvfs stat;
    unsigned long long total_space, used_space;
    int usage_percent;

    if (!mount_path) {
        log_error_app("Error: mount_path parameter is NULL\n");
        return -1;
    }

    if (is_full_percent < 0 || is_full_percent > 100) {
        log_error_app("Error: is_full_percent must be between 0 and 100\n");
        return -1;
    }

    // Get filesystem statistics
    if (statvfs(mount_path, &stat) != 0) {
        perror("Failed to get filesystem statistics");
        return -1;
    }

    // Calculate total space and used space (in bytes)
    total_space = (unsigned long long)stat.f_blocks * stat.f_frsize;
    used_space = total_space - ((unsigned long long)stat.f_bavail * stat.f_frsize);

    // Calculate usage percentage
    if (total_space > 0) {
        usage_percent = (int)((used_space * 100) / total_space);
    } else {
        log_error_app("Error: Total space is 0\n");
        return -1;
    }

    printf("Storage usage: %d%% (threshold: %d%%)\n", usage_percent, is_full_percent);

    // Check if threshold is exceeded
    return (usage_percent >= is_full_percent) ? 1 : 0;
}

long sd_nand_get_free_space(const char *mount_path)
{
    struct statvfs stat;
    unsigned long long free_space_bytes;
    long free_space_mb;

    if (!mount_path) {
        log_error_app("Error: mount_path parameter is NULL\n");
        return -1;
    }

    // Get filesystem statistics
    if (statvfs(mount_path, &stat) != 0) {
        perror("Failed to get filesystem statistics");
        return -1;
    }

    // Calculate free space (in bytes)
    free_space_bytes = (unsigned long long)stat.f_bavail * stat.f_frsize;

    // Convert to MB
    free_space_mb = (long)(free_space_bytes / (1024 * 1024));

    log_info_app("Free space: %ld MB\n", free_space_mb);

    return free_space_mb;
}

int sd_nand_get_dir_file_count(const char *dir_path)
{
    DIR *dir;
    struct dirent *entry;
    int count = 0;

    if (!dir_path) {
        log_error_app("Error: dir_path parameter is NULL\n");
        return -1;
    }

    // Open directory
    dir = opendir(dir_path);
    if (!dir) {
        perror("Failed to open directory");
        return 0;
    }

    // Iterate directory entries
    while ((entry = readdir(dir)) != NULL) {
        // Skip current directory (.) and parent directory (..)
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        count++;
    }

    closedir(dir);

    log_info_app("Directory %s contains %d files/directories\n", dir_path, count);

    return count;
}

int sd_nand_is_mounted(const char *device)
{
    FILE *fp;
    char line[256];
    char mount_device[256];

    if (!device) {
        log_error_app("Error: device parameter is NULL\n");
        return -1;
    }

    // Open /proc/mounts file to check mount info
    fp = fopen("/proc/mounts", "r");
    if (!fp) {
        perror("Failed to open /proc/mounts");
        return -1;
    }

    // Read mount info line by line
    while (fgets(line, sizeof(line), fp)) {
        // Parse the first field (device name) of each line
        if (sscanf(line, "%255s", mount_device) == 1) {
            // Compare if device name matches
            if (strcmp(mount_device, device) == 0) {
                fclose(fp);
                return 1; // Mounted
            }
        }
    }

    fclose(fp);
    return 0; // Not mounted
}

// Helper function: Get device mount point
static int get_mount_point(const char *device, char *mount_point, size_t size)
{
    FILE *fp;
    char line[512];
    char mount_device[256], mount_path[256];

    fp = fopen("/proc/mounts", "r");
    if (!fp) {
        return -1;
    }

    while (fgets(line, sizeof(line), fp)) {
        if (sscanf(line, "%255s %255s", mount_device, mount_path) == 2) {
            if (strcmp(mount_device, device) == 0) {
                strncpy(mount_point, mount_path, size - 1);
                mount_point[size - 1] = '\0';
                fclose(fp);
                return 0;
            }
        }
    }

    fclose(fp);
    return -1;
}

int sd_nand_format_to_fat32(const char *device)
{
    char command[512];
    char mount_point[256];
    int ret;
    int was_mounted = 0;

    if (!device) {
        log_error_app("Error: device parameter is NULL\n");
        return -1;
    }

    // Check if device file exists
    if (access(device, F_OK) != 0) {
        log_error_app("Error: Device %s does not exist\n", device);
        return -1;
    }

    // Check if device is mounted
    ret = sd_nand_is_mounted(device);
    if (ret == 1) {
        // Device is mounted, get mount point and unmount
        if (get_mount_point(device, mount_point, sizeof(mount_point)) == 0) {
            log_info_app("Device %s is mounted at %s, unmounting...\n", device, mount_point);
            ret = sd_nand_umount(mount_point);
            if (ret != 0) {
                log_error_app("Error: Failed to unmount device %s from %s\n", device, mount_point);
                return -1;
            }
            was_mounted = 1;
        } else {
            log_error_app("Error: Device %s is mounted but could not determine mount point\n", device);
            return -1;
        }
    } else if (ret == -1) {
        log_error_app("Warning: Could not check mount status of %s\n", device);
    }

    log_info_app("Formatting device %s to FAT32...\n", device);

    // Use mkfs.vfat command to format to FAT32
    // -F 32: Specify FAT32 file system
    // -v: Detailed output
    snprintf(command, sizeof(command), "mkfs.vfat -F 32 -v %s", device);

    ret = system(command);
    if (ret != 0) {
        log_info_app("Error: Failed to format device %s to FAT32\n", device);
        log_info_app("Make sure:\n");
        log_info_app("1. You have root permissions\n");
        log_info_app("2. mkfs.fat tool is available on the system\n");

        // If formatting failed and it was mounted before, try to remount
        if (was_mounted) {
            log_info_app("Attempting to remount device at %s...\n", mount_point);
            sd_nand_mount(device, mount_point);
        }
        return -1;
    }

    log_info_app("Device %s successfully formatted to FAT32\n", device);

    // If it was mounted before, remount
    if (was_mounted) {
        log_info_app("Remounting device %s at %s...\n", device, mount_point);
        ret = sd_nand_mount(device, mount_point);
        if (ret != 0) {
            log_info_app("Warning: Device formatted successfully but failed to remount at %s\n", mount_point);
            log_info_app("You may need to manually mount the device\n");
        }
    }

    return 0;
}

int sd_nand_find_files_containing(const char *dir_path, const char *search_str, char found_names[][256], int *count, int max_count)
{
    DIR *dir;
    struct dirent *entry;
    int found_count = 0;

    // Parameter check
    if (!dir_path) {
        log_error_app("Error: dir_path parameter is NULL\n");
        return -1;
    }

    if (!search_str) {
        log_error_app("Error: search_str parameter is NULL\n");
        return -1;
    }

    if (!found_names) {
        log_error_app("Error: found_names parameter is NULL\n");
        return -1;
    }

    if (!count) {
        log_error_app("Error: count parameter is NULL\n");
        return -1;
    }

    if (max_count <= 0 || max_count > 255) {
        log_error_app("Error: max_count must be between 1 and 255, got %d\n", max_count);
        return -1;
    }

    if (strlen(search_str) == 0) {
        log_error_app("Error: search_str cannot be empty\n");
        return -1;
    }

    // Initialize output parameters
    *count = 0;

    // Open directory
    dir = opendir(dir_path);
    if (!dir) {
        log_error_app("Error: Failed to open directory %s: ", dir_path);
        perror("");
        return -1;
    }

    log_info_app("Searching for files containing '%s' in directory %s...\n", search_str, dir_path);

    // Iterate directory entries
    while ((entry = readdir(dir)) != NULL && found_count < max_count) {
        // Skip current directory (.) and parent directory (..)
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Check file name length, ensure it does not exceed 255 characters
        if (strlen(entry->d_name) >= 256) {
            log_info_app("Warning: File name too long, skipping: %.50s...\n", entry->d_name);
            continue;
        }

        // Use strstr for substring matching (case sensitive)
        if (strstr(entry->d_name, search_str) != NULL) {
            // Found matching file, copy file name to output array
            strncpy(found_names[found_count], entry->d_name, 255);
            found_names[found_count][255] = '\0'; // Ensure string ends with null
            found_count++;
            log_info_app("Found matching file: %s\n", entry->d_name);
        }
    }

    closedir(dir);

    // Set output parameters
    *count = found_count;

    log_info_app("Search completed: found %d matching files", found_count);
    if (found_count == max_count) {
        printf(" (limited to max_count=%d)", max_count);
    }
    log_info_app("\n");

    return 0;
}

int sd_nand_mount(const char *device, const char *target)
{
    // Check if device file exists
    if (access(device, F_OK) != 0)
    {
        log_error_app("Device %s does not exist\n", device);
        return -1;
    }

    // Check if mount directory exists, create if not
    if (access(target, F_OK) != 0)
    {
        if (mkdir(target, 0755) != 0)
        {
            perror("Failed to create mount directory");
            return -1;
        }
        log_info_app("Created mount directory %s\n", target);
    }

    // Try to mount, first try vfat(fat32) file system, if failed then try exfat
    if (mount(device, target, "vfat", MS_NOATIME, "utf8") == 0)
    {
        log_info_app("SD card successfully mounted from %s to %s (fat32)\n", device, target);
        return 0;
    }
    else if (mount(device, target, "exfat", MS_NOATIME, "utf8") == 0)
    {
        log_info_app("SD card successfully mounted from %s to %s (exfat)\n", device, target);
        return 0;
    }
    else
    {
        // Mount failed
        perror("mount failed");
        fprintf(stderr, "Error: Make sure:\n"
                        "1. You have root permissions (run with sudo)\n"
                        "2. Device %s is a valid block device\n"
                        "3. Device is not already mounted\n"
                        "4. File system is supported (fat32/exfat)\n",
                device);
        return -1;
    }
}

int sd_nand_umount(const char *target)
{
    // const char* target = "/mnt";
    if (umount(target) == 0)
    {
        // umount ok
        log_info_app("SD card successfully unmounted from %s\n", target);
        return 0;
    }
    else
    {
        // umount failed
        perror("umount failed");
        fprintf(stderr, "Error: Make sure:\n"
                        "1. You have root permissions (run with sudo)\n"
                        "2. No processes are using %s\n"
                        "3. Device is actually mounted there\n",
                target);
        return -1;
    }

    return 0;
}

int sd_nand_file_path_init(const char *device)
{
    // Define directory path array to be created
    const char *directories[] = {
        TEMP_PATH,           // "/mnt/Temp"
        TEMP_AUDIO_PATH,     // "/mnt/TempAudio"
        LOG_PATH,            // "/mnt/Log"
        VIDEOS_PATH,         // "/mnt/Videos"
        TEMP_VIDEOS_PATH,    // "/mnt/TempVideos"
        PICTURES_PATH,       // "/mnt/Pictures"
        AUDIO_PATH,          // "/mnt/Audio"
        MUSIC_PATH,          // "/mnt/Music"
        SCHEDULE_PATH,       // "/mnt/Schedule"
        OTA_PATH             // "/mnt/OTA"
    };

    int dir_count = sizeof(directories) / sizeof(directories[0]);
    int success_count = 0;
    int ret;
    char mount_point[256];

    if (!device) {
        log_error_app("Error: device parameter is NULL\n");
        return -1;
    }

    log_info_app("Initializing SD/NAND file paths for device %s...\n", device);

    // Check if device is mounted
    ret = sd_nand_is_mounted(device);
    if (ret == 1) {
        // Device is mounted, check if mounted to /mnt
        if (get_mount_point(device, mount_point, sizeof(mount_point)) == 0) {
            if (strcmp(mount_point, SD_NAND_MOUNT_PATH) == 0) {
                log_info_app("Device %s is already mounted at %s\n", device, SD_NAND_MOUNT_PATH);
            } else {
                log_info_app("Device %s is mounted at %s, but expected at %s\n",
                       device, mount_point, SD_NAND_MOUNT_PATH);
                log_info_app("Warning: Directories will be created at the expected mount point %s\n",
                       SD_NAND_MOUNT_PATH);
            }
        } else {
            log_info_app("Error: Device %s is mounted but could not determine mount point\n", device);
            return -1;
        }
    } else if (ret == 0) {
        // Device not mounted, try to mount to /mnt
        log_info_app("Device %s is not mounted, attempting to mount to %s...\n",
               device, SD_NAND_MOUNT_PATH);
        ret = sd_nand_mount(device, SD_NAND_MOUNT_PATH);
        if (ret != 0) {
            log_error_app("Error: Failed to mount device %s to %s\n", device, SD_NAND_MOUNT_PATH);
            return -1;
        }
        log_info_app("Device %s successfully mounted to %s\n", device, SD_NAND_MOUNT_PATH);
    } else {
        // Error checking mount status
        log_error_app("Error: Could not check mount status of device %s\n", device);
        return -1;
    }

    // Check and create each directory sequentially
    for (int i = 0; i < dir_count; i++) {
        // Check if directory exists
        if (access(directories[i], F_OK) == 0) {
            // Directory already exists
            // printf("Directory %s already exists\n", directories[i]);
            success_count++;
        } else {
            // Directory does not exist, try to create
            ret = mkdir(directories[i], 0755);
            if (ret == 0) {
                log_info_app("Successfully created directory %s\n", directories[i]);
                success_count++;
            } else {
                log_error_app("Failed to create directory %s: ", directories[i]);
                perror("");
                // Continue to process other directories, do not return error immediately
            }
        }
    }

    log_info_app("File path initialization completed: %d/%d directories ready\n",
           success_count, dir_count);

    // If all directories are successfully created or already exist, return 0; otherwise return -1
    return (success_count == dir_count) ? 0 : -1;
}

int sd_nand_get_dir_file_names(const char *dir_path, char file_names[][256], int *count, int max_count)
{
    DIR *dir;
    struct dirent *entry;
    int file_count = 0;

    // Parameter check
    if (!dir_path) {
        log_error_app("Error: dir_path parameter is NULL\n");
        return -1;
    }

    if (!file_names) {
        log_error_app("Error: file_names parameter is NULL\n");
        return -1;
    }

    if (!count) {
        log_error_app("Error: count parameter is NULL\n");
        return -1;
    }

    if (max_count <= 0 || max_count > 255) {
        log_error_app("Error: max_count must be between 1 and 255, got %d\n", max_count);
        return -1;
    }

    // Initialize output parameters
    *count = 0;

    // Open directory
    dir = opendir(dir_path);
    if (!dir) {
        log_error_app("Error: Failed to open directory %s: \n", dir_path);
        perror("");
        return -1;
    }

    log_info_app("Reading directory %s...\n", dir_path);

    // Iterate directory entries
    while ((entry = readdir(dir)) != NULL && file_count < max_count) {
        // Skip current directory (.) and parent directory (..)
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Check file name length, ensure it does not exceed 255 characters
        if (strlen(entry->d_name) >= 256) {
            log_info_app("Warning: File name too long, skipping: %.50s...\n", entry->d_name);
            continue;
        }

        // Copy file name to output array
        strncpy(file_names[file_count], entry->d_name, 255);
        file_names[file_count][255] = '\0'; // Ensure string ends with null
        file_count++;
    }

    closedir(dir);

    // Set output parameters
    *count = file_count;

    log_info_app("Found %d files/directories in %s", file_count, dir_path);
    if (file_count == max_count) {
        log_info_app(" (limited to max_count=%d)", max_count);
    }
    log_info_app("\n");

    return 0;
}

int wait_for_usr_file_ready(char *file_path)
{
    int retries = 0;

    while (retries < P2P_CONNECT_MAX_RETRIES)
    {
        // Check if file exists
        if (access(file_path, F_OK) == 0)
        {
            // printf("Found: %s retries %d\n", file_path, retries);
            return 0; // Success
        }

        // Wait for a while (microseconds)
        usleep(5000); // usleep receives microseconds

        retries++;
    }

    log_info_app("Found %s timeout .\n", file_path);
    return -1; // Failed
}
int wait_for_script()
{
    int retries = 0;

    while (retries < P2P_CONNECT_MAX_RETRIES)
    {
        // Check if script exists
        if (access(P2P_SH_PATH, F_OK) == 0)
        {
            log_info_app("Found: %s retries %d\n", P2P_SH_PATH, retries);
            return 0; // Success
        }

        // Wait for a while (microseconds)
        usleep(5000); // usleep receives microseconds

        retries++;
    }

    log_info_app("Found %s timeout .\n", P2P_SH_PATH);
    return -1; // Failed
}
int wait_for_sdnand(unsigned int cmd)
{
    int retries = 0;
    char *wait_path = NULL;

    switch (cmd)
    {
    case BT_CMD_PHOTO:
    case APP_CMD_PHOTO:
        wait_path = PICTURES_PATH;
        break;
    case BT_CMD_RECORD:
    case APP_CMD_RECORD:
        wait_path = TEMP_VIDEOS_PATH;
        break;
    case BT_CMD_RECORD_AUDIO:
    case APP_CMD_RECORD_AUDIO:
        wait_path = AUDIO_PATH;
        break;
    default:
        wait_path = LOG_PATH;
        break;
    }

    while (retries < WAIT_SDNAND_MAX_RETRIES)
    {
        // Check if script exists
        if (access(wait_path, W_OK) == 0)
        {
            log_info_app("%s: %s retries %d\n", __func__,wait_path, retries);
            return 0; // Success
        }

        // Wait for a while (microseconds)
        usleep(5000); // usleep receives microseconds

        retries++;
    }

    log_info_app("Found %s timeout .\n", wait_path);
    return -1; // Failed
}


// Find the number of specified file types in the specified directory
// Check file extension
static int has_extension(const char *filename, const char *extension) {
    const char *dot = strrchr(filename, '.');
    if (!dot || dot == filename) return 0;
    return strcmp(dot + 1, extension) == 0;
}

int sd_nand_count_files_by_extension(const char *dir_path, const char *extension, int mode)
{
    DIR *dir;
    struct dirent *entry;
    char full_path[1024];
    struct stat statbuf;
    int count = 0;

    // Parameter check
    if (dir_path == NULL || extension == NULL) {
        fprintf(stderr, "Invalid parameters\n");
        return -1;
    }

    dir = opendir(dir_path);
    if (dir == NULL) {
        perror("opendir");
        return -1;
    }

    while ((entry = readdir(dir)) != NULL) {
        // Skip . and ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Construct full path
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);

        // Get file status
        if (stat(full_path, &statbuf) == -1) {
            perror("stat");
            continue;
        }

        if (mode == 1) {
            // Recursive mode
            if (S_ISDIR(statbuf.st_mode)) {
                // If directory, recurse
                int subdir_count = sd_nand_count_files_by_extension(full_path, extension, mode);
                if (subdir_count >= 0) {
                    count += subdir_count;
                }
            } else if (S_ISREG(statbuf.st_mode)) {
                // If regular file and extension matches
                if (has_extension(entry->d_name, extension)) {
                    count++;
                }
            }
        } else {
            // Non-recursive mode - check current directory only
            if (S_ISREG(statbuf.st_mode)) {
                if (has_extension(entry->d_name, extension)) {
                    count++;
                }
            }
        }
    }

    closedir(dir);
    return count;
}
int sd_nand_file_exists(const char *file_path)
{
    if (!file_path) {
        log_error_app("Error: file_path parameter is NULL\n");
        return -1;
    }

    if (strlen(file_path) == 0) {
        log_error_app("Error: file_path cannot be empty\n");
        return -1;
    }

    // Use access function to check if file exists
    if (access(file_path, F_OK) == 0) {
        return 1; // File exists
    } else {
        return 0; // File does not exist
    }
}


long sd_nand_get_total_space_mb(const char *mount_path)
{
    struct statvfs stat;
    unsigned long long total_space_bytes;
    long total_space_mb;

    if (!mount_path) {
        log_error_app("Error: mount_path parameter is NULL\n");
        return -1;
    }

    // Get filesystem statistics
    if (statvfs(mount_path, &stat) != 0) {
        perror("Failed to get filesystem statistics");
        return -1;
    }

    // Calculate total space (in bytes)
    total_space_bytes = (unsigned long long)stat.f_blocks * stat.f_frsize;

    // Convert to MB
    total_space_mb = (long)(total_space_bytes / (1024 * 1024));

    log_info_app("Total space: %ld MB\n", total_space_mb);

    return total_space_mb;
}

int sd_nand_get_storage_info(const char *mount_path, StorageInfo *info)
{
    long total_space_mb;
    long free_space_mb;
    long used_space_mb;

    if (!mount_path || !info) {
        log_error_app("Error: invalid parameters\n");
        return -1;
    }

    // Get total space and free space
    total_space_mb = sd_nand_get_total_space_mb(mount_path);
    free_space_mb = sd_nand_get_free_space(mount_path);

    if (total_space_mb < 0 || free_space_mb < 0) {
        log_error_app("Error: failed to get storage information\n");
        return -1;
    }

    // Calculate used space
    used_space_mb = total_space_mb - free_space_mb;

    // Ensure non-negative
    if (used_space_mb < 0) {
        used_space_mb = 0;
    }

    // Assign to struct (ensure within uint16_t range)
    if (total_space_mb > 65535) {
        // If over 64GB, might need special handling or error return
        info->used = (uint16_t)(used_space_mb > 65535 ? 65535 : used_space_mb);
        info->left = (uint16_t)(free_space_mb > 65535 ? 65535 : free_space_mb);
    } else {
        info->used = (uint16_t)used_space_mb;
        info->left = (uint16_t)free_space_mb;
    }

    log_info_app("Storage Info - Used: %u MB, Free: %u MB\n", info->used, info->left);

    return 0;
}

static volatile bool g_monitor_running = false;
static pthread_t g_monitor_tid = 0;

void* storage_monitor_thread(void* arg)
{
    uint8_t result = RETURN_CODE_SUCCESS ;
    uint8_t glass_info_param[256] = {0};
    uint16_t glass_info_param_len = 0;

    const char* mount_path = (const char*)arg;

    log_info_app("Storage monitor thread started.\n");

    while (g_monitor_running) {
        sleep(5); // Check every 5 seconds
        int result = sd_nand_is_full(mount_path, IS_VIDEO_FULL_PERSENT);

        if (result == 1) {
            log_warn_app("Warning: Storage is almost full! (%d%% or more)\n", IS_VIDEO_FULL_PERSENT);
            int result = RETURN_CODE_SD_NAND_SPACE_FULL;
            memset(glass_info_param, 0, sizeof(glass_info_param));
            glass_info_param_len = ai_glass_info_pack_data(result, glass_info_param, NULL, 0);
            set_glass_info_v4_notify(BT_CMD_RECORD, (char *)glass_info_param, glass_info_param_len);

            printf("%s %d  sd nand full,stop recording !!\n",__func__,__LINE__);
            break;
        } else if (result < 0) {
            log_error_app("Error checking storage usage.\n");
        }
    }

    log_info_app("Storage monitor thread stopped.\n");
    return NULL;
}

int stop_storage_monitor(void)
{
    g_monitor_running = false;
    if (!g_monitor_running) {
        log_warn_app("Storage monitor is not running.\n");
        return 0;
    }
    return 0;
}

int start_storage_monitor(const char* mount_path)
{
    if (g_monitor_running) {
        log_warn_app("Storage monitor is already running.\n");
        return 0;
    }

    g_monitor_running = true;
    int ret = pthread_create(&g_monitor_tid, NULL, storage_monitor_thread, (void*)mount_path);
    if (ret != 0) {
        log_error_app("Failed to create storage monitor thread\n");
        g_monitor_running = false;
        return -1;
    }

    pthread_detach(g_monitor_tid);
    return 0;
}
