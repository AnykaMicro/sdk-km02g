/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd. */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "fun_log.h"
#include "wifi_manager.h"

static volatile int wifi_working_mode = -1; //0:ap 1:p2p 2:sta -1:stop

/* Values from the phone end up inside config files: refuse anything that could end a line or a quoted string */
static int conf_value_ok(const char *s, int quoted)
{
    for (; *s; s++)
        if ((unsigned char)*s < 0x20 || *s == 0x7f || (quoted && *s == '"'))
            return 0;
    return 1;
}

int set_wifi_sta_conf(char *ssid, char *psk)
{
    const char *config_file = "/etc/config/wpa_supplicant.conf";
    const char *temp_file = "/etc/config/wpa_supplicant.conf.tmp";
    const char *backup_file = "/etc/config/wpa_supplicant.conf.bak";
    FILE *fp_write = NULL;
    int ret = 0;
    int is_open_network = 0;

    // Parameter check
    if (!ssid)
    {
        log_error_app("Error: ssid is NULL");
        return -1;
    }

    // Check if it is an open network
    if (!psk || strlen(psk) == 0)
    {
        is_open_network = 1;
        log_info_app("STA ssid:%s (Open Network)", ssid);
    }
    else
    {
        log_info_app("STA ssid:%s (Encrypted Network)", ssid);
    }

    // Check ssid length
    if (strlen(ssid) == 0 || strlen(ssid) > 32)
    {
        log_error_app("Error: Invalid SSID length (must be 1-32 characters)");
        return -1;
    }

    // If it is an encrypted network, check the password length
    if (!is_open_network && (strlen(psk) < 8 || strlen(psk) > 63))
    {
        log_error_app("Error: Invalid WPA psk length (must be 8-63 characters)");
        return -1;
    }

    if (!conf_value_ok(ssid, 1) || (!is_open_network && !conf_value_ok(psk, 1)))
    {
        log_error_app("Error: SSID and psk must not contain control characters or '\"'\n");
        return -1;
    }

    // Backup the original configuration file
    if (rename(config_file, backup_file) != 0)
    {
        log_info_app("Warning: Cannot backup config file: %s", strerror(errno));
        // Continue execution, do not return error
    }

    // Open temporary file for writing
    fp_write = fopen(temp_file, "w");
    if (!fp_write)
    {
        log_error_app("Error: Cannot open %s for writing: %s", temp_file, strerror(errno));
        // Try to restore the backup file
        rename(backup_file, config_file);
        return -1;
    }

    // Write configuration file header
    fprintf(fp_write, "ctrl_interface=/var/run/wpa_supplicant\n");
    fprintf(fp_write, "ap_scan=1\n");

    // Write network configuration block
    fprintf(fp_write, "network={\n");
    fprintf(fp_write, "\tssid=\"%s\"\n", ssid);

    if (is_open_network)
    {
        // Open network configuration
        fprintf(fp_write, "\tkey_mgmt=NONE\n");
        fprintf(fp_write, "\tscan_ssid=1\n");
        log_info_app("Configured as open network\n");
    }
    else
    {
        // Encrypted network configuration
        fprintf(fp_write, "\tkey_mgmt=WPA-PSK\n");
        fprintf(fp_write, "\tpairwise=CCMP TKIP\n");
        fprintf(fp_write, "\tgroup=CCMP TKIP WEP104 WEP40\n");
        fprintf(fp_write, "\tpsk=\"%s\"\n", psk);
        fprintf(fp_write, "\tscan_ssid=1\n");
        log_info_app("Configured as encrypted network\n");
    }

    fprintf(fp_write, "}\n");

    // Close file
    fclose(fp_write);

    // Replace the original file with the temporary file
    if (rename(temp_file, config_file) != 0)
    {
        log_error_app("Error: Cannot replace config file: %s\n", strerror(errno));
        unlink(temp_file); // Delete temporary file
        // Try to restore the backup file
        rename(backup_file, config_file);
        ret = -1;
    }
    else
    {
        printf("WiFi STA configuration updated successfully\n");
        printf("New SSID: %s\n", ssid);
        if (is_open_network)
        {
            printf("Network Type: Open (No Password)\n");
        }
        else
        {
            printf("Network Type: WPA-PSK (Encrypted)\n");
        }
        // Delete backup file
        unlink(backup_file);
        ret = 0;
    }

    return ret;
}

int set_wifi_ap_conf(char *ssid, char *wpa_passphrase)
{
    const char *config_file = "/etc/config/hostapd.conf";
    const char *temp_file = "/etc/config/hostapd.conf.tmp";
    FILE *fp_read = NULL;
    FILE *fp_write = NULL;
    char line[256];
    int ret = 0;

    // Parameter check
    if (!ssid || !wpa_passphrase)
    {
        log_error_app("Error: ssid or wpa_passphrase is NULL\n");
        return -1;
    }
    // Check SSID and password length
    if (strlen(ssid) == 0 || strlen(ssid) > 32)
    {
        log_error_app("Error: Invalid SSID length (must be 1-32 characters)\n");
        return -1;
    }

    if (strlen(wpa_passphrase) < 8 || strlen(wpa_passphrase) > 63)
    {
        log_error_app("Error: Invalid WPA passphrase length (must be 8-63 characters)\n");
        return -1;
    }

    if (!conf_value_ok(ssid, 0) || !conf_value_ok(wpa_passphrase, 0))
    {
        log_error_app("Error: SSID and passphrase must not contain control characters\n");
        return -1;
    }

    // Open original config file for reading
    fp_read = fopen(config_file, "r");
    if (!fp_read)
    {
        log_error_app("Error: Cannot open %s for reading: %s\n", config_file, strerror(errno));
        return -1;
    }

    // Open temp file for writing
    fp_write = fopen(temp_file, "w");
    if (!fp_write)
    {
        log_error_app("Error: Cannot open %s for writing: %s\n", temp_file, strerror(errno));
        fclose(fp_read);
        return -1;
    }

    // Read and process line by line
    while (fgets(line, sizeof(line), fp_read))
    {
        // Remove newline character at end of line
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n')
        {
            line[len - 1] = '\0';
        }

        // Check if it is ssid line
        if (strncmp(line, "ssid=", 5) == 0)
        {
            fprintf(fp_write, "ssid=%s\n", ssid);
            log_info_app("Updated SSID to: %s\n", ssid);
        }
        // Check if it is wpa_passphrase line
        else if (strncmp(line, "wpa_passphrase=", 15) == 0)
        {
            fprintf(fp_write, "wpa_passphrase=%s\n", wpa_passphrase);
            log_info_app("Updated WPA passphrase\n");
        }
        // Other lines remain unchanged
        else
        {
            fprintf(fp_write, "%s\n", line);
        }
    }

    // Close file
    fclose(fp_read);
    fclose(fp_write);

    // Replace the original file with the temporary file
    if (rename(temp_file, config_file) != 0)
    {
        log_error_app("Error: Cannot replace config file: %s\n", strerror(errno));
        unlink(temp_file); // Delete temporary file
        ret = -1;
    }
    else
    {
        log_info_app("WiFi AP configuration updated successfully\n");
        log_info_app("New SSID: %s\n", ssid);
        ret = 0;
    }

    return ret;
}

/* The P2P device name becomes argv[1] of an out-of-tree shell script: allow only [A-Za-z0-9_-], 1-32 chars */
static int p2p_name_ok(const char *s)
{
    size_t n = strlen(s);
    if (n == 0 || n > 32)
        return 0;
    for (; *s; s++)
        if (!((*s >= 'A' && *s <= 'Z') || (*s >= 'a' && *s <= 'z') || (*s >= '0' && *s <= '9') || *s == '_' || *s == '-'))
            return 0;
    return 1;
}

int start_p2p(char *p2p_name)
{
    int result = 0;

    if (!p2p_name || !p2p_name_ok(p2p_name))
    {
        log_error_app("Error: invalid P2P device name (allowed: [A-Za-z0-9_-], 1-32 characters)\n");
        return -1;
    }
    if (wifi_working_mode != -1)
    {
        log_error_app("wifi need stop first!\n");
        return -1;
    }

    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork failed");
        exit(1);
    }
    else if (pid == 0)
    {
        char *args[3];
        args[0] = P2P_SH_PATH;
        args[1] = p2p_name;
        // args[1] = "P2P_DEVICES";
        log_info_app("args[1]:%s len:%zu\n", p2p_name,strlen(p2p_name));
        args[2] = NULL;
        execvp(args[0], args);
        perror("execvp failed");
        exit(1);
    }
    else
    {
        int status;
        waitpid(pid, &status, 0);
        log_info_app("Script execution completed, return:%d\n", WEXITSTATUS(status));
        result = WEXITSTATUS(status);
    }

    if(result == 0)
    {
        wifi_working_mode = WIFI_MODE_P2P;
    }
    else
    {
        log_info_app("quick stop wifi\n");
        system("wifi_driver.sh stop");
    }


    return result;
}

int init_wifi_driver_only()
{
    int result = 0;

    if (wifi_working_mode != -1)
    {
        log_error_app("wifi need stop first!\n");
        return -1;
    }

    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork failed");
        exit(1);
    }
    else if (pid == 0)
    {
        char *args[3];
        args[0] = AP_SH_PATH;
        args[1] = "install";
        args[2] = NULL;
        execvp(args[0], args);
        perror("execvp failed");
        exit(1);
    }
    else
    {
        int status;
        waitpid(pid, &status, 0);
        log_info_app("Script execution completed, return:%d\n", WEXITSTATUS(status));
        result = WEXITSTATUS(status);
    }

    if(result == 0)
        wifi_working_mode = WIFI_MODE_STA;

    return result;
}

int start_ap()
{
    int result = 0;

    if (wifi_working_mode != -1)
    {
        log_error_app("wifi need stop first!\n");
        return -1;
    }

    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork failed");
        exit(1);
    }
    else if (pid == 0)
    {
        char *args[3];
        args[0] = AP_SH_PATH;
        args[1] = "ap";
        args[2] = NULL;
        execvp(args[0], args);
        perror("execvp failed");
        exit(1);
    }
    else
    {
        int status;
        waitpid(pid, &status, 0);
        log_info_app("Script execution completed, return:%d\n", WEXITSTATUS(status));
        result = WEXITSTATUS(status);
    }

    if(result == 0)
    {
        wifi_working_mode = WIFI_MODE_AP;
    }
    else
    {
        log_info_app("quick stop wifi\n");
        system("wifi_driver.sh stop");
    }
    return result;
}

int start_sta()
{
    int result = 0;

    if (wifi_working_mode != WIFI_MODE_STA)
    {
        log_error_app("wifi_working_mode != WIFI_MODE_STA\n");
        return -1;
    }

    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork failed");
        exit(1);
    }
    else if (pid == 0)
    {
        char *args[3];
        args[0] = AP_SH_PATH;
        args[1] = "sta";
        args[2] = NULL;
        execvp(args[0], args);
        perror("execvp failed");
        exit(1);
    }
    else
    {
        int status;
        waitpid(pid, &status, 0);
        result = WEXITSTATUS(status);
        log_info_app("Script execution completed, return:%d\n", result);

        switch(result) {
            case WIFI_SUCCESS:
                log_info_app("WiFi connected successfully\n");
                wifi_working_mode = WIFI_MODE_STA;
                break;
            case WIFI_ERROR_WRONG_PASSWORD:
                log_info_app("WiFi connection failed: Wrong password\n");
                break;
            case WIFI_ERROR_ASSOC_REJECTED:
                log_info_app("WiFi connection failed: Association rejected\n");
                break;
            case WIFI_ERROR_NETWORK_NOT_FOUND:
                log_info_app("WiFi connection failed: Network not found\n");
                break;
            case WIFI_ERROR_TIMEOUT:
                log_info_app("WiFi connection failed: Connection timeout\n");
                break;
            default:
                log_error_app("WiFi connection failed: Unknown error (code: %d)\n", result);
                break;
        }
    }

    return result;
}

int stop_wifi()
{
    int result = 0;

    if (wifi_working_mode == -1)
    {
        log_error_app("wifi need start first!\n");
        return -1;
    }

    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork failed");
        exit(1);
    }
    else if (pid == 0)
    {
        char *args[3];
        args[0] = AP_SH_PATH;
        args[1] = "stop";
        args[2] = NULL;
        execvp(args[0], args);
        perror("execvp failed");
        exit(1);
    }
    else
    {
        int status;
        waitpid(pid, &status, 0);
        log_info_app("Script execution completed, return:%d\n", WEXITSTATUS(status));
        result = WEXITSTATUS(status);
    }

    if(result == 0)
    {
        result = wifi_working_mode;
        wifi_working_mode = -1;
        return result;
    }
    else
    {
        return result;
    }

}


int wifi_try_search_ssid(char *ssid)
{
    FILE *fp;
    char line[256];
    char *essid_start, *essid_end;
    char found_ssid[64];
    int found = 0;

    // Parameter check
    if (!ssid || strlen(ssid) == 0) {
        log_error_app("Error: Invalid SSID parameter\n");
        return -1;
    }

    log_info_app("Searching for SSID: %s\n", ssid);

    // Execute scan command
    fp = popen("iwlist wlan0 scanning 2>/dev/null | grep ESSID", "r");
    if (!fp) {
        log_error_app("Error: Failed to execute iwlist scanning command\n");
        return -1;
    }

    // Parse scan results line by line
    while (fgets(line, sizeof(line), fp)) {
        // Find ESSID:" pattern
        essid_start = strstr(line, "ESSID:\"");
        if (essid_start) {
            essid_start += 7; // Skip "ESSID:\""
            essid_end = strchr(essid_start, '"');
            if (essid_end) {
                // Extract ESSID content
                int len = essid_end - essid_start;
                if (len > 0 && len < sizeof(found_ssid)) {
                    strncpy(found_ssid, essid_start, len);
                    found_ssid[len] = '\0';

                    // Compare SSID
                    if (strcmp(found_ssid, ssid) == 0) {
                        log_info_app("Found matching SSID: %s\n", found_ssid);
                        found = 1;
                        break;
                    }
                }
            }
        }
    }

    pclose(fp);

    if (found) {
        log_info_app("SSID '%s' found in scan results\n", ssid);
        return 1;
    } else {
        log_info_app("SSID '%s' not found in scan results\n", ssid);
        return 0;
    }
}

int wifi_get_status_ssid_password(int *status_out, char *ssid_out, char *password_out)
{
    FILE *fp;
    char line[256];
    char *essid_start, *essid_end;
    char *ap_start;
    char current_ssid[64] = {0};
    char current_password[64] = {0};
    int status = 2; // Default is WiFi stopped status
    int connected = 0;

    // Parameter check
    if (!status_out || !ssid_out || !password_out) {
        log_error_app("Error: Invalid output parameters\n");
        return -1;
    }

    // Initialize output parameters
    *status_out = 2;
    ssid_out[0] = '\0';
    password_out[0] = '\0';

    // Check global WiFi mode status
    if (wifi_working_mode == -1) {
        log_error_app("WiFi is stopped\n");
        *status_out = 2;
        return 0;
    }

    // Execute iwconfig command to get current connection status
    fp = popen("iwconfig wlan0 2>/dev/null", "r");
    if (!fp) {
        log_error_app("Error: Failed to execute iwconfig command\n");
        return -1;
    }

    // Parse iwconfig output
    while (fgets(line, sizeof(line), fp)) {
        // Find ESSID
        essid_start = strstr(line, "ESSID:\"");
        if (essid_start) {
            essid_start += 7; // Skip "ESSID:\""
            essid_end = strchr(essid_start, '"');
            if (essid_end) {
                int len = essid_end - essid_start;
                if (len > 0 && len < sizeof(current_ssid)) {
                    strncpy(current_ssid, essid_start, len);
                    current_ssid[len] = '\0';
                }
            }
        }

        // Find Access Point info to determine if connected
        ap_start = strstr(line, "Access Point:");
        if (ap_start) {
            ap_start += 13; // Skip "Access Point: "
            // Check for valid MAC address (not all zeros or Not-Associated)
            if (strstr(ap_start, "Not-Associated") == NULL &&
                strstr(ap_start, "00:00:00:00:00:00") == NULL &&
                strlen(ap_start) > 10) {
                connected = 1;
            }
        }
    }
    pclose(fp);

    // Determine status based on connection state and SSID
    if (strlen(current_ssid) > 0 && connected) {
        status = 0; // Connected
        strcpy(ssid_out, current_ssid);
        log_info_app("WiFi connected to: %s\n", current_ssid);
    } else if (wifi_working_mode != -1) {
        status = 1; // WiFi started but not connected
        log_info_app("WiFi started but not connected\n");
    }

    // Read password based on WiFi mode
    if (strlen(current_ssid) > 0) {
        const char *config_file = NULL;
        const char *password_pattern = NULL;

        if (wifi_working_mode == WIFI_MODE_STA) {
            config_file = "/etc/config/wpa_supplicant.conf";
            password_pattern = "psk=\"";
        } else if (wifi_working_mode == WIFI_MODE_AP) {
            config_file = "/etc/config/hostapd.conf";
            password_pattern = "wpa_passphrase=";
        }

        if (config_file && password_pattern) {
            FILE *config_fp = fopen(config_file, "r");
            if (config_fp) {
                while (fgets(line, sizeof(line), config_fp)) {
                    // Match the key at line start only: a '#' comment may mention it
                    char *pass_start = line;
                    while (*pass_start == ' ' || *pass_start == '\t')
                        pass_start++;
                    if (*pass_start == '#' || strncmp(pass_start, password_pattern, strlen(password_pattern)) != 0)
                        pass_start = NULL;
                    if (pass_start) {
                        pass_start += strlen(password_pattern);

                        if (wifi_working_mode == WIFI_MODE_STA) {
                            // STA mode: psk="password"
                            char *pass_end = strchr(pass_start, '"');
                            if (pass_end) {
                                int len = pass_end - pass_start;
                                if (len > 0 && len < sizeof(current_password)) {
                                    strncpy(current_password, pass_start, len);
                                    current_password[len] = '\0';
                                }
                            }
                        } else {
                            // AP mode: wpa_passphrase=password
                            char *pass_end = strchr(pass_start, '\n');
                            if (pass_end) {
                                int len = pass_end - pass_start;
                                if (len > 0 && len < sizeof(current_password)) {
                                    strncpy(current_password, pass_start, len);
                                    current_password[len] = '\0';
                                }
                            }
                        }
                        break;
                    }
                }
                fclose(config_fp);
            }
        }
    }

    // Set output parameters
    *status_out = status;
    if (strlen(current_password) > 0) {
        strcpy(password_out, current_password);
    }

    log_info_app("WiFi Status: %d, SSID: %s\n", status, ssid_out);
    return 0;
}

int wifi_get_ip(char *ip_out)
{
    /* wlan0 carries STA and AP mode; p2p0 the P2P group owner */
    static const char *const ifaces[] = { "wlan0", "p2p0" };
    struct ifaddrs *list, *ifa;
    unsigned int i;

    // Parameter check
    if (!ip_out) {
        log_error_app("Error: Invalid IP output parameter\n");
        return -1;
    }
    ip_out[0] = '\0';

    if (getifaddrs(&list) != 0) {
        log_error_app("Error: getifaddrs failed: %s\n", strerror(errno));
        return -1;
    }
    for (i = 0; i < sizeof(ifaces) / sizeof(ifaces[0]) && ip_out[0] == '\0'; i++) {
        for (ifa = list; ifa; ifa = ifa->ifa_next) {
            if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET && strcmp(ifa->ifa_name, ifaces[i]) == 0) {
                /* callers pass at least 16 bytes (INET_ADDRSTRLEN) */
                inet_ntop(AF_INET, &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr, ip_out, INET_ADDRSTRLEN);
                break;
            }
        }
    }
    freeifaddrs(list);

    if (ip_out[0] == '\0') {
        log_error_app("No IPv4 address on wlan0 or p2p0\n");
        return -1;
    }
    log_info_app("Wi-Fi IP address: %s\n", ip_out);
    return 0;
}
