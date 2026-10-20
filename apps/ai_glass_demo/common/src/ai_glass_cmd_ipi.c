/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd. */
#include <signal.h>
#include <semaphore.h>
#include <sys/mman.h>

#include "fun_log.h"
#include "ai_glass_common.h"

#define IPI_MASTER_GLASS_CMD_ACK 0x3
#define IPI_FASTSYS_FILE_OPEN 0x4
#define IPI_FASTSYS_FILE_WRITE 0x5
#define IPI_FASTSYS_FILE_READ 0x6
#define IPI_MASTER_FILE_ACK 0x7
#define IPI_FASTSYS_FILE_SEEK 0x8
#define IPI_FASTSYS_FILE_CLOSE 0x9

#define IPI_FASTSYS_FILE_OPERATE    0x1000

#define APP_OPERATE 0x80
#define is_app_cmd(cmd) (cmd & APP_OPERATE)
#define is_file_cmd(cmd) (cmd & IPI_FASTSYS_FILE_OPERATE)

#define get_cmd(cmd) (cmd & 0x7f)
#define SHARE_SIZE (900 * 1024) // SHARE_SIZE MAX 960K


extern AK_S32 isp_quick_fd;

int glass_fd = -1;

static sem_t glass_sem;

static int glass_cur_cmd = 0;
static int glass_save_fp = -1;

static command_data_t cmd_data = {0};

// Thread synchronization variables
static pthread_mutex_t cmd_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cmd_cond = PTHREAD_COND_INITIALIZER;

// Sync/Async mode control flag
// 0: Sync mode - command_thread_func must wait for worker_thread_func to finish before getting the next command
// 1: Async mode - Queue mode asynchronous execution
static int execution_mode_flag = 0; // Default is async mode

static pthread_t command_thread;
static int command_thread_exit = 0;


int ai_glass_ipi_cmd_ready()
{
	int ret = fcntl(glass_fd, F_GETFL);
    //printf("fcntl F_GETFL ret:%d\n", ret);
    ret = fcntl(glass_fd, F_SETFL, ret | O_ASYNC);
    //printf("fcntl F_SETFL ret:%d\n", ret);
    ret = fcntl(glass_fd, F_SETOWN, getpid());
    //printf("fcntl F_SETOWN ret:%d\n", ret);
    //ret = fcntl(glass_fd, F_SETSIG, SIGIO); // (If custom command, do not uncomment)
    //printf("fcntl F_SETSIG SIG_GLASS:%d, SIGUSR1:%d, ret:%d\n", SIG_GLASS, SIGUSR1, ret);

	quick_glass_info_t glass_info = {0};
    ioctl(glass_fd, NOTIFY_USER_READY, &glass_info);

	return 0;
}


// Pack data
int ai_glass_info_pack_data(uint8_t result, uint8_t* output, uint8_t* data, uint8_t lens)
{
    int _cnt = 0;

    output[_cnt++] = result;

    if(data == NULL)
    {
        return _cnt;
    }

    for(int i= 0; i < lens; i++)
    {
        output[_cnt++] = data[i];
    }

    return _cnt;
}



int set_glass_info_v4_notify(unsigned int cmd, char *parameter, unsigned int param_len)
{
    int ret = 0;
    quick_glass_info_t local_glass_info = {0};

    if (glass_fd < 0)
    {
        log_error_app("fd error return -1\n");
        return -1;
    }

    // Copy crc from global variable glass_info
    local_glass_info.crc = 0;

    // Set input parameters
    local_glass_info.cmd = cmd;
    local_glass_info.param_len = param_len;

    // Copy parameter data
    if (parameter && param_len > 0)
    {
        if (param_len > sizeof(local_glass_info.parameter))
        {
            log_error_app("param_len %u exceeds maximum size %zu\n", param_len, sizeof(local_glass_info.parameter));
            return -1;
        }
        memcpy(&local_glass_info.parameter[0], parameter, param_len);
    }

    ret = ioctl(glass_fd, NOTIFY_GLASS_INFO, &local_glass_info);
    if (ret < 0)
    {
        log_info_app("set ak_glass info failed\n");
    }
    //log_debug_app("notify cmd:%#x, param_len:%d\n", local_glass_info.cmd, local_glass_info.param_len);
    char msg[256] = {0};
    sprintf(msg, "[linux->ipi] notify cmd:0x%02x, param_len:%d, parameter:", local_glass_info.cmd, local_glass_info.param_len);
    if (parameter && param_len > 0) {
        for (unsigned int i = 0; i < param_len && i < 32; ++i) {
            sprintf(msg+strlen(msg), " 0x%02X", (unsigned char)parameter[i]);
        }
    }
    printf("%s\n", msg);

    return 0;
}

// km02 linux ack -> km02 rtt
int set_glass_info_v4(unsigned int cmd, char *parameter, unsigned int param_len)
{

    int ret = 0;
    quick_glass_info_t local_glass_info = {0};

    if (glass_fd < 0)
    {
        log_error_app("fd -1\n");
        return -1;
    }

    // Copy crc from global variable glass_info
    local_glass_info.crc = 0;

    // Set input parameters
    local_glass_info.cmd = cmd;
    local_glass_info.param_len = param_len;

    // Copy parameter data
    if (parameter && param_len > 0)
    {
        if (param_len > sizeof(local_glass_info.parameter))
        {
            log_error_app("param_len %u exceeds maximum size %zu\n", param_len, sizeof(local_glass_info.parameter));
            return -1;
        }
        memcpy(&local_glass_info.parameter[0], parameter, param_len);
    }

    ret = ioctl(glass_fd, SET_GLASS_INFO, &local_glass_info);
    if (ret < 0)
    {
        log_error_app("set ak_glass info failed\n");
    }
    char msg[256] = {0};
    sprintf(msg, "[linux->ipi] ACK cmd:0x%02x, param_len:%d, parameter:", local_glass_info.cmd, local_glass_info.param_len);
    if (parameter && param_len > 0) {
        for (unsigned int i = 0; i < param_len && i < 32; ++i) {
            sprintf(msg+strlen(msg), " 0x%02X", (unsigned char)parameter[i]);
        }
    }
    printf("%s\n", msg);


    return 0;
}



/**
 * @brief Command acquisition thread function
 *
 * @param arg
 * @return void*
 */
static void *command_thread_func(void *arg)
{
    log_info_app("Command thread started\n");
    static uint8_t prev_cmd = 0xF;
    int ret = 0, result = 0;
    char *shared_mem = NULL;
	quick_glass_info_t glass_info = {0};
	char file_name[128] = {0};
	int glass_file_has = 0, glass_try_cnt = 0;

    while (!command_thread_exit)
    {
        // Get command
        ////// New get glass_info
UPDATE_GLASS_INFO:
        sem_wait(&glass_sem);
        memset(&glass_info,0,sizeof(quick_glass_info_t));
        ret = ioctl(glass_fd, GET_GLASS_INFO, &glass_info);

        if (ret < 0)
        {
            log_info_app("get ak_glass info failed.. retrying\n");
            usleep(10 * 1000);
            if(ota_run_flag)
            {
                log_debug_app("ota_run_flag break!!!!!!\n");
                break;
            }else{
                continue;
            }
        }
        else // if(glass_info.cmd > 0)
        {
            printf("[ipi->linux] get ak_glass info cmd:0x%02x, len:%d syst=%lld\n", glass_info.cmd, glass_info.param_len, get_sys_run_time64());
        }

#if 0 // only test more cmd list && ipi file read/write
        if( glass_file_has<3 && !is_file_cmd(glass_info.cmd) && get_cmd(glass_info.cmd) > CMD_SNAPSHOT )
        {
           printf("wait test file operate\n");
           ret = 1;
           memcpy((char *)&glass_info.parameter, (char *)&ret, sizeof(ret));
           glass_info.param_len = sizeof(ret);
           ret=ioctl(glass_fd, SET_GLASS_INFO, &glass_info);
           usleep(20 * 1000);
           goto UPDATE_GLASS_INFO;
        }
#endif

        if (is_file_cmd(glass_info.cmd))
        {
            if (get_cmd(glass_info.cmd) == IPI_FASTSYS_FILE_OPEN && (glass_cur_cmd != IPI_FASTSYS_FILE_OPEN))
            {
                wait_for_sdnand(BT_CMD_PHOTO);
                glass_cur_cmd = IPI_FASTSYS_FILE_OPEN;
                memset(file_name, 0, sizeof(file_name));
                memcpy(file_name, ((char *)glass_info.parameter + 4), glass_info.param_len - 4);
                int open_flag = *((int *)glass_info.parameter);
                // printf("data:0x%02x %02x %02x %02x %02x %02x %02x %02x\n",
                //*((char *)glass_info.parameter+0), *((char *)glass_info.parameter+1), *((char *)glass_info.parameter+2), *((char *)glass_info.parameter+3),
                //*((char *)glass_info.parameter+4), *((char *)glass_info.parameter+5), *((char *)glass_info.parameter+6), *((char *)glass_info.parameter+7)
                //);
                log_info_app("open :%s, flag:0x%x \n", file_name, open_flag);
                if (file_name && file_name[0])
                {
                    // glass_save_fp = fopen(file_name, open_flag);
                    // printf("open ok glass_save_fp :%p\n", glass_save_fp);
                    glass_save_fp = open(file_name, open_flag, 0755);
                    log_info_app("open %s glass_save_fp :%d\n", file_name, glass_save_fp);
                    memset((char *)&glass_info.parameter, 0, glass_info.param_len);
                    if (glass_save_fp >= 0)
                        ret = 1;
                    else
                        ret = 0;
                    memcpy((char *)&glass_info.parameter, (char *)&ret, sizeof(ret));
                }
                else
                {
                    log_info_app("filename empty \n");
                    memset((char *)&glass_info.parameter, 0, glass_info.param_len);
                }
                glass_file_has = 1;
            }
            else if (get_cmd(glass_info.cmd) == IPI_FASTSYS_FILE_SEEK)
            {
                glass_cur_cmd = IPI_FASTSYS_FILE_SEEK;
                if (glass_save_fp >= 0)
                {
                    int whence = *((int *)glass_info.parameter);
                    int off_set = *((int *)glass_info.parameter + 1);
                    log_info_app("off_set:%d, whence:%d\n", off_set, whence);
                    ret = lseek(glass_save_fp, off_set, whence);
                    log_info_app("lseek %d ret:%d\n", whence, ret);
                }
                else
                {
                    log_info_app("first open file, glass_save_fp NULL\n");
                    ret = 0;
                }
                memcpy((char *)&glass_info.parameter, (char *)&ret, sizeof(ret));
            }
            else if (get_cmd(glass_info.cmd) == IPI_FASTSYS_FILE_CLOSE)
            {
                glass_cur_cmd = IPI_FASTSYS_FILE_CLOSE;
                if (glass_save_fp >= 0)
                {
                    close(glass_save_fp);
                    log_info_app("close %s fd:%d\n", file_name, glass_save_fp);
                    ret = 1;
                    glass_save_fp = -1;
                }
                else
                {
                    log_error_app("close fail, glass_save_fp NULL\n");
                    ret = 0;
                }
                munmap(shared_mem, SHARE_SIZE);
                shared_mem = NULL;
                memcpy((char *)&glass_info.parameter, (char *)&ret, sizeof(ret));
            }
            else if (get_cmd(glass_info.cmd) == IPI_FASTSYS_FILE_WRITE)
            {
                glass_cur_cmd = IPI_FASTSYS_FILE_WRITE;
                ret = 0;
                if (shared_mem == NULL)
                    shared_mem = (char *)mmap(NULL, SHARE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, glass_fd, 0);
                if (shared_mem == MAP_FAILED)
                    log_info_app("mmap failed\n");
                else
                {
                    log_info_app("shared_mem:%p\n", shared_mem);
                    FILE *save_fp = NULL;
                    if (glass_save_fp < 0)
                        save_fp = fopen("/mnt/snap.jpg", "w+");

                    if (glass_save_fp >= 0)
                    {
                        ret = write(glass_save_fp, shared_mem, glass_info.param_len);
                        log_info_app("write %s ret:%d\n", file_name, ret);
                        // close(glass_save_fp);
                        // munmap(shared_mem, SHARE_SIZE);
                        ret = 1;
                    }
                    else if (save_fp)
                    {
                        ret = fwrite(shared_mem, glass_info.param_len, 1, save_fp);
                        log_info_app("def write /mnt/snap.jpg ret:%d\n", ret);
                        fflush(save_fp);
                        fclose(save_fp);
                        munmap(shared_mem, SHARE_SIZE);
                        ret = 1;
                    }
                    memcpy((char *)&glass_info.parameter, (char *)&ret, sizeof(ret));
                    glass_file_has = 2;
                }
            }
            else if (get_cmd(glass_info.cmd) == IPI_FASTSYS_FILE_READ)
            {
                glass_cur_cmd = IPI_FASTSYS_FILE_READ;
                ret = 0;

                // #define SHARE_SIZE (300*1024)   //SHARE_SIZE MAX 900K
                if (shared_mem == NULL)
                    shared_mem = (char *)mmap(NULL, SHARE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, glass_fd, 0);
                if (shared_mem == MAP_FAILED)
                    log_info_app("mmap failed\n");
                else
                {
                    log_info_app("shared_mem:%p\n", shared_mem);
                    // FILE *save_fp = NULL;
                    if (glass_save_fp < 0)
                    {
                        log_info_app("need first open file\n");
                        ret = 0;
                    }

                    if (glass_save_fp >= 0)
                    {
                        ret = read(glass_save_fp, shared_mem, glass_info.param_len);
                        log_info_app("read %s ret:%d\n", file_name, ret);
                        // close(glass_save_fp);
                    }
                    memcpy((char *)&glass_info.parameter, (char *)&ret, sizeof(ret));
                    glass_info.param_len = sizeof(ret);
                    // munmap(shared_mem, SHARE_SIZE);
                    //glass_info.cmd = IPI_MASTER_FILE_ACK; //file opera result report

                    ret = ioctl(glass_fd, SET_GLASS_INFO, &glass_info);
                    if (ret < 0)
                    {
                        log_info_app("set ak_glass info failed\n");
                    }
                    else
                        log_info_app("set ak_glass ack cmd:0x%02x, ret:%d\n", glass_info.cmd, *(int *)glass_info.parameter);
                    glass_file_has = 3;
                    goto UPDATE_GLASS_INFO;
                }
            }

            // if( glass_cur_cmd != IPI_FASTSYS_FILE_READ)
            {
                if (ret != 1)
                    ret = 0;

                glass_try_cnt++;

                //glass_info.cmd = IPI_MASTER_FILE_ACK; // file opera result report
                glass_info.param_len = sizeof(ret);
                ret = ioctl(glass_fd, SET_GLASS_INFO, &glass_info);
                if (ret < 0)
                {
                    log_info_app("set ak_glass info faile\n");
                }
                else
                    log_info_app("[FILE_CMD]set ak_glass ack cmd:0x%02x, ret:%d\n", glass_info.cmd, glass_info.parameter[0]);

                //if (glass_cur_cmd != IPI_FASTSYS_FILE_CLOSE)
                    // if( glass_try_cnt<=3 )
                    goto UPDATE_GLASS_INFO;
            }
            //goto UPDATE_GLASS_INFO;
        }
        ///////////
        if (ret == 0 && isp_quick_fd >= 0)
        // if (get_glass_info(isp_quick_fd) == 0)
        {
            // Check if cmd is 0xF, if so ignore and delay 10ms before retrying
            // Check if cmd is the same, if so ignore and delay 10ms before retrying
            // if (glass_info.cmd == 0xF || prev_cmd == glass_info.cmd)
            // {
            //     // printf("Command thread: Ignoring cmd 0xF, retrying in 10ms...\n");
            //     usleep(10 * 1000); // Delay 10ms
            //     continue;
            // }
            // if (prev_cmd == glass_info.cmd)
            // {
            //     printf("Same Command!!!prev_cmd:%#x glass_info.cmd:%#x\n",prev_cmd, glass_info.cmd);
            //     usleep(10 * 1000); // Delay 10ms
            //     continue;
            // }

            // printf("get glass_info.cmd 0x%x. len %d data %s\n",
            //     glass_info.cmd,glass_info.param_len,glass_info.parameter);
            // Get device info
            // AK_U32 u32devid;
            // for (u32devid = 0; u32devid < g_s32DevCnt; u32devid++)
            // {
            //     get_vi_dev_info(isp_quick_fd, u32devid, &qs_dev.dev_info[u32devid]);
            //     get_venc_attr(isp_quick_fd, u32devid, &qs_dev.venc_chn[u32devid]);
            // }

            if (execution_mode_flag == 1) {
                // Async mode: Use queue
                if (cmd_queue_push(&glass_info) == 0) {
                    log_info_app("Command thread: [ASYNC] Got new command 0x%02x, added to queue\n", glass_info.cmd);

                    // Notify worker thread of new command
                    pthread_mutex_lock(&cmd_mutex);
                    pthread_cond_signal(&cmd_cond);
                    pthread_mutex_unlock(&cmd_mutex);
                } else {
                    log_info_app("Command thread: [ASYNC] Failed to add command 0x%02x to queue\n", glass_info.cmd);
                }

                // Async mode: Do not wait for worker thread to finish, continue to get next command directly
                log_info_app("Command thread: [ASYNC] Command queued, ready for next command\n");
            } else {
                // Sync mode: Use shared structure
                pthread_mutex_lock(&cmd_mutex);

                // Wait for previous command to be processed
                while (cmd_data.valid && !cmd_data.processed && !command_thread_exit) {
                    log_info_app("Command thread: [SYNC] Waiting for previous command to be processed...\n");
                    pthread_cond_wait(&cmd_cond, &cmd_mutex);
                }

                if (!command_thread_exit) {
                    // Set new command
                    memcpy(&cmd_data.glass_info, &glass_info, sizeof(quick_glass_info_t));
                    cmd_data.valid = 1;
                    cmd_data.processed = 0;

                    log_info_app("Command thread: [SYNC] Got new command 0x%02x, waiting for processing\n", glass_info.cmd);

                    // Notify worker thread of new command
                    pthread_cond_signal(&cmd_cond);

                    // Wait for worker thread to finish processing
                    while (cmd_data.valid && !cmd_data.processed && !command_thread_exit) {
                        pthread_cond_wait(&cmd_cond, &cmd_mutex);
                    }

                    if (!command_thread_exit) {
                        log_info_app("Command thread: [SYNC] Command 0x%02x processed, ready for next command\n", glass_info.cmd);
                        cmd_data.valid = 0;
                        cmd_data.processed = 0;
                    }
                }

                pthread_mutex_unlock(&cmd_mutex);
            }

            prev_cmd = glass_info.cmd;
        }
        else
        {
            if(ret==0)
            {
               ret = 1;
               memcpy((char *)&glass_info.parameter, (char *)&ret, sizeof(ret));
               glass_info.param_len = sizeof(ret);
               ret=ioctl(glass_fd, SET_GLASS_INFO, &glass_info);
               if (ret < 0)
               log_error_app("set ak_glass info failed\n");
               else
               log_info_app("set ak_glass ack cmd:0x%x, ret:%d\n", glass_info.cmd, *(int *)glass_info.parameter);
            }
            else log_error_app("Command thread: Failed to get glass info, retrying...\n");
            // sleep(1);
            usleep(10 * 1000);
        }
    }

    log_info_app("Command thread exited\n");
    return NULL;
}

static void glass_sig(int sig)
{
    sem_post(&glass_sem);
}

void ai_glass_ipi_cmd_sem_signal()
{
	signal(SIGIO, glass_sig);
}

void ai_glass_ipi_cmd_sem_init()
{
	sem_init(&glass_sem, 0, 0);
}

int ai_glass_ipi_cmd_fd_open()
{
	glass_fd = open("/dev/ak_glass", O_RDWR, 0);
    if (glass_fd < 0)
    {
        log_error_app("open ak_glass device fail, run on normal mode\n");
        return -1;
    }

	return 0;
}

int ai_glass_ipi_cmd_fd_close()
{
	if(glass_fd >= 0)
    {
        close(glass_fd);
        glass_fd = -1;
    }

	return 0;
}



int ai_glass_ipi_cmd_init()
{
	if(execution_mode_flag == 1)
    {
        // Initialize command queue
        cmd_queue_init();
    }
    else
    {
        // Initialize thread synchronization variables
        pthread_mutex_init(&cmd_mutex, NULL);
        pthread_cond_init(&cmd_cond, NULL);
    }

	    // Print current execution mode
    log_info_app("Execution mode: %s (flag=%d)\n",
           execution_mode_flag ? "ASYNC (Queue)" : "SYNC (Wait)",
           execution_mode_flag);


	// Create command acquisition thread
	int ret = pthread_create(&command_thread, NULL, command_thread_func, NULL);
	if (ret != 0)
	{
		log_error_app("Failed to create command thread: %d\n", ret);
		return ret;
	}
	return 0;
}

int ai_glass_ipi_cmd_deinit()
{
	command_thread_exit = 1;
	pthread_cond_broadcast(&cmd_cond);
	pthread_join(command_thread, NULL);

    if(execution_mode_flag == 1)
    {
        // Clean up command queue
        cmd_queue_clear();
        destroy_queue_mutex();
    }
    else
    {
        pthread_mutex_destroy(&cmd_mutex);
        pthread_cond_destroy(&cmd_cond);
    }

	return 0;
}


int ai_glass_ipi_cmd_get(quick_glass_info_t *local_glass_info)
{
	if (execution_mode_flag == 1)
	{
		// Async mode: Fetch command from queue
		pthread_mutex_lock(&cmd_mutex);
		while (cmd_queue_is_empty() && !command_thread_exit /*!worker_thread_exit*/)
		{
		  pthread_cond_wait(&cmd_cond, &cmd_mutex);
		}
		pthread_mutex_unlock(&cmd_mutex);

//		if (worker_thread_exit)
//		{
//		  return -1;
//		}

		// Pop command from queue
		if (cmd_queue_pop(local_glass_info) != 0) {
		  log_info_app("Worker thread: [ASYNC] Failed to pop command from queue\n");
		  return 0;
		}

		log_info_app("Worker thread:Worker thread: [ASYNC] Processing command 0x%02x syst=%lld \n", local_glass_info->cmd, get_sys_run_time64());
	}
	else
	{
		// Sync mode: Fetch command from shared structure
		pthread_mutex_lock(&cmd_mutex);

		// Wait for new command
		while (!cmd_data.valid && !command_thread_exit /*!worker_thread_exit*/) {
		  pthread_cond_wait(&cmd_cond, &cmd_mutex);
		}

//		if (worker_thread_exit) {
//		  pthread_mutex_unlock(&cmd_mutex);
//		  return -1;
//		}

		// Copy command data and immediately mark as processed to prevent duplicates
		memcpy(local_glass_info, &cmd_data.glass_info, sizeof(quick_glass_info_t));
		log_info_app("Worker thread: [SYNC] Processing command 0x%02x\n", local_glass_info->cmd);

		// Immediately clear command data to prevent duplicates
		cmd_data.valid = 0;

		pthread_mutex_unlock(&cmd_mutex);
	}

	return 0;
}

void ai_glass_ipi_cmd_completed(quick_glass_info_t *glass_info)
{
	// In sync mode, notify command_thread that processing is complete
	if (execution_mode_flag == 0) {
		pthread_mutex_lock(&cmd_mutex);
		cmd_data.processed = 1;
		log_info_app("Worker thread: [SYNC] Command 0x%02x processing completed, notifying command thread (syst=%lld)\n", glass_info->cmd, get_sys_run_time64());
		pthread_cond_signal(&cmd_cond);
		pthread_mutex_unlock(&cmd_mutex);
	}

	log_info_app("Worker thread: Command processed, ready for next command\n");
}
