/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd. */


#ifndef _AI_GLASS_CMD_QUEUE_H__
#define _AI_GLASS_CMD_QUEUE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdarg.h>
#include "ai_glass_common.h"


/**
 * @brief Command queue node structure.
 */
typedef struct cmd_queue_node {
    quick_glass_info_t glass_info;  /**< Command information */
    struct cmd_queue_node *next;    /**< Pointer to the next node */
} cmd_queue_node_t;

/**
 * @brief Command queue structure.
 */
typedef struct {
    cmd_queue_node_t *head; /**< Pointer to the head of the queue */
    cmd_queue_node_t *tail; /**< Pointer to the tail of the queue */
    int count;              /**< Current number of items in the queue */
    int max_size;           /**< Maximum capacity of the queue */
} cmd_queue_t;


// Command queue related variables
#define CMD_QUEUE_MAX_SIZE 10   /**< Maximum size of the command queue */

/**
 * @brief Inter-thread communication structure in synchronous mode.
 */
typedef struct
{
    quick_glass_info_t glass_info;  /**< Command information */
    int valid;                      /**< Validity flag: 1 if valid, 0 otherwise */
    int processed;                  /**< Processed flag: 1 if processed, 0 otherwise */
} command_data_t;


/**
 * @brief Initializes the command queue and associated mutex for thread-safe operations.
 */
void cmd_queue_init(void);

/**
 * @brief Adds a new command to the end of the queue.
 *
 * @param glass_info Pointer to the command data to be queued.
 * @return 0 on success, negative value on failure (e.g., queue full).
 */
int cmd_queue_push(const quick_glass_info_t *glass_info);

/**
 * @brief Removes and retrieves the oldest command from the front of the queue.
 *
 * @param glass_info Pointer to store the retrieved command data.
 * @return 0 on success, negative value if queue is empty or error occurs.
 */
int cmd_queue_pop(quick_glass_info_t *glass_info);

/**
 * @brief Checks whether the command queue is currently empty.
 *
 * @return 1 if queue is empty, 0 otherwise.
 */
int cmd_queue_is_empty(void);

/**
 * @brief Clears all commands currently in the queue.
 */
void cmd_queue_clear(void);

/**
 * @brief Destroys the mutex used for protecting the command queue.
 *
 * @return 0 on success, negative value on failure.
 */
int destroy_queue_mutex(void);

/**
 * @brief Sends a notification to indicate that the Linux system is ready.
 *
 * @return 0 on success, negative value on failure.
 */
int send_linux_ready_notify();



#ifdef __cplusplus
}
#endif

#endif
