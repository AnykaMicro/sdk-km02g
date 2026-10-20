/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd. */
#ifndef _AI_GLASS_CMD_IPI_H__
#define _AI_GLASS_CMD_IPI_H__


/**
 * @brief Initializes the IPI (Inter-Processor Interrupt) command module.
 *
 * @return 0 on success, negative error code on failure.
 */
int ai_glass_ipi_cmd_init(void);

/**
 * @brief Deinitializes the IPI command module.
 *
 * @return 0 on success, negative error code on failure.
 */
int ai_glass_ipi_cmd_deinit(void);

/**
 * @brief Opens the file descriptor for IPI communication.
 *
 * @return File descriptor on success, negative error code on failure.
 */
int ai_glass_ipi_cmd_fd_open(void);

/**
 * @brief Closes the file descriptor for IPI communication.
 *
 * @return 0 on success, negative error code on failure.
 */
int ai_glass_ipi_cmd_fd_close(void);

/**
 * @brief Signals the semaphore for IPI command synchronization.
 */
void ai_glass_ipi_cmd_sem_signal(void);

/**
 * @brief Initializes the semaphore for IPI command synchronization.
 */
void ai_glass_ipi_cmd_sem_init(void);

/**
 * @brief Retrieves the next pending IPI command.
 *
 * @param glass_info Pointer to the structure to store the command information.
 * @return 0 on success, negative error code on failure.
 */
int ai_glass_ipi_cmd_get(quick_glass_info_t *glass_info);

/**
 * @brief Marks an IPI command as completed.
 *
 * @param glass_info Pointer to the structure containing the completed command information.
 */
void ai_glass_ipi_cmd_completed(quick_glass_info_t *glass_info);

/**
 * @brief Checks if the IPI command system is ready.
 *
 * @return 1 if ready, 0 if not ready.
 */
int ai_glass_ipi_cmd_ready(void);

/**
 * @brief Packs data into the glass info structure.
 *
 * @param result Result code to pack.
 * @param output Buffer to store the packed output.
 * @param data Input data to pack.
 * @param lens Length of the input data.
 * @return Length of the packed data on success, or negative error code.
 */
int ai_glass_info_pack_data(uint8_t result, uint8_t* output, uint8_t* data, uint8_t lens);

/**
 * @brief Sends a notification response with command data to the glass device (version 4 protocol).
 *
 * @param fd File descriptor for the communication channel.
 * @param cmd Command identifier.
 * @param parameter Pointer to the response data payload.
 * @param param_len Length of the response data in bytes.
 * @return 0 on success, negative value on failure.
 */
int set_glass_info_v4_notify(unsigned int cmd, char *parameter, unsigned int param_len);

/**
 * @brief Sends command response data to the glass device (version 4 protocol).
 *
 * @param fd File descriptor for the communication channel.
 * @param cmd Command identifier.
 * @param parameter Pointer to the response data payload.
 * @param param_len Length of the response data in bytes.
 * @return 0 on success, negative value on failure.
 */
int set_glass_info_v4(unsigned int cmd, char *parameter, unsigned int param_len);



#endif
