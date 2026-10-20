/* SPDX-License-Identifier: Apache-2.0 */
/**
 * @file ftp_monitor.c
 * @brief FTP connection monitor module implementation
 * @version 1.0
 * @date 2025-09-04
 *
 * @copyright Copyright (c) 2025 Guangzhou Anyka Microelectronics Co., Ltd.
 *
 */

#include "ftp_monitor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <errno.h>
#include "fun_log.h"
//#include "ai_glass_common.h"


/**
 * @brief FTP monitor module internal state structure
 */
typedef struct {
    ftp_event_callback_t callback;     /**< Event callback function */
    pthread_t monitor_thread;          /**< Monitor thread */
    pthread_mutex_t mutex;             /**< Mutex */
    volatile int running;              /**< Running flag */
    volatile int thread_exit;          /**< Thread exit flag */
    connection_list_t prev_connections; /**< Last scanned connection list */
    connection_list_t curr_connections; /**< Current scanned connection list */
} ftp_monitor_t;

static ftp_monitor_t g_ftp_monitor = {0};

/**
 * @brief Convert network byte order IP address to string
 *
 * @param addr Network byte order IP address
 * @param ip_str Output IP string buffer
 * @param size Buffer size
 */
static void addr_to_string(uint32_t addr, char *ip_str, size_t size)
{
    struct in_addr in_addr;
    in_addr.s_addr = addr;
    strncpy(ip_str, inet_ntoa(in_addr), size - 1);
    ip_str[size - 1] = '\0';
}

/**
 * @brief Parse /proc/net/tcp file to get FTP connection info
 *
 * @param conn_list Output connection list
 * @return int 0 success, -1 failure
 */
static int parse_proc_net_tcp(connection_list_t *conn_list)
{
    FILE *fp;
    char line[256];
    int line_num = 0;

    if (!conn_list) {
        log_error_app("FTP Monitor: Invalid connection list pointer\n");
        return -1;
    }

    // Clear connection list
    memset(conn_list, 0, sizeof(connection_list_t));

    fp = fopen("/proc/net/tcp", "r");
    if (!fp) {
        log_error_app("FTP Monitor: Failed to open /proc/net/tcp: %s\n", strerror(errno));
        return -1;
    }

    while (fgets(line, sizeof(line), fp) && conn_list->count < MAX_FTP_CONNECTIONS) {
        line_num++;

        // Skip header line
        if (line_num == 1) {
            continue;
        }

        unsigned int local_addr, remote_addr;
        unsigned int local_port, remote_port;
        unsigned int state;

        // Parse TCP connection info
        // Format: sl local_address rem_address st tx_queue rx_queue tr tm->when retrnsmt uid timeout inode
        int ret = sscanf(line, "%*d: %X:%X %X:%X %X %*X:%*X %*X:%*X %*X %*d %*d %*d",
                        &local_addr, &local_port, &remote_addr, &remote_port, &state);

        if (ret != 5) {
            continue;
        }

        // Only care about FTP port (21) connections, but ignore listening connections (0.0.0.0:0)
        if (local_port == 21 && !(remote_addr == 0 && remote_port == 0)) {
            tcp_connection_t *conn = &conn_list->connections[conn_list->count];

            conn->local_addr = local_addr;
            conn->local_port = local_port;
            conn->remote_addr = remote_addr;
            conn->remote_port = remote_port;
            conn->state = (tcp_state_t)state;

            // Convert remote IP address to string
            addr_to_string(remote_addr, conn->remote_ip_str, sizeof(conn->remote_ip_str));

            conn_list->count++;

            // log_info_app("FTP Monitor: Found FTP connection - Remote: %s:%u, State: 0x%02X\n",
            //        conn->remote_ip_str, remote_port, state);
        }
    }

    fclose(fp);

    // log_info_app("FTP Monitor: Found %d FTP connections\n", conn_list->count);
    return 0;
}

/**
 * @brief Find if connection exists in the connection list
 *
 * @param conn_list Connection list
 * @param remote_addr Remote address
 * @param remote_port Remote port
 * @return tcp_connection_t* Found connection, NULL if not found
 */
static tcp_connection_t* find_connection(connection_list_t *conn_list,
                                       uint32_t remote_addr, uint16_t remote_port)
{
    if (!conn_list) {
        return NULL;
    }

    for (int i = 0; i < conn_list->count; i++) {
        tcp_connection_t *conn = &conn_list->connections[i];
        if (conn->remote_addr == remote_addr && conn->remote_port == remote_port) {
            return conn;
        }
    }

    return NULL;
}

/**
 * @brief Compare connection differences between two scans, detect FTP events
 *
 * @param prev_list Last scanned connection list
 * @param curr_list Current scanned connection list
 */
static void detect_ftp_events(connection_list_t *prev_list, connection_list_t *curr_list)
{
    if (!prev_list || !curr_list || !g_ftp_monitor.callback) {
        return;
    }

    // Detect newly established connections (login event)
    for (int i = 0; i < curr_list->count; i++) {
        tcp_connection_t *curr_conn = &curr_list->connections[i];

        // Only care about established connections
        if (curr_conn->state != TCP_ESTABLISHED) {
            continue;
        }

        // Check if this connection existed in the last scan
        tcp_connection_t *prev_conn = find_connection(prev_list,
                                                    curr_conn->remote_addr,
                                                    curr_conn->remote_port);

        if (!prev_conn || prev_conn->state != TCP_ESTABLISHED) {
            // Newly established connection, trigger login event
            log_info_app("FTP Monitor: FTP LOGIN detected - Client: %s\n", curr_conn->remote_ip_str);
            g_ftp_monitor.callback(FTP_EVENT_LOGIN, curr_conn->remote_ip_str);
        }
    }

    // Detect disconnected connections (disconnect event)
    for (int i = 0; i < prev_list->count; i++) {
        tcp_connection_t *prev_conn = &prev_list->connections[i];

        // Only care about previously established connections
        if (prev_conn->state != TCP_ESTABLISHED) {
            continue;
        }

        // Check if this connection still exists in the current scan and is established
        tcp_connection_t *curr_conn = find_connection(curr_list,
                                                    prev_conn->remote_addr,
                                                    prev_conn->remote_port);

        if (!curr_conn || curr_conn->state != TCP_ESTABLISHED) {
            // Connection disconnected, trigger disconnect event
            log_info_app("FTP Monitor: FTP LOGOUT detected - Client: %s\n", prev_conn->remote_ip_str);
            g_ftp_monitor.callback(FTP_EVENT_LOGOUT, prev_conn->remote_ip_str);
        }
    }
}

/**
 * @brief FTP monitor thread function
 *
 * @param arg Thread arguments (unused)
 * @return void* Thread return value
 */
static void* ftp_monitor_thread(void *arg)
{
    log_info_app("FTP Monitor: Monitor thread started\n");

    // First scan, initialize connection list
    pthread_mutex_lock(&g_ftp_monitor.mutex);
    parse_proc_net_tcp(&g_ftp_monitor.prev_connections);
    pthread_mutex_unlock(&g_ftp_monitor.mutex);

    while (!g_ftp_monitor.thread_exit) {
        // Wait for 1 second
        sleep(1);

        if (g_ftp_monitor.thread_exit) {
            break;
        }

        pthread_mutex_lock(&g_ftp_monitor.mutex);

        // Scan current connection status
        if (parse_proc_net_tcp(&g_ftp_monitor.curr_connections) == 0) {
            // Detect connection changes
            detect_ftp_events(&g_ftp_monitor.prev_connections, &g_ftp_monitor.curr_connections);

            // Update last connection list
            memcpy(&g_ftp_monitor.prev_connections, &g_ftp_monitor.curr_connections,
                   sizeof(connection_list_t));
        }

        pthread_mutex_unlock(&g_ftp_monitor.mutex);
    }

    log_info_app("FTP Monitor: Monitor thread exited\n");
    return NULL;
}

/**
 * @brief Initialize FTP monitor module
 *
 * @param callback FTP event callback function
 * @return int 0 success, -1 failure
 */
int ftp_monitor_init(ftp_event_callback_t callback)
{
    if (!callback) {
        log_error_app("FTP Monitor: Invalid callback function\n");
        return -1;
    }

    if (g_ftp_monitor.running) {
        log_error_app("FTP Monitor: Already initialized\n");
        return -1;
    }

    // Initialize monitor structure
    memset(&g_ftp_monitor, 0, sizeof(ftp_monitor_t));
    g_ftp_monitor.callback = callback;

    // Initialize mutex
    if (pthread_mutex_init(&g_ftp_monitor.mutex, NULL) != 0) {
        log_error_app("FTP Monitor: Failed to initialize mutex\n");
        return -1;
    }

    log_info_app("FTP Monitor: Initialized successfully\n");
    return 0;
}

/**
 * @brief Start FTP monitor
 *
 * @return int 0 success, -1 failure
 */
int ftp_monitor_start(void)
{
    if (!g_ftp_monitor.callback) {
        log_error_app("FTP Monitor: Not initialized\n");
        return -1;
    }

    if (g_ftp_monitor.running) {
        log_error_app("FTP Monitor: Already running\n");
        return -1;
    }

    g_ftp_monitor.thread_exit = 0;

    // Create monitor thread
    if (pthread_create(&g_ftp_monitor.monitor_thread, NULL, ftp_monitor_thread, NULL) != 0) {
        log_error_app("FTP Monitor: Failed to create monitor thread\n");
        return -1;
    }

    g_ftp_monitor.running = 1;
    log_info_app("FTP Monitor: Started successfully\n");
    return 0;
}

/**
 * @brief Stop FTP monitor
 */
void ftp_monitor_stop(void)
{
    if (!g_ftp_monitor.running) {
        log_error_app("FTP Monitor: Not running\n");
        return;
    }

    log_info_app("FTP Monitor: Stopping...\n");

    // Set thread exit flag
    g_ftp_monitor.thread_exit = 1;

    // Wait for monitor thread to end
    if (pthread_join(g_ftp_monitor.monitor_thread, NULL) != 0) {
        log_error_app("FTP Monitor: Failed to join monitor thread\n");
    }

    g_ftp_monitor.running = 0;
    log_info_app("FTP Monitor: Stopped successfully\n");
}

/**
 * @brief Cleanup FTP monitor module resources
 */
void ftp_monitor_cleanup(void)
{
    if (g_ftp_monitor.running) {
        ftp_monitor_stop();
    }

    // Destroy mutex
    pthread_mutex_destroy(&g_ftp_monitor.mutex);

    // Clear structure
    memset(&g_ftp_monitor, 0, sizeof(ftp_monitor_t));

    log_info_app("FTP Monitor: Cleanup completed\n");
}

/**
 * @brief Check if FTP monitor is running
 *
 * @return int 1 running, 0 not running
 */
int ftp_monitor_is_running(void)
{
    return g_ftp_monitor.running;
}

/**
 * @brief Get current FTP connection count
 *
 * @return int Current connection count
 */
int ftp_monitor_get_connection_count(void)
{
    int count = 0;

    pthread_mutex_lock(&g_ftp_monitor.mutex);

    // Count established FTP connections
    for (int i = 0; i < g_ftp_monitor.curr_connections.count; i++) {
        if (g_ftp_monitor.curr_connections.connections[i].state == TCP_ESTABLISHED) {
            count++;
        }
    }

    pthread_mutex_unlock(&g_ftp_monitor.mutex);

    return count;
}
/**
 * @brief FTP event callback function
 *
 * @param event FTP event type
 * @param client_ip Client IP address
 */
void ftp_event_callback(ftp_event_type_t event, const char* client_ip)
{
    int result = 0;
    switch (event) {
        case FTP_EVENT_LOGIN:
            log_info_app("=== FTP client login ===\n");
            log_info_app("Client IP: %s\n", client_ip);
            log_info_app("login time: %s\n", ctime(&(time_t){time(NULL)}));
            log_info_app("==================\n");
            result = 1;
            // set_glass_info_v3(isp_quick_fd,CMD_FTP_LOGIN_NOTIFY,(char *)&result,sizeof(result));
            //set_glass_info_v4_notify(glass_fd,CMD_FTP_LOGIN_NOTIFY,(char *)&result,sizeof(result));
            break;

        case FTP_EVENT_LOGOUT:
            log_info_app("=== FTP client logout ===\n");
            log_info_app("Client IP: %s\n", client_ip);
            log_info_app("logout time: %s\n", ctime(&(time_t){time(NULL)}));
            log_info_app("==================\n");
            result = 1;
            // set_glass_info_v3(isp_quick_fd,CMD_FTP_LOGOUT_NOTIFY,(char *)&result,sizeof(result));
            //set_glass_info_v4_notify(glass_fd,CMD_FTP_LOGOUT_NOTIFY,(char *)&result,sizeof(result));
            break;

        default:
            log_error_app("FTP Monitor: Unknown event type: %d\n", event);
            break;
    }
}
