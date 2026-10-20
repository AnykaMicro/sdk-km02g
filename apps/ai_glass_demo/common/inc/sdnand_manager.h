/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd. */
#ifndef __STORAGE_MANAGER_H__
#define __STORAGE_MANAGER_H__

#include <stdint.h>

#define IS_VIDEO_FULL_PERSENT (90) //Storage full threshold
#define IS_AUDIO_FULL_PERSENT (90) //Storage full threshold
#define IS_PHOTO_FULL_PERSENT (90) //Storage full threshold


typedef struct {
    uint16_t used;  // Used space, unit MB
    uint16_t left;  // Free space, unit MB
} StorageInfo;

/**
 * @brief sd card/nand mount func
 *
 * @param device device path (e.g., /dev/mmcblk1p1)
 * @param target mount path (e.g., /mnt)
 * @return int 0:success others:error
 */
int sd_nand_mount(const char *device, const char *target);

/**
 * @brief sd card/nand umount func
 *
 * @param target mount path
 * @return int 0:success others:error
 */
int sd_nand_umount(const char *target);

/**
 * @brief check if device is mounted
 *
 * @param device device path (e.g., /dev/mmcblk1p1)
 * @return int 1:mounted 0:not mounted -1:error
 */
int sd_nand_is_mounted(const char *device);

/**
 * @brief format device to fat32
 *
 * @param device device path (e.g., /dev/mmcblk1p1)
 * @return int 0:success others:error
 */
int sd_nand_format_to_fat32(const char *device);

/**
 * @brief check if device is full based on percentage threshold
 *
 * @param mount_path mount path (e.g., /mnt)
 * @param is_full_percent percentage threshold [0-100]
 * @return int 1:full 0:not full -1:error
 */
int sd_nand_is_full(const char *mount_path, int is_full_percent);

/**
 * @brief get free space of mounted device in MB
 *
 * @param mount_path mount path (e.g., /mnt)
 * @return long free space in MB, -1 on error
 */
long sd_nand_get_free_space(const char *mount_path);

/**
 * @brief count files and directories in specified directory
 *
 * @param dir_path directory path
 * @return int count of files and directories, -1 on error
 */
int sd_nand_get_dir_file_count(const char *dir_path);

/**
 * @brief initialize file paths for media storage
 *
 * Checks if the specified device is mounted to /mnt, mounts it if not,
 * then creates necessary directories for media files if they don't exist.
 * Directories include: Temp, TempAudio, Log, Videos, TempVideos,
 * Pictures, Audio, Music, Schedule, OTA
 *
 * @param device device path (e.g., /dev/mmcblk1p1)
 * @return int 0:success (all directories created/exist) -1:error (mount failed or some directories failed)
 */
int sd_nand_file_path_init(const char *device);


/**
 * @brief get all file and directory names in specified directory
 *
 * @param dir_path directory path
 * @param file_names 2D array to store file names [max_count][256]
 * @param count output parameter for actual file count
 * @param max_count maximum number of files to return (should not exceed 255)
 * @return int 0:success -1:error
 */
int sd_nand_get_dir_file_names(const char *dir_path, char file_names[][256], int *count, int max_count);

/**
 * @brief find all files containing specified string in their names
 *
 * @param dir_path directory path to search in
 * @param search_str string to search for in file names (case sensitive)
 * @param found_names 2D array to store matching file names [max_count][256]
 * @param count output parameter for actual matching file count
 * @param max_count maximum number of matching files to return (should not exceed 255)
 * @return int 0:success -1:error
 */
int sd_nand_find_files_containing(const char *dir_path, const char *search_str, char found_names[][256], int *count, int max_count);

/**
 * @brief Wait for user file to become ready
 * @param file_path Path to the file to check
 * @return 0 on success, -1 on failure
 */
int wait_for_usr_file_ready(char *file_path);

/**
 * @brief Wait for script execution to complete
 * @return 0 on success, -1 on failure
 */
int wait_for_script();

/**
 * @brief Wait for SDNAND command to complete
 * @param cmd SDNAND command to wait for
 * @return 0 on success, -1 on failure
 */
int wait_for_sdnand(unsigned int cmd);

/**
 * @brief Counts the number of files with a specific extension in a given directory.
 * @param dir_path Path to the directory to search in.
 * @param extension File extension to filter by (e.g., "mp4", "jpg").
 * @param mode Search mode (e.g., recursive or non-recursive).
 * @return Number of files found with the specified extension, or -1 on error.
 */
int sd_nand_count_files_by_extension(const char *dir_path, const char *extension, int mode);

/**
 * @brief Checks if a file or directory exists at the specified path.
 *
 * @param file_path Full path to the file or directory to check.
 * @return 1 if exists, 0 if not exists, -1 on error (invalid parameter).
 */
int sd_nand_file_exists(const char *file_path);

/**
 * @brief Retrieves storage information (used and free space).
 *
 * @param mount_path Mount point path (e.g., "/mnt").
 * @param info Pointer to the StorageInfo structure to store the result.
 * @return 0 on success, negative error code on failure.
 */
int sd_nand_get_storage_info(const char *mount_path, StorageInfo *info);

/**
 * @brief Starts the storage monitoring thread.
 *
 * Monitors storage usage and handles full-storage scenarios.
 *
 * @param mount_path Mount point path to monitor.
 * @return 0 on success, negative error code on failure.
 */
int start_storage_monitor(const char* mount_path);

/**
 * @brief Stops the storage monitoring thread.
 *
 * @return 0 on success, negative error code on failure.
 */
int stop_storage_monitor(void);


#endif
