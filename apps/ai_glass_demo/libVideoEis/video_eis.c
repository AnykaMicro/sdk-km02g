/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd. */
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <pthread.h>
#include <errno.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <signal.h>
#include <poll.h>
#ifndef __USE_POSIX199309
#define __USE_POSIX199309
#endif
#include <time.h>

// Platform related header files
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

// Anti-shake algorithm C interface
#include "ak_eis.h"
#include "antiShake.h"
#include "ai_glass_time.h"

#include "fun_log.h"

#include "ak_imu_cmd.h"

#include "video_eis.h"
#include "print_color_simple.h"
#include "ak_gyro_app.h"



typedef struct {
    uint64_t timestamp;
    IPU_DMA_S stIpuDmaTrans;
} TransformParams;


// Queue structure definition
typedef struct QueueNode {
    void* data;
    struct QueueNode* next;
} QueueNode;

typedef struct {
    QueueNode* front;
    QueueNode* rear;
    int size;
    AK_MUTEX_T mutex;
} Queue;

typedef struct {
    int vi_width;
    int vi_height;
    int out_width;
    int out_height;
} eis_video_param_t;

static eis_video_param_t g_video_param = {0};



// Global variables
static bool g_thread_running = true;
static AK_PTHREAD_T g_eis_data_thread;
static AK_PTHREAD_T g_eis_process_thread;
static AK_PTHREAD_T g_crop_eis_thread;


// Queue and data storage
static Queue g_quat_queue;
static Queue g_frame_queue;
static Queue g_frame_pts_queue;
static Queue g_param_queue;
static AK_MUTEX_T g_data_mutex;

// Anti-shake algorithm related objects
static CIntegrator g_integrator;
static CSmoother g_smoother;


// Video stream related
static int g_vpss_grp = 0; // Main stream vpss group
static int g_vpss_chn = 0; // Main stream channel
static VideoCallBackFunc video_call_back_func = NULL;
static float gyro_sample_ms = 1.25;
static int g_frame_depth = 4;


MUTEX_DEFINE( IMU );

// Queue operation functions
static void queue_init(Queue* queue) {
    queue->front = NULL;
    queue->rear = NULL;
    queue->size = 0;
    AK_BASE_ThreadMutexInit(&queue->mutex, NULL);
}

static void queue_destroy(Queue* queue) {
    while (queue->front) {
        QueueNode* temp = queue->front;
        queue->front = queue->front->next;
        free(temp->data);
        free(temp);
    }
    AK_BASE_ThreadMutexDestroy(&queue->mutex);
}

static void queue_push(Queue* queue, void* data, size_t data_size)
{
    if (!queue || !data || data_size == 0) {
        printf("queue_push: Invalid parameters\n");
        return;
    }

    AK_BASE_ThreadMutexLock(&queue->mutex);

    QueueNode* new_node = malloc(sizeof(QueueNode));
    if (!new_node) {
        printf("queue_push: Failed to allocate node\n");
        AK_BASE_ThreadMutexUnlock(&queue->mutex);
        return;
    }

    new_node->data = malloc(data_size);
    if (!new_node->data) {
        printf("queue_push: Failed to allocate data\n");
        free(new_node);
        AK_BASE_ThreadMutexUnlock(&queue->mutex);
        return;
    }

    memcpy(new_node->data, data, data_size);
    new_node->next = NULL;

    if (queue->rear) {
        queue->rear->next = new_node;
    } else {
        queue->front = new_node;
    }
    queue->rear = new_node;
    queue->size++;

    AK_BASE_ThreadMutexUnlock(&queue->mutex);
}

static bool queue_pop(Queue* queue, void* data, size_t data_size) {
    AK_BASE_ThreadMutexLock(&queue->mutex);

    if (!queue->front) {
        AK_BASE_ThreadMutexUnlock(&queue->mutex);
        return false;
    }

    QueueNode* temp = queue->front;
    memcpy(data, temp->data, data_size);
    queue->front = queue->front->next;
    if (!queue->front) {
        queue->rear = NULL;
    }
    queue->size--;

    free(temp->data);
    free(temp);

    AK_BASE_ThreadMutexUnlock(&queue->mutex);
    return true;
}

static bool queue_empty(Queue* queue) {
    AK_BASE_ThreadMutexLock(&queue->mutex);
    bool empty = (queue->size == 0);
    AK_BASE_ThreadMutexUnlock(&queue->mutex);
    return empty;
}

static int queue_size(Queue* queue)
{
    AK_BASE_ThreadMutexLock(&queue->mutex);
    int size = queue->size;
    AK_BASE_ThreadMutexUnlock(&queue->mutex);
    return size;
}

// IPU related helper functions
static AK_S32 IPU_Dma_Alloc(IPU_DMA_S *pstIpuDma, AK_U32 u32Size) {
    if ((pstIpuDma != NULL) && (u32Size > 0) &&
        ((pstIpuDma->pvAddr = AK_BASE_MemDmaAlloc(MODULE_ID_GUI, u32Size)) != NULL) &&
        (AK_BASE_MemDmaVaddr2Paddr(pstIpuDma->pvAddr, &pstIpuDma->u64PhyAddr) == AK_SUCCESS)) {
        pstIpuDma->u32Len = u32Size;
        pstIpuDma->u32PhyAddrLow = pstIpuDma->u64PhyAddr & 0xFFFFFFFF;
        pstIpuDma->u8PhyAddrHigh = (pstIpuDma->u64PhyAddr >> 32) & 0xFF;
        return AK_SUCCESS;
    } else {
        return AK_FAILED;
    }
}

static AK_S32 IPU_Dma_Free(IPU_DMA_S *pstIpuDma) {
    if ((pstIpuDma != NULL) && (pstIpuDma->u32Len > 0) && (pstIpuDma->pvAddr != NULL)) {
        AK_BASE_MemDmaFree(pstIpuDma->pvAddr);
        pstIpuDma->u64PhyAddr = 0;
        pstIpuDma->u32Len = 0;
        pstIpuDma->pvAddr = NULL;
        return AK_SUCCESS;
    } else {
        return AK_FAILED;
    }
}

static void init_dma(IPU_COLOR_FMT_E color_fmt, AK_U32 width, AK_U32 height, IPU_DMA_S* dma) {
    AK_U32 u32Size = 0;

    if (dma == NULL) {
        return;
    }

    switch (color_fmt) {
        case IPU_COLOR_FMT_SEMIPLANE_NV12:
        case IPU_COLOR_FMT_SEMIPLANE_NV21:
            u32Size = width * height * 3 / 2;
            break;
        default:
            u32Size = 0;
            break;
    }

    if (u32Size > 0) {
        if (IPU_Dma_Alloc(dma, u32Size) == AK_SUCCESS) {
            dma->enColorFmt = color_fmt;
            dma->u32W = width;
            dma->u32H = height;
        }
    }
}

static void init_surface(IPU_DMA_S* dma, IPU_SURFACE_S* surface) {
    if (dma == NULL || surface == NULL) {
        return;
    }

    surface->u32Width = dma->u32W;
    surface->u32Height = dma->u32H;
    surface->enColorFmt = dma->enColorFmt;
    surface->enAlphaChnChoice = IPU_ALPHA_CHN_NONE;
    surface->enDirection = IPU_DIRECTION_UNCHANGE;

    switch (dma->enColorFmt) {
        case IPU_COLOR_FMT_SEMIPLANE_NV12:
        case IPU_COLOR_FMT_SEMIPLANE_NV21:
            surface->stPlant[0].u64PhyAddr = dma->u64PhyAddr;
            surface->stPlant[0].u8PixStride = 1;
            surface->stPlant[0].u32PitchLine = dma->u32W;

            surface->stPlant[1].u64PhyAddr = dma->u64PhyAddr + dma->u32W * dma->u32H;
            surface->stPlant[1].u8PixStride = 2;
            surface->stPlant[1].u32PitchLine = dma->u32W;
            break;
        default:
            break;
    }
}

static void data2dma(IPU_COLOR_FMT_E color_fmt, AK_U32 width, AK_U32 height,
              AK_U64 phy_addr, void* virt_addr, IPU_DMA_S* dma) {
    AK_U32 u32Size = 0;

    if (dma == NULL || width <= 0 || height <= 0 || phy_addr <= 0 || virt_addr == NULL) {
        return;
    }

    switch (color_fmt) {
        case IPU_COLOR_FMT_SEMIPLANE_NV12:
        case IPU_COLOR_FMT_SEMIPLANE_NV21:
            u32Size = width * height * 3 / 2;
            break;
        default:
            u32Size = 0;
            break;
    }

    if (u32Size > 0) {
        dma->pvAddr = virt_addr;
        dma->u64PhyAddr = phy_addr;
        dma->u32Len = u32Size;
        dma->u32PhyAddrLow = dma->u64PhyAddr & 0xFFFFFFFF;
        dma->u8PhyAddrHigh = (dma->u64PhyAddr >> 32) & 0xFF;
        dma->enColorFmt = color_fmt;
        dma->u32W = width;
        dma->u32H = height;
    }
}


// Thread 1: Get Gyro Data Thread
static void* run_get_eis_data(void* arg)
{
    int packet_cnt;

    int dropCnt = 0;
    CTimeIMU* imu_data = NULL;
    int imu_data_count = 0;
    int imu_data_capacity = 1000;

    AK_BASE_ThreadSetName("eisDataThread");
    printf("=================== eisDataThread start =============\n");

    imu_data = malloc(imu_data_capacity * sizeof(CTimeIMU));
    if (!imu_data) {
        printf("Failed to allocate IMU data buffer\n");
        return NULL;
    }

    while (g_thread_running)
    {
        gyro_data_t *data = NULL;
        packet_cnt = gyro_get_raw_data(&data);
        if(packet_cnt <= 0)
        {
            usleep(5*1000);
            continue;
        }

        for (int i = 0; i < packet_cnt; i++)
        {
            CTimeIMU entry;

            double gyro_x = (1.0) * (double)(data[i].gyro_x) / 16.4;
            double gyro_y = (-1.0) * (double)(data[i].gyro_y) / 16.4;
            double gyro_z = (-1.0) * (double)(data[i].gyro_z) / 16.4;

            entry.timestamp_ms = data[i].timestamp / 1000000.0;
            entry.has_gyro = true;
            entry.gyro = vector3_create(gyro_x, gyro_y, gyro_z);
            entry.has_accl = false;
            entry.has_magn = false;

            // Add to IMU data buffer
            if (imu_data_count >= imu_data_capacity) {
                imu_data_capacity *= 2;
                imu_data = realloc(imu_data, imu_data_capacity * sizeof(CTimeIMU));
            }
            imu_data[imu_data_count++] = entry;
        }

        gyro_release_raw_data(data);

        // Lower threshold to update integrator more frequently
        if (imu_data_count > 10) { // Reduced to 10 samples (originally 20)
            MUTEX_LOCK( IMU );
            integrator_simple_gyro_integrator_test(g_integrator,imu_data,imu_data_count,gyro_sample_ms);
            // Clear IMU data buffer
            imu_data_count = 0;
            MUTEX_UNLOCK( IMU );
        }

    }

    free(imu_data);
    return NULL;
}

// Thread 2: EIS Process Thread
static void* run_eis_process(void* arg)
{
    static bool eis_data_ready = false;  // Static variable to record if enough data accumulated
    VPSS_FRAME_INFO_S frame = {0};
    TransformParams params = {0};
    int64_t frame_pts = 0;
    int frame_cnt = 0;
    bool is_first_frame = true;
    CQuaternion quat_first = quaternion_create_identity();
    CQuaternion quat_smooth = quaternion_create_identity();
    CQuaternion prev_quat = quaternion_create_identity();
    uint64_t last_frame_pts = 0;
    T_TRANS4_PAR stTrans4Par[ 2 ] = {0}, *apstTrans4Par[ 2 ] = {NULL, NULL}; // Distortion correction input parameters

    #define SLIDE_SIZE (1600)
    #define MIN_SLIDE_SIZE (50)  // Minimum sliding window size

    ISP_AE_RUN_INFO_S stAeRunInfo = {0};
    ISP_AE_ATTR_S stAeAttr = {0};
    AK_S32 s32Fps = 0;

    integrator_init(g_integrator,SLIDE_SIZE);

    AK_BASE_ThreadSetName("eisPrcThread");
    printf("=================== eisPrcThread start =============\n");
    // init_trans4_par(&stTrans4Par, DST_FRAME_W, DST_FRAME_H, DST_FRAME_W, DST_FRAME_H);
    init_trans4_par( &stTrans4Par[ 0 ], g_video_param.vi_width, g_video_param.vi_height, g_video_param.vi_width, g_video_param.vi_height ); // Initialize a structure generating distortion correction instructions
    apstTrans4Par[ 0 ] = &stTrans4Par[ 0 ];
    apstTrans4Par[ 1 ] = &stTrans4Par[ 1 ];

    // Create camera matrix
    Matrix3x3 camera_mat = {0};

    // Copy camera matrix
    memcpy(&camera_mat.data,&gm_config.eis_param.data,sizeof(gm_config.eis_param.data));

    while (g_thread_running)
    {
        int ret = AK_MPI_VPSS_GetFrame(g_vpss_grp, g_vpss_chn, &frame, 2000);
        if (ret != AK_SUCCESS) {
            usleep(10000);
            continue;
        }

		static int vno = 0;
        if(vno++ == 0)
        {
            printf("%s_%d vno=%" PRIu64 "",__FUNCTION__, __LINE__, frame.stFrame.u64Ts);
        }

        // Check current available gyro data amount
        int current_quat_size = integrator_get_quat_vector_size(g_integrator);
        if (!eis_data_ready) {
            if (current_quat_size >= MIN_SLIDE_SIZE) {
                eis_data_ready = true;  // Data accumulation ready, switch to normal mode
                printf("EIS data accumulation ready (%d samples), switching to normal mode\n", current_quat_size);
            }
        }

        bool use_center_crop = !eis_data_ready;  // Decide whether to use center crop based on data readiness

		AK_MPI_ISP_GetAeRunInfo(0, &stAeRunInfo);
		AK_MPI_ISP_GetAeAttr(0, &stAeAttr);
		AK_MPI_ISP_GetSensorFps(0, &s32Fps);

		if (0 == s32Fps) s32Fps = 30;
		//printf("exptime : %lu, %lu, fps: %d!\n", stAeRunInfo.u32CurrentExpTime, stAeAttr.u32ExpTimeMax, s32Fps);
		AK_U32 u32ExptimeMs = 1000 * stAeRunInfo.u32CurrentExpTime / stAeAttr.u32ExpTimeMax / s32Fps;
		//printf("u32ExptimeMs : %lu!\n", u32ExptimeMs);

		frame_pts = frame.stFrame.u64Ts + (EXPOSE_OFFSET_TIME - u32ExptimeMs);

        // Add frame to queue
        queue_push(&g_frame_queue, &frame, sizeof(VPSS_FRAME_INFO_S));
        queue_push(&g_frame_pts_queue, &frame_pts, sizeof(uint64_t));
        frame_cnt++;

        if (frame_cnt == g_frame_depth-2) {
            queue_pop(&g_frame_pts_queue, &frame_pts, sizeof(uint64_t));
            frame_cnt--;
        } else {
            continue;
        }

        last_frame_pts = frame_pts;

        Matrix3x3 final_mat;
	CQuaternion quat_diff;

        if (use_center_crop)
        {
            // Use fixed center crop projection matrix when integration data is insufficient
            final_mat = matrix3x3_identity();
        }
        else
        {
            // Normal EIS processing flow
            long long lla = get_sys_run_time64();
            MUTEX_LOCK( IMU );
            integrator_find_quat_by_ts_restore_tmp_quat(g_integrator, frame_pts);
            long long llb = get_sys_run_time64();
            MUTEX_UNLOCK( IMU );
            quat_diff = cal_quat_diff_by_smoothed_window(g_integrator, g_smoother);
            long long llc = get_sys_run_time64();
            //printf("aaaaa a-b=%lld,b-c=%lld\n", llb-lla, llc-llb);
        }

        if (!use_center_crop)
        {
            // Quaternion to rotation matrix conversion
            Matrix3x3 rotation_mat = matrix3x3_quaternion_to_rotation(&quat_diff);

            // Correct rotation matrix (based on original code)
            rotation_mat.data[2][1] *= -1.0;
            rotation_mat.data[1][2] *= -1.0;
            rotation_mat.data[0][1] *= -1.0;
            rotation_mat.data[1][0] *= -1.0;
            rotation_mat.data[2][0] *= -1.0;
            rotation_mat.data[0][2] *= -1.0;

            // Matrix operation: camera_mat * rotation_mat
            Matrix3x3 multi_tmp = matrix3x3_multiply(&camera_mat, &rotation_mat);

            // Calculate pseudoinverse matrix: camera_mat * pseudoinverse(multi_tmp)
            Matrix3x3 multi_tmp_inv = matrix3x3_inverse(&multi_tmp);
            Matrix3x3 pseudo_inv_mat = matrix3x3_multiply(&camera_mat, &multi_tmp_inv);

            // Final matrix: pseudoinverse(pseudo_inv_mat)
            final_mat = matrix3x3_inverse(&pseudo_inv_mat);
       }

        // Copy final matrix to transform parameters
        for (int i = 0; i < 3; i++) {
            for (int m = 0; m < 3; m++) {
                stTrans4Par[0].trans4_factor.perspective_tran4_mat[i][m] = final_mat.data[i][m];
            }
        }

        // Generate transform parameters
        params.timestamp = frame_pts;
        IPU_Dma_Alloc(&params.stIpuDmaTrans, 64 * 1024);

        AK_U32 i_len_trans = get_branch_quu_trans4_distrect_gradual_joint_yv12_new(
                apstTrans4Par, 0, 1, 1, (uint8_t*)params.stIpuDmaTrans.pvAddr);

        if (i_len_trans > 0) {
            // Add parameters to queue
            queue_push(&g_param_queue, &params, sizeof(TransformParams));
        } else {
            // If instruction generation failed, free DMA memory
            IPU_Dma_Free(&params.stIpuDmaTrans);
        }
    }

    return NULL;
}

// Thread 3: Crop and EIS Process Thread
static void* run_crop_eis_process(void* arg)
{
    VPSS_FRAME_INFO_S frame = {0};
    TransformParams crop_params = {0};

    int print_cnt = 0;
    int i_count = 0;
    int ret = 0;
    int venc_handle_id = 0;
    unsigned long prev_ts = 0;

    IPU_RECT_S stIpuRectMid = { 0 }, stIpuRectDst = {0, 0, g_video_param.out_width, g_video_param.out_height}, stIpuRectSrc = {0, 0, g_video_param.vi_width, g_video_param.vi_height};
    IPU_DMA_S stIpuDmaSrc = {0}, stIpuDmaMid = {0}, stIpuDmaDst = {0};
    IPU_SURFACE_S stSurfaceSrc= {0}, stSurfaceMid= {0}, stSurfaceDst = {0};
    char ac_file_src[ 64 ] = {0}, ac_file_mid[ 64 ] = {0}, ac_file_dst[ 64 ], ac_time[ 64 ]= {0};

#ifdef DEBUG_EIS
    unsigned int num = 0;
    FILE *fd_save = NULL;
    DATE_S stDate;
    AK_CHAR cTimeStr[32] = {0};
    AK_CHAR cFilePath[255] = {0};
    char cmd[128] = {0};
#endif

    AK_BASE_ThreadSetName("cropEisThread");
    printf("=================== cropEisThread start =============\n");

    if( ( g_video_param.vi_width != g_video_param.out_width ) && ( g_video_param.vi_height != g_video_param.out_height ) ) {
        stIpuRectMid.s32X = (int)((1 - CROP_RATIO) / 2 * g_video_param.vi_width + 0.5);
        stIpuRectMid.u32W = (int)(CROP_RATIO * g_video_param.vi_width + 0.5);
        stIpuRectMid.u32H = (int)( ( double )stIpuRectMid.u32W / g_video_param.out_width * g_video_param.out_height + 0.5); // Keep aspect ratio
        stIpuRectMid.s32Y = (int)( ( double )( g_video_param.vi_height - stIpuRectMid.u32H ) / 2 + 0.5 );
    }

    // Initialize IPU DMA
    if( ( g_video_param.vi_width != g_video_param.out_width ) || ( g_video_param.vi_height != g_video_param.out_height ) ) { // If output layer needs scaling, create a temporary intermediate layer
        init_dma( IPU_COLOR_FMT_SEMIPLANE_NV12, g_video_param.vi_width, g_video_param.vi_height, &stIpuDmaMid );
        init_surface( &stIpuDmaMid, &stSurfaceMid );
    }
    init_dma(IPU_COLOR_FMT_SEMIPLANE_NV12, g_video_param.out_width, g_video_param.out_height, &stIpuDmaDst);
    init_surface(&stIpuDmaDst, &stSurfaceDst);

    VIDEO_STREAM_S *pstStream = AK_BASE_MemAlloc(MODULE_ID_APP,sizeof(VIDEO_STREAM_S));
    if(NULL == pstStream)
    {
        AK_BASE_PrintErrorEx(MODULE_ID_APP,
            "Can't alloc memory for stream buffer!\n");
        goto exit;
    }

    while (g_thread_running) {
        if (!queue_empty(&g_param_queue) && !queue_empty(&g_frame_queue)) {
            queue_pop(&g_param_queue, &crop_params, sizeof(TransformParams));
            queue_pop(&g_frame_queue, &frame, sizeof(VPSS_FRAME_INFO_S));

            if (print_cnt % 100 == 0) {
                printf("Processing frame with timestamp %lu  framets:%lu\n", crop_params.timestamp,frame.stFrame.u64Ts);
                print_cnt = 0;
            }

            print_cnt++;
        } else {
            usleep(10000);
            continue;
        }

        // Data conversion to DMA format
        data2dma(IPU_COLOR_FMT_SEMIPLANE_NV12, g_video_param.vi_width, g_video_param.vi_height,
                 frame.u64PhyAddr, (void*)frame.stFrame.pu8Data, &stIpuDmaSrc);
        init_surface(&stIpuDmaSrc, &stSurfaceSrc);
        stSurfaceSrc.u8ColorPadding = 0x1;

        i_count++;

        // Execute IPU transform
        AK_MPI_IPU_StartQue(0);
        if( ( g_video_param.vi_width != g_video_param.out_width ) || ( g_video_param.vi_height != g_video_param.out_height ) ) { // If output layer needs scaling, create a temporary intermediate layer
            AK_MPI_IPU_Trans4( 0, &stSurfaceSrc, &stSurfaceMid, &crop_params.stIpuDmaTrans );
            AK_MPI_IPU_CompositeResize( 0, &stSurfaceMid, &stIpuRectMid, &stSurfaceDst, &stIpuRectDst );
        }
        else {
            AK_MPI_IPU_Trans4( 0, &stSurfaceSrc, &stSurfaceDst, &crop_params.stIpuDmaTrans );
        }
        ret = AK_MPI_IPU_EndQue( 0, AK_TRUE, AK_TRUE );
        if(ret == AK_FAILED)
        {
            AK_MPI_IPU_Reset();printf("%s_%d vno=%" PRIu64 " \n",__FUNCTION__, __LINE__, frame.stFrame.u64Ts);
            IPU_Dma_Free(&crop_params.stIpuDmaTrans);printf("%s_%d vno=%" PRIu64 " \n",__FUNCTION__, __LINE__, frame.stFrame.u64Ts);
            AK_MPI_VPSS_ReleaseFrame(g_vpss_grp, g_vpss_chn, &frame);
            continue;
        }
        // Update frame data
        VPSS_FRAME_INFO_S vpss_frame = frame;
        vpss_frame.stFrame.pu8Data = (uint8_t*)stIpuDmaDst.pvAddr;
        vpss_frame.u64PhyAddr = stIpuDmaDst.u64PhyAddr;
        vpss_frame.stFrame.u32Len = stIpuDmaDst.u32Len;

        // Send to encoder
        // video_send_to_encode(0, &vpss_frame);
        ret = AK_MPI_VENC_EncodeFrame(venc_handle_id, vpss_frame.stFrame.pu8Data,    \
                                        vpss_frame.stFrame.u32Len, NULL, pstStream);
        pstStream->u64Ts = vpss_frame.stFrame.u64Ts;
        pstStream->u64SeqNo = vpss_frame.stFrame.u64SeqNo;

        if(ret == AK_SUCCESS) {
            if(pstStream->enFrameType ==FRAME_TYPE_I)
            {
                printf("---venc%d_th Get Stream! type:%s len:%8u ts:%12lu ts_diff:%lu fps %ld\n",
                            venc_handle_id,
                            pstStream->enFrameType==FRAME_TYPE_P?"P":
                            pstStream->enFrameType==FRAME_TYPE_I?"I":"P",
                            pstStream->u32Len,pstStream->u64Ts,pstStream->u64Ts-prev_ts,1000/(pstStream->u64Ts-prev_ts));
            }
            prev_ts = pstStream->u64Ts;

            if(video_call_back_func)
                video_call_back_func(venc_handle_id, pstStream);

            int iRet = AK_MPI_VENC_ReleaseStream(venc_handle_id, pstStream);
       }
       else {
            if(ret != ERROR_EXT_VIDEO_ENCODRE_HAS_NO_DATA && ret != ERROR_VENC_GET_STREAM_FAILED)
                AK_BASE_PrintErrorEx(MODULE_ID_APP,
                                    "Video get stream failed .venc_handle_id:%d ret:%d %3x\n",
                                    venc_handle_id,ret,ret);

            AK_BASE_SleepMs(10);
        }

		int iRet = 0;
        // Free IPU DMA
        iRet = IPU_Dma_Free(&crop_params.stIpuDmaTrans);

        // Free video frame
        iRet = AK_MPI_VPSS_ReleaseFrame(g_vpss_grp, g_vpss_chn, &frame);
        i_count++;
    }
exit:
    if(pstStream != NULL) {
        AK_BASE_MemFree(pstStream);
    }

    return NULL;
}

// Initialization function
int eis_init(int vpss_grp, int vpss_chn, int vi_w,int vi_h, int out_width, int out_height,int frame_depth, VideoCallBackFunc video_cb)
{
    int ret = 0;
    IPU_ATTR_S stIpuAttr = {0};

    gyro_init(1);

    // Pass in channel information
    g_vpss_grp = vpss_grp;
    g_vpss_chn = vpss_chn;
    g_frame_depth = frame_depth;
    video_call_back_func = video_cb;

    g_video_param.vi_width = vi_w;
    g_video_param.vi_height = vi_h;
    g_video_param.out_width = out_width;
    g_video_param.out_height = out_height;

    // Open IPU
    stIpuAttr.au32QueSize[ 0 ] = 8192;                                          // Set command buffer length
    stIpuAttr.au32QueSize[ 1 ] = 8192;
    stIpuAttr.u32RespSize = 8192;                                               // Response queue length
    if( AK_MPI_IPU_Open( &stIpuAttr ) != AK_SUCCESS )
    {
        AK_BASE_PrintErrorEx( MODULE_ID_APP," AK_MPI_IPU_Open error.\n" );
        return -1;
    }
    AK_MPI_IPU_Reset();
    // Initialize queues
    queue_init(&g_quat_queue);
    queue_init(&g_frame_queue);
    queue_init(&g_frame_pts_queue);
    queue_init(&g_param_queue);

    // Initialize mutex
    AK_BASE_ThreadMutexInit(&g_data_mutex, NULL);

    // Create EIS algorithm objects
    g_integrator = integrator_create();
    g_smoother = smoother_create();


    printf("EIS system initialized\n");
    return 0;
}

// Start thread function
int eis_start_threads()
{
    int ret = 0;
    g_thread_running = true;

    // Create gyro data acquisition thread
    ret = AK_BASE_ThreadCreate(&g_eis_data_thread, run_get_eis_data, NULL,
                               ANYKA_THREAD_NORMAL_STACK_SIZE, ANYKA_THREAD_PRIO_HIGH);
    if (ret != 0) {
        printf("Failed to create EIS data thread: %d\n", ret);
        return -1;
    }

    // Create EIS processing thread
    ret = AK_BASE_ThreadCreate(&g_eis_process_thread, run_eis_process, NULL,
                               ANYKA_THREAD_NORMAL_STACK_SIZE, ANYKA_THREAD_PRIO_HIGH);
    if (ret != 0) {
        printf("Failed to create EIS process thread: %d\n", ret);
        return -1;
    }

    // // Create crop EIS processing thread
    ret = AK_BASE_ThreadCreate(&g_crop_eis_thread, run_crop_eis_process, NULL,
                               ANYKA_THREAD_NORMAL_STACK_SIZE, ANYKA_THREAD_PRIO_HIGH);
    if (ret != 0) {
        printf("Failed to create crop EIS thread: %d\n", ret);
        return -1;
    }

    printf("All EIS threads started successfully\n");
    return 0;
}

// Stop thread function
int eis_stop_threads()
{
    // Set thread stop flag
    g_thread_running = false;
    extern int gyro_fd;
    if(gyro_fd > 0)
    {
     close(gyro_fd);
     gyro_fd = -1;
    }
    printf("%s_%d \n", __FUNCTION__, __LINE__);
    // Wait for threads to finish
    AK_BASE_ThreadJoin(g_eis_data_thread);
    printf("%s_%d \n", __FUNCTION__, __LINE__);
    AK_BASE_ThreadJoin(g_eis_process_thread);
    printf("%s_%d \n", __FUNCTION__, __LINE__);
    AK_BASE_ThreadJoin(g_crop_eis_thread);

    printf("All EIS threads stopped\n");
    return 0;
}

// Cleanup function
void eis_cleanup()
{
    // Destroy queues
    queue_destroy(&g_quat_queue);
    queue_destroy(&g_frame_queue);
    queue_destroy(&g_frame_pts_queue);
    queue_destroy(&g_param_queue);

    // Destroy mutex
    AK_BASE_ThreadMutexDestroy(&g_data_mutex);

    // Destroy stabilization algorithm objects
    if (g_integrator) {
        integrator_destroy(g_integrator);
    }
    if (g_smoother) {
        smoother_destroy(g_smoother);
    }

    // Close IPU
    AK_MPI_IPU_Close();

    printf("EIS system cleaned up\n");
}
