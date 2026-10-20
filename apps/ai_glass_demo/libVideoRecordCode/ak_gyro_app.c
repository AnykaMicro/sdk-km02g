/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd. */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/ioctl.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "media_configs.h"
#include "ak_common.h"
#include "ak_types.h"
#include "ak_thread.h"
#include "ak_log.h"
#include "ak_mem.h"
#include "ak_common_video.h"
#include "ak_vi.h"
#include "ak_vpss.h"
#include "ak_venc.h"
#include "ak_video.h"
#include "ak_ipu.h"

#include "ak_gyro_app.h"
#include "ai_glass_cmd_handle.h"

#include "ak_imu_cmd.h"
#include "fun_log.h"
#include "ak_eis.h"


#define GYRO_SENSITIVITY 16.4f    // LSB/(deg/s) for +/-2000DPS
#define ACCEL_SENSITIVITY 16384.0f // LSB/g for +/-2g

typedef struct {
    float x;
    float y;
    float z;
    double timestamp_ms;
} vector_3f;



//Return 0: Indicates that no calibration data was read
//Return 1: Indicates successful reading
int read_eis_offset_data(int32_t *raw_gyro_x, int32_t *raw_gyro_y, int32_t *raw_gyro_z)
{
	log_warn_app("read eis offset data start");
    FILE *hf = fopen("/etc/config/eis_offset.cfg", "r");
	if(hf == NULL)
	{
		log_error_app("fopen eis offset data cfg failed");
		return 0;
	}

	char data[32] = {0};
	int datalen = fread(data, 1, sizeof(data)-1, hf);
	fclose(hf);
	if(datalen < 5) //strlen("0,0,0")
	{
		log_error_app("read eis offset data failed");
		return 0;
	}

	int result = sscanf(data, "%d,%d,%d", raw_gyro_x, raw_gyro_y, raw_gyro_z);
	if(result != 3)
	{
		log_error_app("read eis offset data failed");
		return 0;
	}

	log_warn_app("read eis offset data ok(x=%d,y=%d,z=%d)", *raw_gyro_x, *raw_gyro_y, *raw_gyro_z);
	return 1;
}

int write_eis_offset_data(int32_t raw_gyro_x, int32_t raw_gyro_y, int32_t raw_gyro_z)
{
	log_warn_app("write eis offset data start");
	FILE *hf = fopen("/etc/config/eis_offset.cfg", "w");
	if(hf == NULL)
	{
		log_error_app("fopen eis offset data cfg failed");
		return 0;
	}

	char data[32] = {0};
	sprintf(data, "%d,%d,%d", raw_gyro_x, raw_gyro_y, raw_gyro_z);
	int datalen = fwrite(data, 1, strlen(data), hf);
	fclose(hf);

	log_warn_app("write eis offset data ok(x=%d,y=%d,z=%d)", raw_gyro_x, raw_gyro_y, raw_gyro_z);

	return 1;
}


static void* get_eis_data_th(void* arg);

int32_t gyro_offset_pitch = 0;
int32_t gyro_offset_yaw = 0;
int32_t gyro_offset_roll = 0;


static AK_PTHREAD_T th = 0;
int gyro_fd = -1;
int record_state_flag = 1;

vector_3f gyro_offset = {0};
vector_3f accel_offset = {0};

int gyro_data_reset(void)
{

    // Calibrate gyroscope
    int32_t gx, gy, gz;
    float sum_gx = 0.0f, sum_gy = 0.0f, sum_gz = 0.0f;
    vector_3f accel_sum = {0.0f, 0.0f, 0.0f};
    int data_cnt = 0;

    gyro_data_t *data = NULL;
    int ret = 0;
    int reset_cnt = 0;
    do
    {
        ret = gyro_get_raw_data(&data);
        if(reset_cnt++ > 10)
        {
            printf("fun=%s line=%d error.\n",__FUNCTION__,__LINE__);
            break;
        }
    }while (ret == 0);

// If too much data comes in for the first time, take only 50 data points
    ret = ret > 50 ? 50 : ret;

    for(int i = 0;i < ret;i++)
    {
        sum_gx += data[i].gyro_x;
        sum_gy += data[i].gyro_y;
        sum_gz += data[i].gyro_z;
    }
    gyro_release_raw_data(data);

    data_cnt += ret;

    gyro_offset_pitch = (int)(sum_gx / (1.0 * data_cnt));
    gyro_offset_yaw   = (int)(sum_gy / (1.0 * data_cnt));
    gyro_offset_roll  = (int)(sum_gz / (1.0 * data_cnt));

    //accel_offset.x = accel_sum.x / (1.0 * samples);
    //accel_offset.y = accel_sum.y / (1.0 * samples);
    //accel_offset.z = accel_sum.z / (1.0 * samples);

	write_eis_offset_data(gyro_offset_pitch, gyro_offset_yaw, gyro_offset_roll);

	return 1;
}

static int imu_start(int fd)
{	printf("%s_%d \n", __FUNCTION__, __LINE__);
    #if 1
    struct imu_config imu_config;
    imu_config.acc_config.acc_odr = ACC_ORD_800Hz;
    imu_config.acc_config.acc_range = FULL_SCALE_RANGE_16G;

    imu_config.gyro_config.gyro_odr = GYRO_ORD_800Hz;
    imu_config.gyro_config.gyro_range = FULL_SCALE_RANGE_2000;

    if (ioctl(fd, IOC_SET_CONFIG, &imu_config) < 0) {
        printf("imu_config fail!\r\n");
        close(fd);
        return -1;
    }
    #endif
    return 0;
}

int gyro_init(int mode)
{
		printf("%s_%d eis_ver=%s \n", __FUNCTION__, __LINE__, ak_eis_get_version());
    char* filename = "/dev/imu";
    gyro_fd = open(filename, O_RDWR);
    if (gyro_fd < 0) {
        printf("can't open file %s\r\n", filename);
        return -1;
    }
    int flags = fcntl(gyro_fd, F_GETFL, 0);
    fcntl(gyro_fd, F_SETFL, flags | O_NONBLOCK);

    if(imu_start(gyro_fd) < 0)
    {
		printf("imu_start fail!\r\n");
		gyro_fd = -1;
        return -1;
    }

	int iRet = read_eis_offset_data(&gyro_offset_pitch, &gyro_offset_yaw, &gyro_offset_roll);
	if(iRet == 0)
	{
		gyro_data_reset();
	}

    if(mode == 0)
    {
        AK_BASE_ThreadCreate(&th, get_eis_data_th,
               (void *)NULL, ANYKA_THREAD_MIN_STACK_SIZE + (512*1024), 60);
    }


	return 0;
}

int gyro_deinit()
{
	record_state_flag = 0;

    if(gyro_fd > 0)
    {
        close(gyro_fd);
        gyro_fd = -1;
    }

    printf("%s_%d \n", __FUNCTION__, __LINE__);
    if(th != 0)
	{
        AK_BASE_ThreadJoin(th);
        th = 0;
    }
	printf("%s_%d \n", __FUNCTION__, __LINE__);
	th = 0;

}
#define GYRO_DROP_DATA 100
// Get raw gyroscope data
int gyro_get_raw_data(gyro_data_t **data)
{
	if(gyro_fd < 0)
	{
		return 0;
	}
    static char *buff = NULL;
    static int buff_len = 0;
    if(buff == NULL)
    {
        buff_len = 1024 * sizeof(struct imu_data);
        buff = malloc(buff_len);
    }

	int cnt = read(gyro_fd, buff, buff_len);

    int packet_cnt = cnt / sizeof(struct imu_data);
	if(packet_cnt <= 0)
	{
	*data = NULL;
		return 0;
	}

    static int dropCnt = 0;

    struct imu_data* buffer = ( struct imu_data*)buff;

    // Reduce the number of discarded packets to start processing faster
    if (dropCnt < GYRO_DROP_DATA ) { // Only discard the first 0.5s of data (originally 2s)

        if(dropCnt + packet_cnt <= GYRO_DROP_DATA)
        {
            printf("fun=%s line=%d packet_cnt=%d \n",__FUNCTION__,__LINE__,packet_cnt);
            *data = NULL;
            dropCnt += packet_cnt;
            return 0;
        }
        else
        {
            buffer = buffer + ( GYRO_DROP_DATA - dropCnt);
            printf("fun=%s line=%d dropCnt=%d packet_cnt=%d \n",__FUNCTION__,__LINE__,dropCnt,packet_cnt);
            packet_cnt = packet_cnt - ( GYRO_DROP_DATA - dropCnt);
            dropCnt += GYRO_DROP_DATA+1;
        }
    }

    //*data =;
    gyro_data_t *tmp = (gyro_data_t *)malloc(sizeof(gyro_data_t) * packet_cnt);
    *data = tmp;

    for(int i = 0; i < packet_cnt; i++)
    {
        tmp[i].acc_x = buffer[i].acc_x;
        tmp[i].acc_y = buffer[i].acc_y;
        tmp[i].acc_z = buffer[i].acc_z;
        tmp[i].gyro_x = buffer[i].gyro_x - gyro_offset_pitch;
        tmp[i].gyro_y = buffer[i].gyro_y - gyro_offset_yaw;
        tmp[i].gyro_z = buffer[i].gyro_z - gyro_offset_roll;
        tmp[i].timestamp = buffer[i].timestamp;
    }


	return packet_cnt;
}


int gyro_release_raw_data(gyro_data_t *data)
{
    if(data)
    {
        free(data);
    }

    return 0;
}


static void* get_eis_data_th(void* arg)
{
	printf("%s_%d \n", __FUNCTION__, __LINE__);

    int ret = -1;
    char *buffer = NULL;
    int buff_len = 1024 * sizeof(struct imu_data);
    buffer = malloc(buff_len);
    if (!buffer) {
        perror("Failed to allocate buffer");
        free(buffer);
        return NULL;
    }

    FILE *fd = fopen(GYRO_DATA_PATH,"w+");
    setvbuf(fd, NULL, _IOFBF, 8192);

//    char *tag = "timestamp,gyro_x,gyro_y,gyro_z\n";
//    fwrite(tag,strlen(tag),1,fd);

    printf("%s_%d \n", __FUNCTION__, __LINE__);
    while(record_state_flag)
    {
        gyro_data_t *data = NULL;
        ret = gyro_get_raw_data(&data);
        if(ret == 0)
        {
            usleep(10*1000);
            continue;
        }

        for(int i = 0; i < ret; i++)
        {
            eis_gyro_info gyro_data = {0};

            gyro_data.gyro_x = data[i].gyro_x;
            gyro_data.gyro_y = data[i].gyro_y;
            gyro_data.gyro_z = data[i].gyro_z;
            gyro_data.timestamp = ak_eis_code_mux(data[i].timestamp);

//            int len = gyro_data_format_str_remote(&data[i],str);
            fwrite(&gyro_data,sizeof(eis_gyro_info),1,fd);
        }
        gyro_release_raw_data(data);

    }

    fclose(fd);
	printf("%s_%d \n", __FUNCTION__, __LINE__);

//	char dst_name[512] = {0};
//	snprintf(dst_name, sizeof(dst_name), "%s_imu.csv", g_eis_data_file_name);
//
//	char cmd[512] = {0};
//	snprintf(cmd, sizeof(cmd), "mv %s %s", GYRO_DATA_PATH, dst_name);
//	system(cmd);

//	printf("%s_%d dst_name=%s g_eis_data_file_name=%s\n", __FUNCTION__, __LINE__, dst_name, g_eis_data_file_name);

    return NULL;
}
