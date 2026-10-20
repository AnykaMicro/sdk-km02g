/* SPDX-License-Identifier: Apache-2.0 */
/**
 * @file main.c
 * @brief ai glass demo - multi-threaded version
 * @version 0.2
 * @date 2025-08-05
 *
 * @copyright Copyright (c) 2025 Guangzhou Anyka Microelectronics Co., Ltd.
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>
#include <getopt.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <sys/time.h>
#include <time.h>
#include <linux/netlink.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "media_configs.h"
#include "ak_common.h"
#include "ak_log.h"

#include "isp_quick_ioctl.h"

#include "ak_its.h"
#include "sdnand_manager.h"
#include "wifi_manager.h"
//#include "y_gyro.h"
#include "ak_eis.h"
#include "ai_glass_cmd_ipi.h"
#include "ai_glass_isp_quick.h"

#include "ak_snap_demo.h"
#ifdef RECORD_SUPPORT_ENABLE
#include "ak_record_demo.h"
#endif

#ifdef RTSP_SUPPORT_ENABLE
#include "ak_rtsp_demo.h"
#endif

#ifdef EIS_ENABLED
#include "video_eis.h"
#endif

#ifdef SOUND_REC_SUPPORT_ENABLE
#include "ak_sound_demo.h"
#endif


#include "ftp_monitor.h"
#include "ai_glass_time.h"
#include "ai_glass_common.h"
#include "ai_glass_cmd_handle.h"

#include "fun_log.h"
#include "media_configs.h"

int ai_glass_ipi_cmd_get(quick_glass_info_t *glass_info);
void ai_glass_ipi_cmd_completed(quick_glass_info_t *glass_info);

#define SDK_SW_VERSION_MAJOR            1
#define SDK_SW_VERSION_MINOR            0
#define SDK_SW_VERSION_PATCH            3

static int worker_thread_exit = 0;
int ota_run_flag = 0;

#define SIG_GLASS    (SIGRTMIN+3)

static int wait_config_enable = 0;

ai_glass_timer_t timer;

ai_glass_version_t sw_version;
char sw_buffer[17] = {0};

void ai_glass_print_sw_version(void)
{
    printf("=============================================\n");
    memset(sw_buffer, 0, sizeof(sw_buffer));  // Use sizeof for safety
    snprintf(sw_buffer, sizeof(sw_buffer), "V%d.%02d.%02d",
             SDK_SW_VERSION_MAJOR,
             SDK_SW_VERSION_MINOR,
             SDK_SW_VERSION_PATCH);
    printf("Software version: %s\n", sw_buffer);
    printf("=============================================\n");
}

static quick_glass_info_t glass_info = {0};

// Thread synchronization variables
static pthread_mutex_t cmd_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cmd_cond = PTHREAD_COND_INITIALIZER;

// Sync/Async mode control flag
// 0: Sync mode - command_thread_func must wait for worker_thread_func to finish before fetching next command
// 1: Async mode - Queue mode async execution
static int execution_mode_flag = 0; // Default is async mode



// Thread handles
static pthread_t worker_thread;
static pthread_t usb_thread;

// Program exit flag
static volatile int program_exit = 0;


/**
 * @brief Signal handler
 *
 * @param sig
 */
static void signal_handler(int sig)
{
    program_exit = 1;
//  command_thread_exit = 1;
    worker_thread_exit = 1;

//    rtsp_thread_exit = 1;
//    record_thread_exit = 1;
//    pthread_cond_broadcast(&cmd_cond);
}

/**
 * @brief open NETLINK, monitor NETLINK_KOBJECT_UEVENT
 *
 * @param arg
 * @return int
 */
static int init_hotplug_sock(void) {
     struct sockaddr_nl snl;
     int buffersize = 1024;
     int ret;

     bzero(&snl, sizeof(struct sockaddr_nl));
     snl.nl_family = AF_NETLINK;
     snl.nl_pid = getpid();
     snl.nl_groups = 1;

     int s = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
     if(s == -1) return -1;

     setsockopt(s, SOL_SOCKET, SO_RCVBUF, &buffersize, sizeof(buffersize));
     ret = bind(s, (struct sockaddr *)&snl, sizeof(struct sockaddr_nl));
     if(ret < 0) {
         close(s);
         log_info_app("%s ret:%d\n",__func__, ret);
         return -1;
     }
     return s;
 }



int hotplug_sock = 0;
int usbdetect_fun()
{
	 //??system("/usr/sbin/usb_udc_g.sh install");

     hotplug_sock = init_hotplug_sock();
     char buf[1024];
     int drv_installed = 0;
     int fd = -1, usb_status = USB_MODE_CHARGE;
     ai_glass_timer_t usbtimer;
     double duration_ms_usb;

     log_info_app("hotplug_sock:%d\n",hotplug_sock);
     while(hotplug_sock>0) {
        recv(hotplug_sock, &buf, 1024, 0);
        if( strstr(buf, "storage") )
        {
             log_info_app("usb event: %s\n",buf);
             drv_installed = 1;
             break;
        }
     }

     ai_glass_timer_start(&usbtimer);
     fd = open("/sys/class/udc/c0a50000.usb/state", O_RDONLY);
     while(fd>0 && drv_installed) {
         lseek(fd, 0, SEEK_SET);
         memset(buf, 0, sizeof(buf));
         read(fd, buf, sizeof(buf));
         //log_info_app("gadget state:%s\n", buf);
         if( strstr(buf, "configured") )
         {
             log_info_app("current mass storage mode\n");
             usb_status = USB_MODE_STROGE;
             break;
         }
         duration_ms_usb = ai_glass_timer_stop(&usbtimer);
         if ( duration_ms_usb > 2000.0f ) //>2s timeout exit
         {
            log_info_app("current : %s\n", buf);
            usb_status = USB_MODE_CHARGE;
            break;
         }
         usleep(20000);
     }

     if(fd>0)
     close(fd);
     duration_ms_usb = ai_glass_timer_stop(&usbtimer);
     log_info_app("usb_storage_status :%d, cost time:%.3f ms\n", usb_status, duration_ms_usb);
     //set_glass_info_v4_notify(glass_fd,CMD_USB_MASS_STORAGE,(char *)&usb_status,sizeof(usb_status));

     return usb_status;
 }



void ai_glass_demo_exit(void)
{
    program_exit = 1;
}


/**
 * @brief ak sdk first init
 *
 * @return int
 */
static int ak_sdk_env_init()
{
    int ret = 0;
    SDK_RUN_CONFIG_S stConfig = {0};
    stConfig.s32MemTraceFlag = SDK_RUN_NORMAL;
    stConfig.s32DmaMemTraceFlag = 0;
    // stConfig.s32IspToolServerFlag = 1;

    ret = AK_BASE_SdkInit(&stConfig);
    AK_BASE_PrintSetLevel(MODULE_ID_ALL,LOG_LEVEL_ERROR);
    AK_EXT_ITS_Start(8765);
    return ret;
}



int get_usb_remove(void)
{
    char buf[1024];
    int drv_installed = 0;
    int fd = -1;
    int result = 0;

    //log_info_app("hotplug_sock--llll:%d\n",hotplug_sock);
    while(hotplug_sock>0) {
       recv(hotplug_sock, &buf, 1024, 0);
       //log_info_app("buf: %s\n", buf);
       if( strstr(buf, "storage") )
       {
            log_info_app("usb event: %s\n",buf);
            drv_installed = 1;
            break;
       }
    }

    fd = open("/sys/class/udc/c0a50000.usb/state", O_RDONLY);
    while(fd>0 && drv_installed) {
        lseek(fd, 0, SEEK_SET);
        memset(buf, 0, sizeof(buf));
        read(fd, buf, sizeof(buf));
        //printf("gadget state:%s\n", buf);
           if( !strstr(buf, "configured") )
           {
               log_info_app("current : %s\n", buf);
               result = 1;
               set_work_mode(WORK_MODE_IDLE);
               break;
           }
        usleep(20000);
    }

    if(fd>0)
    close(fd);
    return result;
}

void wait_for_config_ok(void)
{
    while(1)
      {
          usleep(5*1000);
          if(wait_config_enable)
          {
            break;
          }
      }
}

/**
 * @brief Worker thread function
 *
 * @param arg
 * @return void*
 */
static void *worker_thread_func(void *arg)
{
    int ret = 0;
    uint8_t glass_info_param[256] = {0};
    uint16_t glass_info_param_len = 0;
    log_info_app("Worker thread started\n");

    while (!worker_thread_exit)
    {
        quick_glass_info_t local_glass_info;

//        if (execution_mode_flag == 1) {
//            // Async mode: Get command from queue
//            pthread_mutex_lock(&cmd_mutex);
//            while (cmd_queue_is_empty() && !worker_thread_exit)
//            {
//                pthread_cond_wait(&cmd_cond, &cmd_mutex);
//            }
//            pthread_mutex_unlock(&cmd_mutex);

//            if (worker_thread_exit)
//            {
//                break;
//            }

//            // Pop command from queue
//            if (cmd_queue_pop(&local_glass_info) != 0) {
//                log_info_app("Worker thread: [ASYNC] Failed to pop command from queue\n");
//                continue;
//            }

//            log_info_app("Worker thread:Worker thread: [ASYNC] Processing command 0x%02x syst=%lld \n", local_glass_info.cmd, get_sys_run_time64());
//        } else {
//            // Sync mode: Get command from shared structure
//            pthread_mutex_lock(&cmd_mutex);

//            // Wait for new command
//            while (!cmd_data.valid && !worker_thread_exit) {
//                pthread_cond_wait(&cmd_cond, &cmd_mutex);
//            }

//            if (worker_thread_exit) {
//                pthread_mutex_unlock(&cmd_mutex);
//                break;
//            }

//            // Copy command data and mark as processed immediately to prevent duplicate processing
//            memcpy(&local_glass_info, &cmd_data.glass_info, sizeof(quick_glass_info_t));
//            log_info_app("Worker thread: [SYNC] Processing command 0x%02x\n", local_glass_info.cmd);

//            // Clear command data immediately to prevent duplicate processing
//            cmd_data.valid = 0;

//            pthread_mutex_unlock(&cmd_mutex);
//        }
        if(ota_run_flag ==1)
        {
            log_info_app("ota mode Worker thread started exit");
            usleep(1000*1000);
            break;
        }
	    ai_glass_ipi_cmd_get(&local_glass_info);

        // Update global parameters
        // unsigned int i_sec = *(uint32_t *)local_glass_info.parameter;
        // AK_U32 u32devid;
        // for (u32devid = 0; u32devid < g_s32DevCnt; u32devid++)
        // {
        //     get_vi_dev_info(isp_quick_fd, u32devid, &qs_dev.dev_info[u32devid]);
        //     get_venc_attr(isp_quick_fd, u32devid, &qs_dev.venc_chn[u32devid]);
        // }

        // unsigned int i_fps = pdev->dev_info[VIDEO_DEV0].frame_rate;
        // unsigned int i_frame_depth = pdev->dev_info[VIDEO_DEV0].main_chn_buf;
        // log_info_app("Worker thread: i_fps:%u i_frame_depth:%u\n",
        //                 i_fps, i_frame_depth);

        // Execute corresponding operation
        uint8_t result = 1;
        if (local_glass_info.cmd == APP_CMD_P2P
            || local_glass_info.cmd == APP_CMD_AP
            //|| local_glass_info.cmd == BT_CMD_WIFI_STA
            //|| local_glass_info.cmd == APP_CMD_WIFI_STA
            )
        {
            // move to glass_main.sh
            wait_for_usr_file_ready(P2P_SH_PATH);
            //run ftp
            //system("killall -9 ftp_server_riscv;/usr/bin/ftp_server_riscv -p 21 -u root -w \"\" -d / -s 1 -a 0 &");
        }

        if (local_glass_info.cmd == BT_CMD_RECORD
            || local_glass_info.cmd == BT_CMD_RECORD_AUDIO
            || local_glass_info.cmd == APP_CMD_RECORD
            || local_glass_info.cmd == APP_CMD_RECORD_AUDIO)
        {
            wait_for_sdnand(local_glass_info.cmd);
            sd_nand_file_path_init(SD_NAND_DEVICE_PATH);
        }

        switch (local_glass_info.cmd)
        {
        case BT_CMD_PHOTO:
        case APP_CMD_PHOTO:
            wait_for_config_ok();
            handle_take_photo(&local_glass_info);
            break;
#if 0//def RTSP_SUPPORT_ENABLE
         case CMD_START_RTSP:
            handle_start_rtsp(&local_glass_info);
            break;
         case CMD_STOP_RTSP:
            handle_stop_rtsp(&local_glass_info);
            break;
#endif
#ifdef RECORD_SUPPORT_ENABLE
        case BT_CMD_RECORD:
        case APP_CMD_RECORD:
            handle_start_video_record(&local_glass_info);
            break;
        case BT_CMD_STOP_RECORD:
        case APP_CMD_STOP_RECORD:
            handle_stop_video_record(&local_glass_info);
            break;
#endif
        case APP_CMD_P2P:
            handle_wifi_p2p_mode(&local_glass_info);
            break;
        case BT_CMD_POWER_OFF_REQ:
        case APP_CMD_POWER_OFF_REQ:
            handle_power_off(&local_glass_info);
            break;
        case APP_CMD_AP:
            handle_wifi_ap_mode(&local_glass_info);
            break;
        #if 0
        case CMD_SET_TIME:
            handle_set_time(&local_glass_info);
            break;
        case CMD_SET_TIMEZONE:
            handle_set_timezone(&local_glass_info);
            break;
        #endif
        case BT_CMD_SET_LOCAL_TIME:
            handle_set_local_time(&local_glass_info);
            break;
        case APP_CMD_GET_STORAGE_FILE_COUNT:
            handle_get_storage_file_count(&local_glass_info);
            break;
#ifdef SOUND_REC_SUPPORT_ENABLE
        case BT_CMD_RECORD_AUDIO:
        case APP_CMD_RECORD_AUDIO:
            handle_start_audio_record(&local_glass_info);
            break;
        case BT_CMD_STOP_AUDIO:
        case APP_CMD_STOP_AUDIO:
            handle_stop_audio_record(&local_glass_info);
            break;
#endif
        //case CMD_POWER_ON_NOTIFY:
        //{
            // Power on notification todo
        //    break;
        //}
        case APP_CMD_RCOVERY_FACTORY:
            handle_recovert_factory(&local_glass_info);
            break;
        //case CMD_WIFI_STA:
        //{
        //    log_info_app("CMD_WIFI_STA init wifi driver\n");
        //    result = init_wifi_driver_only();
        //    set_glass_info_v4(glass_fd,local_glass_info.cmd,(char *)&result,sizeof(int));
        //    break;
        //}
       // case CMD_WIFI_STA_CONNECT:
       //     handle_wifi_sta_connect(&local_glass_info);
        //    break;
        //case CMD_WIFI_STOP:
        //    handle_wifi_stop(&local_glass_info);
       //     break;
        //case CMD_WIFI_STA_GET_STATUS:
        //    handle_get_sta_wifi_status(&local_glass_info);
        //    break;
       // case CMD_WIFI_STA_GET_IP:
       //     handle_get_wifi_ip(&local_glass_info);
       //     break;
        case APP_CMD_GET_VERSION:
            result = RETURN_CODE_SUCCESS;
            memset(glass_info_param, 0, sizeof(glass_info_param));
            glass_info_param_len = ai_glass_info_pack_data(result,glass_info_param,sw_buffer,strlen(sw_buffer)+1);
            set_glass_info_v4(local_glass_info.cmd,(char*)glass_info_param,glass_info_param_len);
            break;
       // case CMD_GET_NUM_IMG_VIDEO:

        //case APP_CMD_GET_PHOTO_VIDEO_COUNT:
        //    handle_get_num_img_video(&local_glass_info);
        //    break;
        //case CMD_UMONT_SDNAND:
        //    handle_power_off(&local_glass_info);
        //    break;
        case APP_CMD_OTA:
            handle_ota(&local_glass_info);
            break;
        //case CMD_GET_STATUS:
        //    handle_get_work_mode(&local_glass_info);
        //    break;
        case BT_CMD_USB_CONFIG:
			handle_usb_mode_set(&local_glass_info);
			break;
        case APP_CMD_P2P_DICONN:
            log_info_app("Worker thread: APP_CMD_P2P_DICONN\n");
            set_work_mode(WORK_MODE_IDLE);
            result = RETURN_CODE_SUCCESS;
            memset(glass_info_param, 0, sizeof(glass_info_param));
            glass_info_param_len = ai_glass_info_pack_data(result,glass_info_param,NULL,0);
            set_glass_info_v4(local_glass_info.cmd,(char*)glass_info_param,glass_info_param_len);
            break;
        case APP_CMD_GET_STORAGE:
            handle_get_storage(&local_glass_info);
            break;

        default:
            log_info_app("Worker thread: UNSUPPORT CMD\n");
            result = RETURN_CODE_UNKNOWN_ERROR;
            memset(glass_info_param, 0, sizeof(glass_info_param));
            glass_info_param_len = ai_glass_info_pack_data(result,glass_info_param,NULL,0);
            set_glass_info_v4(local_glass_info.cmd, (char*)glass_info_param,glass_info_param_len);
            break;
        }

//        // Notify command_thread that command processing is complete in sync mode
//        if (execution_mode_flag == 0) {
//            pthread_mutex_lock(&cmd_mutex);
//            cmd_data.processed = 1;
//            log_info_app("Worker thread: [SYNC] Command 0x%02x processing completed, notifying command thread (syst=%lld)\n", local_glass_info.cmd, get_sys_run_time64());
//            pthread_cond_signal(&cmd_cond);
//            pthread_mutex_unlock(&cmd_mutex);
//        }

//        log_info_app("Worker thread: Command processed, ready for next command\n");
		ai_glass_ipi_cmd_completed(&local_glass_info);
    }

    log_info_app("Worker thread exited\n");
    return NULL;
} /* end of func */
// Thread function: Wait for /etc/config directory to be accessible then load configuration
static void *wait_and_load_config(void *arg)
{
    struct stat st;
    const char *config_path = "/etc/config/app_main_config.ini";
    int max_attempts = 300;  // try 300
    int attempt = 0;
    wait_config_enable = 0;
    log_info_app("wait %s ...\n", config_path);

    while (attempt < max_attempts) {
        if (access(config_path, F_OK) == 0){
            log_info_app("%s mount ok!!!\n", config_path);
            print_timer_cost(__FUNCTION__,__LINE__);
            load_configuration();
            set_configuration();
            load_camera_configuration();
            load_eis_configuration();
            wait_config_enable = 1;
            break;
        }

        attempt++;
        log_info_app("try %d/%d: %s can not use ,wait 10ms...\n", attempt, max_attempts, config_path);
        usleep(10*1000);
    }

    if (attempt >= max_attempts) {
        load_configuration();
        load_camera_configuration();
    }

    return NULL;
}


#define __FASTSYS__ 1

int main(int argc, char *argv[])
{
    int ret = 0;
    pthread_t config_thread;

    // sd_nand_file_path_init(SD_NAND_DEVICE_PATH);
    ai_glass_log_init();
    printf("main:AK GLASS [Compiled time:%s %s][syst=%lld]\n",
           __DATE__, __TIME__, get_sys_run_time64());
    ai_glass_print_sw_version();
    init_global_time();

    // Create thread to wait for /etc/config directory and load configuration
    ret = pthread_create(&config_thread, NULL, wait_and_load_config, NULL);
    if (ret != 0) {
        log_error_app("wait_and_load_config fail\n");
    }

#if __FASTSYS__
    ai_glass_ipi_cmd_sem_init();

    ret = ai_glass_ipi_cmd_fd_open();
    if (ret != 0)
    {
        log_error_app("open ak_glass device fail, run on normal mode\n");
        return -1;
    }
#endif

#if 0
    struct sigaction sa;
    sa.sa_sigaction = glass_handler;
    sa.sa_flags = SA_SIGINFO;// | SA_RESTART;
    sigemptyset(&sa.sa_mask);
    ret = sigaction(SIG_GLASS, &sa, NULL);
    printf("sigaction SIG_GLASS-%d ret:%d\n", SIG_GLASS, ret);
#else
    //signal(SIGIO, glass_sig);
	ai_glass_ipi_cmd_sem_signal();
#endif

#if __FASTSYS__
//    ret = fcntl(glass_fd, F_GETFL);
//    //printf("fcntl F_GETFL ret:%d\n", ret);
//    ret = fcntl(glass_fd, F_SETFL, ret | O_ASYNC);
//    //printf("fcntl F_SETFL ret:%d\n", ret);
//    ret = fcntl(glass_fd, F_SETOWN, getpid());
//    //printf("fcntl F_SETOWN ret:%d\n", ret);
//    //ret = fcntl(glass_fd, F_SETSIG, SIGIO);(If custom command, do not uncomment)
//    //printf("fcntl F_SETSIG SIG_GLASS:%d, SIGUSR1:%d, ret:%d\n", SIG_GLASS, SIGUSR1, ret);

//	quick_glass_info_t glass_info = {0};
//    ioctl(glass_fd, NOTIFY_USER_READY, &glass_info);
	ai_glass_ipi_cmd_ready();
#endif
    // Set signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

#if __FASTSYS__
    ai_glass_isp_quick_init();
#endif

    // // Initial command fetch to get device info
    // if (get_glass_info(isp_quick_fd) != 0) {
    //     printf("Failed to get initial glass info\n");
    //     close(isp_quick_fd);
    //     return -1;
    // }

    // // Get device info
    // AK_U32 u32devid;
    // for(u32devid = 0; u32devid < g_s32DevCnt; u32devid++)
    // {
    //     get_vi_dev_info(isp_quick_fd, u32devid, &qs_dev.dev_info[u32devid]);
    //     get_venc_attr(isp_quick_fd, u32devid, &qs_dev.venc_chn[u32devid]);
    // }

    // sdk init
    ret = ak_sdk_env_init();
    if (ret != 0)
    {
        log_error_app("Failed to initialize SDK environment\n");
        ai_glass_isp_quick_deinit();
        return -1;
    }

//    if(execution_mode_flag == 1)
//    {
//        // Initialize command queue
//        cmd_queue_init();
//    }
//    else
//    {
//        // Initialize thread synchronization variables
//        pthread_mutex_init(&cmd_mutex, NULL);
//        pthread_cond_init(&cmd_cond, NULL);
//    }

//    // Print current execution mode
//    log_info_app("Execution mode: %s (flag=%d)\n",
//           execution_mode_flag ? "ASYNC (Queue)" : "SYNC (Wait)",
//           execution_mode_flag);

    ret = ai_glass_ipi_cmd_init();
    if (ret != 0)
    {
        log_error_app("Failed to create command thread: %d\n", ret);
        goto cleanup;
    }

    // Create worker thread
    ret = pthread_create(&worker_thread, NULL, worker_thread_func, NULL);
    if (ret != 0)
    {
        log_info_app("Failed to create worker thread: %d\n", ret);
//        command_thread_exit = 1;
//        pthread_join(command_thread, NULL);
		ai_glass_ipi_cmd_deinit();
        goto cleanup;
    }

    // send_linux_ready_notify();

    // Initialize and start FTP monitor
    //printf("Initializing FTP monitor...\n");
//    if (ftp_monitor_init(ftp_event_callback) == 0) {
//        if (ftp_monitor_start() == 0) {
//            log_info_app("FTP monitor started successfully\n");
//        } else {
//            log_error_app("Failed to start FTP monitor\n");
//        }
//    } else {
//        log_error_app("Failed to initialize FTP monitor\n");
//    }
    // Program keeps running until exit signal received
    //monitor usb mass storage status thread 2s timeout
    //system("/usr/sbin/usb_udc_g.sh install");
//    ret = pthread_create(&usb_thread, NULL, usbdetect_thread, NULL);
//    if (ret != 0)
//    {
//        log_error_app("Failed to create usb thread: %d\n", ret);
//    }


    while (!program_exit)
    {
        sleep(1);
    }

    log_info_app("Stopping threads...\n");
	handle_cmd_deinit();
	ai_glass_ipi_cmd_deinit();

    // Wait for threads to complete
    pthread_join(worker_thread, NULL);
    pthread_join(usb_thread, NULL);

cleanup:
    // Stop and clean up FTP monitor
//    log_info_app("Stopping FTP monitor...\n");
//    ftp_monitor_stop();
//    ftp_monitor_cleanup();
//    log_info_app("FTP monitor stopped and cleaned up\n");

    // Clean up resources
    worker_thread_exit = 1;
//    command_thread_exit = 1;
//    rtsp_thread_exit = 1;
//    record_thread_exit = 1;

//    pthread_cond_broadcast(&cmd_cond);

//    if(execution_mode_flag == 1)
//    {
//        // Clean up command queue
//        cmd_queue_clear();
//        destroy_queue_mutex();
//    }
//    else
//    {
//        pthread_mutex_destroy(&cmd_mutex);
//        pthread_cond_destroy(&cmd_cond);
//    }

    ai_glass_ipi_cmd_fd_close();
    ai_glass_isp_quick_deinit();

    if(!ota_run_flag){
         // umount sdcard
         sd_nand_umount(SD_NAND_MOUNT_PATH);
         // sd_nand_umount(SD_NAND_MOUNT_PATH);
    }else{
        log_info_app("ota running,do not umount sdnand!\n");
    }

    ak_sdk_env_destory();

    log_info_app("Multi-threaded demo completed\n");
    return 0;
}
