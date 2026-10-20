/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd. */

#include "fun_log.h"
#include "ai_glass_cmd_queue.h"
//#include "ai_glass_common.h"

static pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;

cmd_queue_t cmd_queue = {NULL, NULL, 0, CMD_QUEUE_MAX_SIZE};


void cmd_queue_init(void)
{
    pthread_mutex_lock(&queue_mutex);
    cmd_queue.head = NULL;
    cmd_queue.tail = NULL;
    cmd_queue.count = 0;
    cmd_queue.max_size = CMD_QUEUE_MAX_SIZE;
    pthread_mutex_unlock(&queue_mutex);
}

int cmd_queue_push(const quick_glass_info_t *glass_info)
{
    if (!glass_info) {
        return -1;
    }

    pthread_mutex_lock(&queue_mutex);

    // Check if the queue is full
    if (cmd_queue.count >= cmd_queue.max_size) {
        log_info_app("Command queue is full, dropping oldest command\n");
        // Delete the oldest command (at the head of the queue)
        if (cmd_queue.head) {
            cmd_queue_node_t *old_head = cmd_queue.head;
            cmd_queue.head = cmd_queue.head->next;
            if (cmd_queue.head == NULL) {
                cmd_queue.tail = NULL;
            }
            free(old_head);
            cmd_queue.count--;
        }
    }

    // Create a new node
    cmd_queue_node_t *new_node = (cmd_queue_node_t *)malloc(sizeof(cmd_queue_node_t));
    if (!new_node) {
        pthread_mutex_unlock(&queue_mutex);
        log_error_app("Failed to allocate memory for command queue node\n");
        return -1;
    }

    // Copy command data
    memcpy(&new_node->glass_info, glass_info, sizeof(quick_glass_info_t));
    new_node->next = NULL;

    // Add to the tail of the queue
    if (cmd_queue.tail) {
        cmd_queue.tail->next = new_node;
    } else {
        cmd_queue.head = new_node;
    }
    cmd_queue.tail = new_node;
    cmd_queue.count++;

    log_info_app("Command %d added to queue, queue size: %d\n", glass_info->cmd, cmd_queue.count);

    pthread_mutex_unlock(&queue_mutex);
    return 0;
}


int cmd_queue_pop(quick_glass_info_t *glass_info)
{
    if (!glass_info) {
        return -1;
    }

    pthread_mutex_lock(&queue_mutex);

    if (cmd_queue.head == NULL) {
        pthread_mutex_unlock(&queue_mutex);
        return -1; // Queue is empty
    }

    // Take out the command at the head of the queue
    cmd_queue_node_t *node = cmd_queue.head;
    memcpy(glass_info, &node->glass_info, sizeof(quick_glass_info_t));

    cmd_queue.head = node->next;
    if (cmd_queue.head == NULL) {
        cmd_queue.tail = NULL;
    }
    cmd_queue.count--;

    log_info_app("Command %d popped from queue, queue size: %d\n", glass_info->cmd, cmd_queue.count);

    free(node);
    pthread_mutex_unlock(&queue_mutex);
    return 0;
}


int cmd_queue_is_empty(void)
{
    pthread_mutex_lock(&queue_mutex);
    int empty = (cmd_queue.count == 0);
    pthread_mutex_unlock(&queue_mutex);
    return empty;
}


void cmd_queue_clear(void)
{
    pthread_mutex_lock(&queue_mutex);

    while (cmd_queue.head) {
        cmd_queue_node_t *node = cmd_queue.head;
        cmd_queue.head = node->next;
        free(node);
    }

    cmd_queue.tail = NULL;
    cmd_queue.count = 0;

    pthread_mutex_unlock(&queue_mutex);
    log_info_app("Command queue cleared\n");
}

int destroy_queue_mutex(void)
{
    // Directly call pthread_mutex_destroy, passing the address of the global variable queue_mutex
    int ret = pthread_mutex_destroy(&queue_mutex);

    if (ret != 0) {
        log_error_app("Error: pthread_mutex_destroy failed with code %d\n", ret);
    }

    return ret;
}


int send_linux_ready_notify()
{
    log_info_app("send_linux_ready_notify\n");
    //set_glass_info_v4_notify(glass_fd, CMD_POWER_ON_NOTIFY, NULL, 0);
    return 0;
}
