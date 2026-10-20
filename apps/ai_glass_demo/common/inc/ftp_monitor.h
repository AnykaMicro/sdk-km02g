/* SPDX-License-Identifier: Apache-2.0 */
/**
 * @file ftp_monitor.h
 * @brief FTP Connection Monitoring Module Header File
 * @version 1.0
 * @date 2025-09-04
 *
 * @copyright Copyright (c) 2025 Guangzhou Anyka Microelectronics Co., Ltd.
 *
 */

#ifndef __FTP_MONITOR_H__
#define __FTP_MONITOR_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 * @brief FTP event type enumeration
 */
typedef enum {
    FTP_EVENT_LOGIN = 0,    /**< FTP user login event */
    FTP_EVENT_LOGOUT        /**< FTP user logout event */
} ftp_event_type_t;



/**
 * @brief TCP connection state enumeration
 */
typedef enum {
    TCP_ESTABLISHED = 0x01, /**< Connection established */
    TCP_SYN_SENT = 0x02,    /**< SYN sent */
    TCP_SYN_RECV = 0x03,    /**< SYN received */
    TCP_FIN_WAIT1 = 0x04,   /**< FIN wait 1 */
    TCP_FIN_WAIT2 = 0x05,   /**< FIN wait 2 */
    TCP_TIME_WAIT = 0x06,   /**< TIME wait */
    TCP_CLOSE = 0x07,       /**< Closed */
    TCP_CLOSE_WAIT = 0x08,  /**< Close wait */
    TCP_LAST_ACK = 0x09,    /**< Last ACK */
    TCP_LISTEN = 0x0A,      /**< Listening */
    TCP_CLOSING = 0x0B      /**< Closing */
} tcp_state_t;


/**
 * @brief TCP connection information structure
 */
typedef struct {
    uint32_t local_addr;        /**< Local address */
    uint16_t local_port;        /**< Local port */
    uint32_t remote_addr;       /**< Remote address */
    uint16_t remote_port;       /**< Remote port */
    tcp_state_t state;          /**< Connection state */
    char remote_ip_str[16];     /**< Remote IP string */
} tcp_connection_t;


/**
 * @brief Connection list structure
 */
#define MAX_FTP_CONNECTIONS 32
typedef struct {
    tcp_connection_t connections[MAX_FTP_CONNECTIONS];
    int count;                  /**< Number of connections */
} connection_list_t;


/**
 * @brief FTP event callback function type
 *
 * @param event FTP event type (connect, disconnect, upload, download, etc.)
 * @param client_ip Client IP address string
 */
typedef void (*ftp_event_callback_t)(ftp_event_type_t event, const char* client_ip);

/**
 * @brief Initialize the FTP monitoring module
 *
 * @param callback FTP event callback function to handle FTP events
 * @return 0 on success, -1 on failure
 */
int ftp_monitor_init(ftp_event_callback_t callback);

/**
 * @brief Start FTP monitoring
 *
 * @return 0 on success, -1 on failure
 */
int ftp_monitor_start(void);

/**
 * @brief Stop FTP monitoring
 */
void ftp_monitor_stop(void);

/**
 * @brief Clean up FTP monitoring module resources
 */
void ftp_monitor_cleanup(void);

/**
 * @brief Check if FTP monitoring is currently running
 *
 * @return 1 if running, 0 if not running
 */
int ftp_monitor_is_running(void);

/**
 * @brief Get current number of FTP connections
 *
 * @return Current connection count
 */
int ftp_monitor_get_connection_count(void);

/**
 * @brief FTP event callback function to handle various events in FTP service.
 *
 * @param event Event type such as connect, disconnect, upload, download, etc.
 * @param client_ip Client IP address (optional, valid when event is related to client)
 *
 * @note This function is automatically called by the FTP module when events occur.
 *       Users need to implement the specific logic.
 */
void ftp_event_callback(ftp_event_type_t event, const char* client_ip);



#ifdef __cplusplus
}
#endif

#endif /* __FTP_MONITOR_H__ */
