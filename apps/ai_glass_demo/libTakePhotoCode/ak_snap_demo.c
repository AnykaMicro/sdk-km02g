/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd. */
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <errno.h>

#include "media_configs.h"

#include "ak_snap_demo.h"
#include "ak_dma_api.h"

#include "ak_common.h"
#include "ak_log.h"
#include "ak_thread.h"

#include "ak_common_video.h"
#include "ak_venc.h"
#include "ak_mem.h"
#include "ak_vi.h"
#include "ak_vpss.h"
#include "ak_ipu.h"

#include "isp_quick_ioctl.h"
#include "ai_glass_cmd_ipi.h"
#include "ai_glass_common.h"
#include "fw_media_types.h"
#include "video.h"


#define FAST_PIC_SAVE_MODE
// #define SAVE_YUV_FRAME_FILE
#define JPG_DROP_FRAME_NUM  3 //(PHOTO_FPS-5)//Frame drop count before photo (wait for stability)

static IPU_RECT_S stRectSrc[3] = {0};
static IPU_RECT_S stRectDst = {0};

static VI_RECTANGLE_S stViRectangle = {0};

static IPU_DMA_S stIpuDmaSrc = {0};
static IPU_DMA_S stIpuDmaDst = {0};
static IPU_SURFACE_S stSurfaceSrc = {0};
static IPU_SURFACE_S stSurfaceDst = {0};
//static IPU_RECT_S stRectSrc[gm_config.photo_param.main_jpg_slice_num] = {{0, 0, gm_config.photo_param.main_org_width, gm_config.photo_param.main_org_slice_one_height},
//                                                         {0, gm_config.photo_param.main_org_slice_one_height, gm_config.photo_param.main_res_width, gm_config.photo_param.main_org_slice_one_height},
//                                                         {0, (gm_config.photo_param.main_org_slice_one_height * 2), gm_config.photo_param.main_org_width, gm_config.photo_param.main_org_slice_one_height}};

//static IPU_RECT_S stRectDst = {0, 0, gm_config.photo_param.main_res_width, gm_config.photo_param.main_jpg_slice_one_height};

//static int g_fps = gm_config.photo_param.main_chn_fps;
static int g_frame_depth = 1;


void update_rect_config()
{

    stRectSrc[0].s32X = 0;
    stRectSrc[0].s32Y = 0;
    stRectSrc[0].u32W = gm_config.photo_param.main_org_width;
    stRectSrc[0].u32H = gm_config.photo_param.main_org_slice_one_height;

    stRectSrc[1].s32X = 0;
    stRectSrc[1].s32Y = gm_config.photo_param.main_org_slice_one_height;
    stRectSrc[1].u32W = gm_config.photo_param.main_res_width;
    stRectSrc[1].u32H = gm_config.photo_param.main_org_slice_one_height;


    stRectSrc[2].s32X = 0;
    stRectSrc[2].s32Y = (gm_config.photo_param.main_org_slice_one_height * 2);
    stRectSrc[2].u32W = gm_config.photo_param.main_org_width;
    stRectSrc[2].u32H = gm_config.photo_param.main_org_slice_one_height;



    // Update stRectDst
    stRectDst.s32X = 0;
    stRectDst.s32Y = 0;
    stRectDst.u32W = gm_config.photo_param.main_res_width;
    stRectDst.u32H = gm_config.photo_param.main_jpg_slice_one_height;
}
// struct snap_date
// {
//     int year;     // 4 number
//     int mon;      // 1-12
//     int day;      // 1-31
//     int hour;     // 0-23
//     int min;      // 0-59
//     int sec;      // 0-59
//     int timezone; // local time zone 0-23

//     struct timeval timeval_val;   // tv_sec tv_usec
//     struct timezone timezone_val; // timezone
//     struct tm tm_val;             // tm_year tm_mon tm_mday tm_hour tm_min tm_sec
// };

// static int snap_get_localdate(struct snap_date *p_snap_date) // Get current time
// {
//     if (p_snap_date == NULL)
//     {
//         return AK_FAILED;
//     }

//     gettimeofday(&p_snap_date->timeval_val, &p_snap_date->timezone_val);
//     localtime_r(&(p_snap_date->timeval_val.tv_sec), &p_snap_date->tm_val);
//     p_snap_date->year = p_snap_date->tm_val.tm_year + 1900;       // year
//     p_snap_date->mon = p_snap_date->tm_val.tm_mon + 1;            // month
//     p_snap_date->day = p_snap_date->tm_val.tm_mday;               // day
//     p_snap_date->hour = p_snap_date->tm_val.tm_hour;              // hour
//     p_snap_date->min = p_snap_date->tm_val.tm_min;                // min
//     p_snap_date->sec = p_snap_date->tm_val.tm_sec;                // sec
//     p_snap_date->timezone = p_snap_date->timezone_val.tz_dsttime; // timezone

//     return AK_SUCCESS;
// }

static int wait_for_file(unsigned char *wait_path)
{
    int retries = 0;

    while (retries < 1000)
    {
        // Check if script exists
        if (access(wait_path, F_OK) == 0)
        {
            printf("Found: %s retries %d\n", wait_path, retries);
            return 0; // Success
        }

        // Wait for a while (milliseconds)
        usleep(5000); // usleep accepts microseconds

        retries++;
    }

    printf("Found %s timeout .\n", wait_path);
    return -1; // Failed
}


static int destory_ipu()
{
    if (stIpuDmaDst.pvAddr)
    {
        IPU_Dma_Free(&stIpuDmaDst, 0);
    }

    AK_MPI_IPU_Close();

    return 0;
}

static int init_ipu()
{
    IPU_ATTR_S stIpuAttr = {0};
    stIpuAttr.au32QueSize[0] = 8192; // Set command buffer length
    stIpuAttr.au32QueSize[1] = 8192;
    stIpuAttr.u32RespSize = 8192; // Receipt queue length
    AK_MPI_IPU_Open(&stIpuAttr);
    AK_MPI_IPU_Reset();

    stIpuDmaSrc.u32W = gm_config.photo_param.main_org_width;
    stIpuDmaSrc.u32H = gm_config.photo_param.main_org_height;
    stIpuDmaSrc.enColorFmt = IPU_COLOR_FMT_SEMIPLANE_NV12;

    stIpuDmaDst.u32W = gm_config.photo_param.main_res_width;
    stIpuDmaDst.u32H = gm_config.photo_param.main_jpg_slice_one_height;
    stIpuDmaDst.enColorFmt = IPU_COLOR_FMT_SEMIPLANE_NV12;

    init_dma_img(&stIpuDmaSrc, AK_FALSE, AK_FALSE);
    init_dma_img(&stIpuDmaDst, AK_FALSE, AK_TRUE);
    // init_surface( &stIpuDmaSrc, &stSurfaceSrc );
    init_surface(&stIpuDmaDst, &stSurfaceDst);

    return 0;
}

static int init_venc(AK_S32 s32ChnIndex)
{
    AK_S32 s32Ret = -1;
    VENC_CHN_ATTRS_S stChnAttrs = {0};
    VENC_ATTR_S *pstVencAttrs = &stChnAttrs.stVencAttrs;    // venc attr
    VENC_RC_ATTRS_S *pstRcAttrs = &stChnAttrs.stRcAttrs;    // rc attr
    VENC_EXT_ATTRS_S *pstExtAttrs = &stChnAttrs.stExtAttrs; // ext attr

    // pstVencAttrs
    pstVencAttrs->u16Width = gm_config.photo_param.main_res_width;   // s32Width;                      //width
    pstVencAttrs->u16Height = gm_config.photo_param.main_res_height; // s32Height;                  //height
    pstVencAttrs->enOutputType = MJPEG_ENC_TYPE;     // output type
    pstVencAttrs->u32MaxBufSize = 6 * 1024;          // ext buf size, KBytes
    pstVencAttrs->u32MaxDMASize = 0;                 // inner buf size, KBytes
    pstVencAttrs->enChromaMode = VENC_CHROMA_YUV420; // yuv420sp
    pstVencAttrs->u32Priority = 0;                   // priority

    pstVencAttrs->stH264Attr.u16Framerate = 10;
    pstVencAttrs->stH264Attr.u16GopLen = 10;
    pstVencAttrs->stH264Attr.enGdrMode = 0;
    pstVencAttrs->stH264Attr.enProfileMode = VENC_PROFILE_JPEG;
    pstVencAttrs->stH264Attr.enEncLevel = VENC_HEVC_AVC_LEVEL_5;

    // pstRcAttrs
    pstRcAttrs->enRcMode = VENC_RC_MODE_MJPEG; // rc mode
    pstRcAttrs->stMjpegRc.u16Quality = 90;     // quality

    // pstExtAttrs
    pstExtAttrs->stAdvance.enCuBlock = VENC_CUSize32x32; // CUSize
    for (AK_S32 i = 0; i < 3; i++)
    {
        /* I/P/B min and max qp */
        pstExtAttrs->stAdvance.u16MinQP[i] = 20;
        pstExtAttrs->stAdvance.u16MaxQP[i] = 48;
    }
    pstExtAttrs->stAdvance.s16IPDelta = 0;         // Delta
    pstExtAttrs->stAdvance.u32MaxPictureISize = 0; // MAX I Frame, Kbits
    pstExtAttrs->stAdvance.u32MaxPicturePSize = 0; // MAX P Frame, Kbits
    pstExtAttrs->stAdvance.u32MaxPictureBSize = 0; // Max B Frame, Kbits
    pstExtAttrs->stAdvance.bEnableSRD = AK_FALSE;  // SRD
    pstExtAttrs->stAdvance.u8SRDThreshold = 0;     // SRDThreshold
    pstExtAttrs->stAdvance.bEnableMMA = AK_TRUE;   // Minimum Memory Allocation mode

    s32Ret = AK_MPI_VENC_Open(s32ChnIndex, &stChnAttrs);
    if (s32Ret)
    {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "dev%d open venc failed\n", s32ChnIndex);
        return s32Ret;
    }

    return 0;
}

static int destory_venc(AK_S32 s32ChnIndex)
{
    AK_S32 s32Ret = 0;
    s32Ret = AK_MPI_VENC_Close(s32ChnIndex);
    if (s32Ret)
    {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "dev%d close venc failed\n", s32ChnIndex);
        return s32Ret;
    }

    return 0;
}

static int start_vi(AK_S32 s32GrpIndex)
{
    AK_S32 ret = 0;

    ret = AK_MPI_VI_EnableDev(s32GrpIndex);
    if (ret)
    {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "enable vi device fail \n");
        return -1;
    }

    ret = AK_MPI_VI_EnableChn(s32GrpIndex, 0);
    if (ret)
    {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "enable vi channel fail \n");
        return -1;
    }

    ret = AK_MPI_VPSS_StartGrp(s32GrpIndex);
    if (ret)
    {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "start vpss grp fail \n");
        return -1;
    }
    /* main channel */
    ret = AK_MPI_VPSS_EnableChn(s32GrpIndex, 0);
    if (ret)
    {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "enable vpss chn fail \n");
        return -1;
    }

    return 0;
}

static int init_vi(AK_U32 u32GrpId, AK_CHAR *isp_conf_path)
{
    AK_S32 ret = 0;
    VI_DEV_ATTR_S stViDevAttr = {0};
    VPSS_GRP_ATTR_S stGrpAttr = {0};

    /*
     * step 1: init vi
     */
    stViDevAttr.u32Width = 320;
    stViDevAttr.u32Height = 184;
    stViDevAttr.bFlip = 0;
    stViDevAttr.enIntfMode = (VI_INTF_MODE_E)VI_INTF_MIPI_2;
    // if(g_s32DevCnt > 1)
    //     stViDevAttr.enIntfMode = (VI_INTF_MODE_E)VI_INTF_MULTI_MIPI_2;
    stViDevAttr.bMirror = 0;
    stViDevAttr.u32Fps = gm_config.photo_param.main_chn_fps;
    stViDevAttr.stCropInfo.u32PointX = 0;
    stViDevAttr.stCropInfo.u32PointY = 0;
    stViDevAttr.stCropInfo.u32Width = stViDevAttr.u32Width;   // sensor resolution
    stViDevAttr.stCropInfo.u32Height = stViDevAttr.u32Height; // sensor resolution
    ret = AK_MPI_VI_CreateDev(u32GrpId, &stViDevAttr);
    if (ret)
    {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "Create vi device fail \n");
        return -1;
    }
    ret = AK_MPI_VI_GetSensorRes(u32GrpId, &stViRectangle);
    if (ret)
    {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "Get sensor res fail \n");
        return -1;
    }
    printf("#####stViRectangle u32Width::%u u32Height:%u\n",stViRectangle.u32Width,stViRectangle.u32Height);

    if (stViRectangle.u32Width > 3840)
        stViRectangle.u32Width = 3840;

    if (stViRectangle.u32Height > 2880)
        stViRectangle.u32Height = 2880;

    /*
     * step 1: create vi dev
     */
    stViDevAttr.u32Width = stViRectangle.u32Width;
    stViDevAttr.u32Height = stViRectangle.u32Height;
    stViDevAttr.stCropInfo.u32Width = stViRectangle.u32Width;
    stViDevAttr.stCropInfo.u32Height = stViRectangle.u32Height;
    ret = AK_MPI_VI_SetDevAttr(u32GrpId, &stViDevAttr);
    if (ret)
    {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "set device attr fail \n");
        return -1;
    }

    /*
     * step 2: set vi chn attr
     */
    VI_CHN_ATTR_S stViChnAttr = {0};
    stViChnAttr.enMode = VI_CHN_MODE_AUTO; // only user mode can get raw data.
    stViChnAttr.enOutputPixFmt = PIXEL_FORMAT_YUV420SP;
    stViChnAttr.u32Fps = gm_config.photo_param.main_chn_fps; // no framerate ctrl.
    stViChnAttr.u32FrameDepth = g_frame_depth;
    stViChnAttr.u32Width = stViRectangle.u32Width; // the same res to device.
    stViChnAttr.u32Height = stViRectangle.u32Height;
    stViChnAttr.u32MaxPerFrameSize = 0; // use default value.
    ret = AK_MPI_VI_SetChnAttr(u32GrpId, 0, &stViChnAttr);
    if (ret)
    {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "Set vi chn attr fail \n");
        return -1;
    }

    /*
     * step 2: init vpss
     */
    stGrpAttr.u32Width = stViRectangle.u32Width;
    stGrpAttr.u32Height = stViRectangle.u32Height;
    stGrpAttr.bEnGrpBypass = 0;
    stGrpAttr.bFlip = 0;
    stGrpAttr.bMirror = 0;
    stGrpAttr.enInputPixFmt = PIXEL_FORMAT_YUV420SP;
    stGrpAttr.enWorkMode = VPSS_WORK_AUTO;
    strcpy(stGrpAttr.pu8IspCfgPath, isp_conf_path);
    AK_U32 u32SensorId = 0;
    AK_MPI_VI_GetSensorId(u32GrpId, &u32SensorId);
    stGrpAttr.s32AttachSensorId = u32SensorId;
    ret = AK_MPI_VPSS_CreateGrp(u32GrpId, &stGrpAttr);
    if (ret)
    {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "Create vpss Grp fail \n");
        return -1;
    }

    VPSS_CHN_ATTR_S stVpssChnAttr = {0};
    stVpssChnAttr.u32Width = gm_config.photo_param.main_org_width;   // stViRectangle.u32Width;
    stVpssChnAttr.u32Height = gm_config.photo_param.main_org_height; // stViRectangle.u32Height;
    stVpssChnAttr.enChnMode = VPSS_CHN_FRAME_MODE;
    stVpssChnAttr.enOutputPixFmt = PIXEL_FORMAT_YUV420SP;
    stVpssChnAttr.stCropInfo.u32Width = stViRectangle.u32Width;
    stVpssChnAttr.stCropInfo.u32Height = stViRectangle.u32Height;
    stVpssChnAttr.stCropInfo.u32PointX = 0;
    stVpssChnAttr.stCropInfo.u32PointY = 0;
    stVpssChnAttr.u32MaxBufSize = 0;
    stVpssChnAttr.s32Fps = gm_config.photo_param.main_chn_fps;
    stVpssChnAttr.u32FrameDepth = g_frame_depth;
    /* main channel */
    ret = AK_MPI_VPSS_SetChnAttr(u32GrpId, 0, &stVpssChnAttr);
    if (ret)
    {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "set vpss main chn attr fail \n");
        return -1;
    }

    /*
     * step 3: bind vi vpss
     */
    ret = AK_MPI_VI_BindVpssGrp(u32GrpId, 0, u32GrpId);
    if (ret)
    {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "bind vpss grp fail \n");
        return ret;
    }
    ret = AK_MPI_VPSS_BindChn(u32GrpId, 0);
    if (ret)
    {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "bind vi device fail \n");
        return ret;
    }

    return AK_SUCCESS;
}

static int stop_and_destory_vi(AK_U32 u32GrpId)
{
    AK_S32 ret = AK_FAILED;

    ret = AK_MPI_VPSS_DisableChn(u32GrpId, 0);
    if (ret)
    {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "Disable vpss chn1 fail .\n");
        return AK_FAILED;
    }

    ret = AK_MPI_VPSS_StopGrp(u32GrpId);
    if (ret)
    {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "AK_MPI_VPSS_StopGrp fail \n");
        return ret;
    }

    ret = AK_MPI_VI_DisableChn(u32GrpId, 0);
    if (ret)
    {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "Disable vi chn fail .\n");
        return -1;
    }

    ret = AK_MPI_VI_DisableDev(u32GrpId);
    if (ret)
    {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "Disable vi device fail .\n");
        return -1;
    }

    ret = AK_MPI_VPSS_DestroyGrp(u32GrpId);
    if (ret)
    {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "AK_MPI_VPSS_DestroyGrp fail \n");
        return ret;
    }

    ret = AK_MPI_VI_DestroyDev(u32GrpId);
    if (ret)
    {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "Destroy vi device fail .\n");
        return -1;
    }

    return 0;
}

void ak_snap_photo_get_default_file_name(char *file_name)
{
    DATE_S sDate = {0};
    AK_BASE_GetLocalDate(&sDate);

    sprintf(file_name, "%04d%02d%02d%02d%02d%02d.jpeg",
        sDate.s32Year,sDate.s32Month,sDate.s32Day,
        sDate.s32Hour,sDate.s32Minute,sDate.s32Second);
}

void ak_snap_photo_get_default_frame_file_name(char *file_name)
{
    DATE_S sDate = {0};
    AK_BASE_GetLocalDate(&sDate);

    sprintf(file_name, "%04d%02d%02d%02d%02d%02d.yuv",
        sDate.s32Year,sDate.s32Month,sDate.s32Day,
        sDate.s32Hour,sDate.s32Minute,sDate.s32Second);
}

int ak_snap_photo(const char *photo_save_path)
{
    uint8_t result = RETURN_CODE_SUCCESS ;
    uint8_t glass_info_param[256] = {0};
    uint16_t glass_info_param_len = 0;

    VPSS_FRAME_INFO_S vpss_frame = {0};
    // struct snap_date snap_date = {0};
    FILE *save_fp = NULL;
    char file_name[32] = {0};
    char file_name_with_path[128] = {0};
    unsigned int file_len = 0;
    unsigned int frame_count = 0;
    AK_S32 s32Ret = 0;

    ISP_AE_RUN_INFO_S stAeRunInfo = {0};
    ISP_AE_ATTR_S stAeAttr = {0};
    EXIF_INFO setExifInfo = {0};

#ifdef SAVE_YUV_FRAME_FILE
    FILE *save_fp_yuv = NULL;
    char file_name_yuv[32] = {0};
    char file_name_with_path_yuv[128] = {0};
#endif

#ifdef FAST_PIC_SAVE_MODE
    char file_name_with_path_tmp[128] = {0};
#else
    // Check path
    wait_for_file("/mnt/Pictures");
    ensure_directory_exists(photo_save_path);
#endif

    ak_snap_photo_get_default_file_name(file_name);

    sprintf(file_name_with_path,"%s/%s",
                photo_save_path,file_name);
#ifdef FAST_PIC_SAVE_MODE
    sprintf(file_name_with_path_tmp,"%s/%s",
                "/tmp",file_name);
#endif

#ifdef SAVE_YUV_FRAME_FILE
    ak_snap_photo_get_default_frame_file_name(file_name_yuv);

    sprintf(file_name_with_path_yuv,"%s/%s",
                photo_save_path,file_name_yuv);
#endif

    if(gm_config.photo_param.main_org_width != gm_config.photo_param.main_res_width || gm_config.photo_param.main_org_height != gm_config.photo_param.main_res_height)
    {
        AK_MPI_VENC_SetJpegSlice(0, gm_config.photo_param.main_res_height, gm_config.photo_param.main_jpg_slice_one_height);
    }

DROP_FRAME_WHILE:
    s32Ret = AK_MPI_VPSS_GetFrame(0, 0, &vpss_frame, 2000);
    if (!s32Ret)
    {
        frame_count++;
		if(frame_count == 1)
		{
            result = RETURN_CODE_SUCCESS;
            memset(glass_info_param, 0, sizeof(glass_info_param));
            glass_info_param_len = ai_glass_info_pack_data(result, glass_info_param, NULL, 0);
            if(is_bit7_set(global_cmd_id)){
                set_glass_info_v4(APP_CMD_PHOTO, (char *)glass_info_param, glass_info_param_len);
            }else{

                set_glass_info_v4(BT_CMD_PHOTO, (char *)glass_info_param, glass_info_param_len);
            }
            printf("get vpss frame ok (framet=%" PRIu64 ",syst=%lld) \n", vpss_frame.stFrame.u64Ts, get_sys_run_time64());
		}
        if(frame_count <= JPG_DROP_FRAME_NUM)
        {
            printf("frame_count:%d <= %d, drop it(framet=%" PRIu64 ",syst=%lld) \n",frame_count,JPG_DROP_FRAME_NUM, vpss_frame.stFrame.u64Ts, get_sys_run_time64());
            AK_MPI_VPSS_ReleaseFrame(0, 0, &vpss_frame);
            goto DROP_FRAME_WHILE;
        }

		 // Check path
		wait_for_file((unsigned char*)photo_save_path);
		ensure_directory_exists(photo_save_path);
		//int result = 0;
		//set_glass_info_v4(glass_fd, CMD_PHOTO,(char *)&result,sizeof(int));
		//printf("get vpss frame ok (framet=%" PRIu64 ",syst=%lld) \n", vpss_frame.stFrame.u64Ts, get_sys_run_time64());

        VIDEO_STREAM_S *stStream = AK_BASE_MemAlloc(MODULE_ID_APP, sizeof(VIDEO_STREAM_S));
        memset(stStream,0,sizeof(VIDEO_STREAM_S));

#ifdef SAVE_YUV_FRAME_FILE
        save_fp_yuv = fopen(file_name_with_path_yuv, "w+");
        if (NULL == save_fp_yuv)
        {
            AK_BASE_PrintErrorEx(MODULE_ID_APP, "fopen %s failed!\n", file_name_with_path_yuv);
            return -1;
        }
        else
        {
            // fwrite(vpss_frame.stFrame.pu8Data,vpss_frame.stFrame.u32Len, 1, save_fp_yuv);
            fwrite(vpss_frame.stFrame.pu8Data,gm_config.photo_param.main_org_width*gm_config.photo_param.main_org_height*3/2, 1, save_fp_yuv);
            fflush(save_fp_yuv);
            fclose(save_fp_yuv);
            save_fp_yuv = NULL;
        }
#endif

#ifdef FAST_PIC_SAVE_MODE
        save_fp = fopen(file_name_with_path_tmp, "w+");
#else
        save_fp = fopen(file_name_with_path, "w+");
#endif
        if (NULL == save_fp)
        {
#ifdef FAST_PIC_SAVE_MODE
            AK_BASE_PrintErrorEx(MODULE_ID_APP, "fopen %s failed!\n", file_name_with_path_tmp);
#else
            AK_BASE_PrintErrorEx(MODULE_ID_APP, "fopen %s failed!\n", file_name_with_path);
#endif
            return -1;
        }

        // AK_BASE_PrintNormalEx(MODULE_ID_VENC, "get one yuv frame. ts:%llu\n",vpss_frame.stFrame.u64Ts);
        printf("[test]%dx%d->%dx%d\n",gm_config.photo_param.main_org_width,gm_config.photo_param.main_org_height,gm_config.photo_param.main_res_width,gm_config.photo_param.main_res_height);

        // Embed ISO speed and exposure time information
	    AK_MPI_ISP_GetAeRunInfo(0, &stAeRunInfo);
	    AK_MPI_ISP_GetAeAttr(0, &stAeAttr);

	    AK_BASE_PrintNormalEx(MODULE_ID_APP, "gain : %u, %u, %u!\n", stAeRunInfo.u32CurrentAGain, stAeRunInfo.u32CurrentDGain, stAeRunInfo.u32CurrentIspDGain);
	    AK_U32 u32Iso = 100 * stAeRunInfo.u32CurrentAGain / 256 * stAeRunInfo.u32CurrentDGain / 256 * stAeRunInfo.u32CurrentIspDGain / 256;
	    AK_BASE_PrintNormalEx(MODULE_ID_APP, "iso : %u!\n", u32Iso);
	    AK_S32 s32Fps = 10;
	    AK_MPI_ISP_GetSensorFps(0, &s32Fps);
	    if (0 == s32Fps)
	        s32Fps = 10;
	    AK_BASE_PrintNormalEx(MODULE_ID_APP, "exptime : %u, %u, fps: %d!\n", stAeRunInfo.u32CurrentExpTime, stAeAttr.u32ExpTimeMax, s32Fps);
	    AK_U32 u32ExptimeMs = 1000 * stAeRunInfo.u32CurrentExpTime / stAeAttr.u32ExpTimeMax / s32Fps;
	    AK_BASE_PrintNormalEx(MODULE_ID_APP, "u32ExptimeMs : %u!\n", u32ExptimeMs);
	    if (0 == u32ExptimeMs)
	        u32ExptimeMs = 1;
	    AK_U32 u32Shutter = 1000 / u32ExptimeMs;

        // Enable
        s32Ret = AK_MPI_VENC_SetExifEnable(0, AK_TRUE);
        if(s32Ret)
        {
            AK_BASE_PrintErrorEx(MODULE_ID_APP, "dev0 set exif enable failed\n");
            return s32Ret;
        }

        memset(&setExifInfo, 0, sizeof(EXIF_INFO));
        setExifInfo.user_tag = USER_EXIF_TAG_EXPOSURE_TIME;
        setExifInfo.value.rational_value.numerator = 1;
        setExifInfo.value.rational_value.denominator = u32Shutter;
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "setExifInfo.value.rational_value.numerator: %d setExifInfo.value.rational_value.denominator: %d!\n", \
                                setExifInfo.value.rational_value.numerator, setExifInfo.value.rational_value.denominator);
        AK_MPI_VENC_SetExifInfo(0, &setExifInfo);

        memset(&setExifInfo, 0, sizeof(EXIF_INFO));
        setExifInfo.user_tag = USER_EXIF_TAG_ISO_SPEED_RATINGS;
        setExifInfo.value.short_value = (AK_U16) u32Iso;
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "setExifInfo.value.short_value: %d !\n", \
                        setExifInfo.value.short_value);
        AK_MPI_VENC_SetExifInfo(0, &setExifInfo);


        if(gm_config.photo_param.main_org_width != gm_config.photo_param.main_res_width || gm_config.photo_param.main_org_height != gm_config.photo_param.main_res_height)
        {
            printf("silce jpg mode frametime=%" PRIu64 " syst=%lld \n", vpss_frame.stFrame.u64Ts, get_sys_run_time64());
            // JPG slice encoding scaling
            stIpuDmaSrc.pvAddr = vpss_frame.stFrame.pu8Data;
            stIpuDmaSrc.u32Len = gm_config.photo_param.main_org_width*gm_config.photo_param.main_org_height*3/2;//vpss_frame.stFrame.u32Len;
            AK_BASE_MemDmaVaddr2Paddr(stIpuDmaSrc.pvAddr, &stIpuDmaSrc.u64PhyAddr);
            stIpuDmaSrc.u32PhyAddrLow = stIpuDmaSrc.u64PhyAddr & 0xFFFFFFFF;   // Low address
            stIpuDmaSrc.u8PhyAddrHigh = (stIpuDmaSrc.u64PhyAddr >> 32) & 0XFF; // High address
            // print_dma( &stIpuDmaSrc );
            init_surface(&stIpuDmaSrc, &stSurfaceSrc);
            for (int i = 0; i < gm_config.photo_param.main_jpg_slice_num; i++)
            {
                AK_MPI_IPU_StartQue(0);
                AK_MPI_IPU_CompositeResize(0, &stSurfaceSrc, &stRectSrc[i], &stSurfaceDst, &stRectDst);
                AK_MPI_IPU_EndQue(0, AK_TRUE, AK_TRUE);
                // AK_MPI_IPU_WaitForDone(0, 1000);

                /* send it to encode */
                s32Ret = AK_MPI_VENC_JpegSliceEncode(0, stIpuDmaDst.pvAddr,
                                                    stIpuDmaDst.u32Len, i, gm_config.photo_param.main_jpg_slice_one_height, stStream);
                if (s32Ret)
                {
                    /* send to encode failed */
                    AK_BASE_PrintErrorEx(MODULE_ID_APP, "send to encode failed\n");
                }
                else
                {
                    if (gm_config.photo_param.main_jpg_slice_num - 1 == i)
                    {
                        AK_BASE_PrintErrorEx(MODULE_ID_VENC, "Photo len:%u.\n", stStream->u32Len);
                        if (stStream->u32Len > 0)
                        {
                            if (fwrite(stStream->pu8Data, stStream->u32Len, 1, save_fp) <= 0)
                            {
                                AK_BASE_PrintErrorEx(MODULE_ID_VENC, "fwrite failed \n");
                            }

                            file_len += stStream->u32Len;
                            AK_MPI_VENC_ReleaseStream(0, stStream);
                            AK_BASE_MemFree(stStream);
                        }
                        else
                            AK_BASE_PrintNoticeEx(MODULE_ID_VENC, "encode err.s32Ret:%d %#x\n", s32Ret, s32Ret);
                    }
                    else
                        AK_BASE_PrintNoticeEx(MODULE_ID_VENC, "encode one slice.\n");
                }
            }
        }
        else
        {
            // Full frame encoding
            printf("frame jpg mode frametime=%" PRIu64 " syst=%lld \n", vpss_frame.stFrame.u64Ts, get_sys_run_time64());
            s32Ret = AK_MPI_VENC_EncodeFrame(0,vpss_frame.stFrame.pu8Data,gm_config.photo_param.main_org_width*gm_config.photo_param.main_org_height*3/2, NULL, stStream);
            if(!s32Ret && stStream->u32Len > 0)
            {
                if (fwrite(stStream->pu8Data, stStream->u32Len, 1, save_fp) <= 0)
                {
                    AK_BASE_PrintErrorEx(MODULE_ID_VENC, "fwrite failed \n");
                }
                else
                {
                    AK_BASE_PrintNoticeEx(MODULE_ID_VENC, "encode jpg ok len:%u.\n",stStream->u32Len);
                    file_len += stStream->u32Len;
                }

            }
            else
            {
                AK_BASE_PrintErrorEx(MODULE_ID_APP, "send to encode failed\n");
            }
        }

        printf("enc write ok frametime=%" PRIu64 " syst=%lld \n", vpss_frame.stFrame.u64Ts, get_sys_run_time64());

        AK_MPI_VPSS_ReleaseFrame(0, 0, &vpss_frame);
        AK_MPI_VENC_SetExifEnable(0, AK_FALSE);

        if (NULL != save_fp)
        {
            fflush(save_fp);
            fclose(save_fp);
            save_fp = NULL;
            if(file_len > 0)
            {
#ifdef FAST_PIC_SAVE_MODE
                // Wait for card ready
                s32Ret = wait_for_file("/mnt/Pictures");
                if(!s32Ret)
                {
                    // Copy picture to sdnand
                    char mv_cmd[256] = {0};
                    // snprintf(mv_cmd,sizeof(mv_cmd),"mv %s %s;sync",
                    //         file_name_with_path_tmp,file_name_with_path);
                    snprintf(mv_cmd,sizeof(mv_cmd),"mv %s %s",
                            file_name_with_path_tmp,file_name_with_path);
                    printf("run cmd:%s\n",mv_cmd);
                    system(mv_cmd);
                }
#endif
                AK_BASE_PrintNormalEx(MODULE_ID_VENC, "########save pic ok! path:%s\n", file_name_with_path);
            }
            else
            {
                remove(file_name_with_path);
                AK_BASE_PrintNormalEx(MODULE_ID_VENC, "########pic len is 0 ! remove path:%s\n", file_name_with_path);
            }
        }
    }
    else
    {
        AK_BASE_PrintErrorEx(MODULE_ID_VENC, "AK_MPI_VPSS_GetFrame failed! %d %#x\n", s32Ret, s32Ret);
    }



    return 0;
}

int ak_snap_env_init(char *isp_conf_path, int i_fps, int i_frame_depth)
{
    // for test only
    int ret = 0;

    gm_config.photo_param.main_chn_fps = i_fps;
    g_frame_depth = i_frame_depth;

    // Initialize rectangle area configuration
    update_rect_config();

    ret = init_vi(0, isp_conf_path);
    if(ret != 0)
    {
        printf("init_vi failed! ret:%d\n", ret);
        return ret;
    }

    ret = start_vi(0);
    if(ret != 0)
    {
        printf("start_vi failed! ret:%d\n", ret);
        return ret;
    }

    ret = init_venc(0);
    if(ret != 0)
    {
        printf("init_venc failed! ret:%d\n", ret);
        return ret;
    }

    if(gm_config.photo_param.main_org_width != gm_config.photo_param.main_res_width
        || gm_config.photo_param.main_org_height != gm_config.photo_param.main_res_height)
    {
        ret = init_ipu();
        if(ret != 0)
        {
            printf("init_ipu failed! ret:%d\n", ret);
            return ret;
        }
    }


    return 0;
}

int ak_snap_env_destory()
{
    int ret = 0;

    if(gm_config.photo_param.main_org_width != gm_config.photo_param.main_res_width
        || gm_config.photo_param.main_org_height != gm_config.photo_param.main_res_height)
    {
        ret = destory_ipu();
        if(ret != 0)
        {
            printf("destory_ipu failed! ret:%d\n", ret);
            return ret;
        }
    }

    ret = destory_venc(0);
    if(ret != 0)
    {
        printf("destory_venc failed! ret:%d\n", ret);
        return ret;
    }

    ret = stop_and_destory_vi(0);
    if(ret != 0)
    {
        printf("stop_and_destory_vi failed! ret:%d\n", ret);
        return ret;
    }

    return 0;
}

int get_jpeg_frame_cb(int nHandle, fw_frame_info *pframe, void* pDataCbUser)
{
    static VIDEO_STREAM_S stStream = {0};
	printf("get_jpeg_frame_cb start \n");

	VPSS_FRAME_INFO_S vpss_frame = {0};

	if( stStream.u32Len > 0 )
	{
		AK_MPI_VENC_ReleaseStream(0, &stStream);
		memset(&stStream,0,sizeof(VIDEO_STREAM_S));
	}

    unsigned int file_len = 0;
    unsigned int frame_count = 0;
    AK_S32 s32Ret = 0;

    ISP_AE_RUN_INFO_S stAeRunInfo = {0};
    ISP_AE_ATTR_S stAeAttr = {0};
    EXIF_INFO setExifInfo = {0};

    s32Ret = AK_MPI_VPSS_GetFrame(0, 0, &vpss_frame, 2000);
    if (!s32Ret)
    {
        frame_count++;
		if(frame_count == 1)
		{
			printf("get vpss frame ok (framet=%" PRIu64 ",syst=%lld) \n", vpss_frame.stFrame.u64Ts, get_sys_run_time64());
		}

        //VIDEO_STREAM_S *stStream = AK_BASE_MemAlloc(MODULE_ID_APP, sizeof(VIDEO_STREAM_S));
        //memset(stStream,0,sizeof(VIDEO_STREAM_S));

        // AK_BASE_PrintNormalEx(MODULE_ID_VENC, "get one yuv frame. ts:%llu\n",vpss_frame.stFrame.u64Ts);
        //printf("[test]%dx%d->%dx%d\n",PHOTO_MAIN_ORG_WIDTH,PHOTO_MAIN_ORG_HEIGHT,PHOTO_MAIN_RES_WIDTH,PHOTO_MAIN_RES_HEIGHT);
        if(gm_config.photo_param.main_org_width != gm_config.photo_param.main_res_width ||
			gm_config.photo_param.main_org_height != gm_config.photo_param.main_res_height)
        {
            printf("silce jpg mode frametime=%" PRIu64 " syst=%lld \n", vpss_frame.stFrame.u64Ts, get_sys_run_time64());
            // JPG slice encoding scaling
            stIpuDmaSrc.pvAddr = vpss_frame.stFrame.pu8Data;
            stIpuDmaSrc.u32Len = gm_config.photo_param.main_org_width*gm_config.photo_param.main_org_height*3/2;//vpss_frame.stFrame.u32Len;
            AK_BASE_MemDmaVaddr2Paddr(stIpuDmaSrc.pvAddr, &stIpuDmaSrc.u64PhyAddr);
            stIpuDmaSrc.u32PhyAddrLow = stIpuDmaSrc.u64PhyAddr & 0xFFFFFFFF;   // Low address
            stIpuDmaSrc.u8PhyAddrHigh = (stIpuDmaSrc.u64PhyAddr >> 32) & 0XFF; // High address
            // print_dma( &stIpuDmaSrc );
            init_surface(&stIpuDmaSrc, &stSurfaceSrc);
            for (int i = 0; i < gm_config.photo_param.main_jpg_slice_num; i++)
            {
                AK_MPI_IPU_StartQue(0);
                AK_MPI_IPU_CompositeResize(0, &stSurfaceSrc, &stRectSrc[i], &stSurfaceDst, &stRectDst);
                AK_MPI_IPU_EndQue(0, AK_TRUE, AK_TRUE);
                // AK_MPI_IPU_WaitForDone(0, 1000);

                /* send it to encode */
                s32Ret = AK_MPI_VENC_JpegSliceEncode(0, stIpuDmaDst.pvAddr,
                                                    stIpuDmaDst.u32Len, i, gm_config.photo_param.main_jpg_slice_one_height, &stStream);
                if (s32Ret)
                {
                    /* send to encode failed */
                    AK_BASE_PrintErrorEx(MODULE_ID_APP, "send to encode failed s32Ret=%d i=%d\n", s32Ret, i);
                }
                else
                {
                    if (gm_config.photo_param.main_jpg_slice_num - 1 == i)
                    {
                        AK_BASE_PrintErrorEx(MODULE_ID_VENC, "Photo len:%u.\n", stStream.u32Len);
                        if (stStream.u32Len > 0)
                        {
							pframe->pData = stStream.pu8Data;
							pframe->uiDataLen = stStream.u32Len;

                            file_len += stStream.u32Len;
                            //AK_BASE_MemFree(stStream);
                        }
                        else
                            AK_BASE_PrintNoticeEx(MODULE_ID_VENC, "encode err.s32Ret:%d %#x\n", s32Ret, s32Ret);
                    }
                    else
                        AK_BASE_PrintNoticeEx(MODULE_ID_VENC, "encode one slice.\n");
                }
            }
        }
        else
        {
            // Full frame encoding
            printf("frame jpg mode frametime=%" PRIu64 " syst=%lld \n", vpss_frame.stFrame.u64Ts, get_sys_run_time64());
            s32Ret = AK_MPI_VENC_EncodeFrame(0,vpss_frame.stFrame.pu8Data,gm_config.photo_param.main_org_width*gm_config.photo_param.main_org_height*3/2, NULL, &stStream);
            if(!s32Ret && stStream.u32Len > 0)
            {
	pframe->pData = stStream.pu8Data;
					pframe->uiDataLen = stStream.u32Len;
                    AK_BASE_PrintNoticeEx(MODULE_ID_VENC, "encode jpg ok len:%u.\n",stStream.u32Len);
                    file_len += stStream.u32Len;

            }
            else
            {
                AK_BASE_PrintErrorEx(MODULE_ID_APP, "send to encode failed\n");
            }
        }

		AK_MPI_VPSS_ReleaseFrame(0, 0, &vpss_frame);
        printf("enc write ok frametime=%" PRIu64 " syst=%lld \n", vpss_frame.stFrame.u64Ts, get_sys_run_time64());
	    //AK_MPI_VENC_SetExifEnable(0, AK_FALSE);
    }
    else
    {
        AK_BASE_PrintErrorEx(MODULE_ID_VENC, "AK_MPI_VPSS_GetFrame failed! %d %#x\n", s32Ret, s32Ret);
    }


    return 0;
}

int ak_snap_env_enable(int bUVC)
{
	if(gm_config.photo_param.main_org_width != gm_config.photo_param.main_res_width
		|| gm_config.photo_param.main_org_height != gm_config.photo_param.main_res_height)
    {
        AK_MPI_VENC_SetJpegSlice(0, gm_config.photo_param.main_res_height, gm_config.photo_param.main_jpg_slice_one_height);
    }

	/* UVC frames are the photo JPEGs */
	fw_video_enc_param venc = {0};
	venc.width = gm_config.photo_param.main_res_width;
	venc.height = gm_config.photo_param.main_res_height;
	venc.framerate = gm_config.photo_param.main_chn_fps;
/*	while(1)
	{
		fw_frame_info frame;
		get_jpeg_frame_cb(0, &frame, NULL);
		usleep(300*100);
	}*/
	return video_uvc_init(&venc, get_jpeg_frame_cb);
}
