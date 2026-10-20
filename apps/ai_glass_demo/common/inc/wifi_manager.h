/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd. */
#ifndef __WIFI_MANAGER_H__
#define __WIFI_MANAGER_H__

#define P2P_SH_PATH "/fast/usr/sbin/app_go_group.sh"
#define AP_SH_PATH "/fast/usr/sbin/wifi_driver.sh"

/**
 * @brief WiFi operation modes.
 */
typedef enum
{
    WIFI_MODE_AP = 10,  /**< Access Point Mode */
    WIFI_MODE_P2P = 11, /**< Peer-to-Peer Mode */
    WIFI_MODE_STA = 12, /**< Station Mode */
} ak_wifi_mode_e;

/**
 * @brief WiFi error codes.
 */
typedef enum {
    WIFI_SUCCESS = 0,                 /**< Operation successful */
    WIFI_ERROR_TIMEOUT = 1,           /**< Connection timed out */
    WIFI_ERROR_WRONG_PASSWORD = 2,    /**< Incorrect password */
    WIFI_ERROR_ASSOC_REJECTED = 3,    /**< Association rejected */
    WIFI_ERROR_NETWORK_NOT_FOUND = 4, /**< Network SSID not found */
    WIFI_ERROR_UNKNOWN = 5            /**< Unknown error */
} wifi_error_code_e;

/**
 * @brief Set WiFi STA configuration
 * @param ssid SSID for STA connection
 * @param psk Pre-shared key/password
 * @return 0 on success, -1 on failure
 */
int set_wifi_sta_conf(char *ssid, char *psk);

/**
 * @brief Set WiFi AP configuration
 * @param ssid SSID for AP mode
 * @param wpa_passphrase WPA passphrase for AP
 * @return 0 on success, -1 on failure
 */
int set_wifi_ap_conf(char *ssid, char *wpa_passphrase);

/**
 * @brief Start P2P mode with specified name
 * @param p2p_name P2P device name
 * @return 0 on success, -1 on failure
 */
int start_p2p(char *p2p_name);

/**
 * @brief Initialize WiFi driver only
 * @return 0 on success, -1 on failure
 */
int init_wifi_driver_only();

/**
 * @brief Start WiFi in AP mode
 * @return 0 on success, -1 on failure
 */
int start_ap();

/**
 * @brief Start WiFi in STA mode
 * @return 0 on success, -1 on failure
 */
int start_sta();

/**
 * @brief Stop WiFi completely
 * @return 0 on success, -1 on failure
 */
int stop_wifi();

/**
 * @brief Search for specified SSID
 * @param ssid SSID to search for
 * @return 0 on success, -1 on failure
 */
int wifi_try_search_ssid(char *ssid);

/**
 * @brief Get current WiFi status, SSID and password
 * @param status_out Output parameter for WiFi status
 * @param ssid_out Output buffer for SSID
 * @param password_out Output buffer for password
 * @return 0 on success, -1 on failure
 */
int wifi_get_status_ssid_password(int *status_out, char *ssid_out, char *password_out);

/**
 * @brief Get the Wi-Fi IPv4 address (wlan0, else p2p0)
 * @param ip_out Output buffer, at least 16 bytes
 * @return 0 on success, -1 on failure
 */
int wifi_get_ip(char *ip_out);


#endif
