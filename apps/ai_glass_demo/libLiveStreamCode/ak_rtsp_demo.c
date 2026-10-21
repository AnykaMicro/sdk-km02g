/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd. */
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>
#include <getopt.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include "media_configs.h"

#include "ak_common.h"
#include "ak_log.h"
#include "ak_common_video.h"
#include "ak_venc.h"
#include "ak_thread.h"
#include "ak_mem.h"
#include "ak_vi.h"
#include "ak_vpss.h"
#include "ak_video.h"
#include "ak_common_audio.h"
#include "ak_ai.h"
#include "ak_aenc.h"
#include "aenc_mp3.h"
#include "aenc_aac.h"
#include "aenc_amr.h"
#include "ak_its.h"

#include "ak_rtsp_server.h"
#include "ak_circular_buffer.h"
#include "ak_rtsp_demo.h"

#ifdef AI_DENOISE_ENABLE
#include "ak_npu.h"
#endif

#define THREAD_PRIO -1
#define MAX_ENC_NUM   4
#define ENABLE_VENC_MMA
// #define VENC_BIND_MODE_ENABLE


#ifndef VIDEO_DEV0
#define VIDEO_DEV0 0
#endif

struct venc_info
{
    int venc_handle;
    VENC_CHN_ATTRS_S stChnAttrs;
    int dev_id;
    int chn_id;
};
struct audio_param
{
    int code_type ;     //  = 4
    int channel_num ;   //= 1
    int sample_bits ;   //= 16
    int sample_rate ;   //= 16000
};

typedef struct _rtsp_info
{
    char isp_conf[256];
    struct venc_info venc_info[2];

    //ring buffer handle
    CIRBUF_HANDLE ring_buffer_id[3];//0:main stream; 1:sub stream; 2:audio stream
    AK_MUTEX_T lock_buffer[3];

    //[audio]
    struct audio_param audio;
}rtsp_info_t;

//video
rtsp_info_t rtsp_param                  = {0};
static AK_PTHREAD_T venc_stream_th[2]   = {0};
static AK_S32 s32VideoFlag              = 0;
static int dev_cnt                      = 1;
static int g_venc_thread_run_flag       = 0;
//audio
static int audio_enable = 1;
static AK_CHAR *spchConfFile        = "/etc/config/audio_config.ini";// config path
static AK_CHAR *spchConfSection     = "def_near";                  // config section
static AK_S32 s32FrameInterval      = 32;
static AK_S32 s32AiChnId            = 0;                  //ai chn id
static AK_S32 s32AiDevId            = DEV_ADC;            //ai dev id
static AUDIO_IN_PARAM_S stAiParam   = {0};
static AK_S32 s32Gain               = 30;
static AK_S32 s32AencUserId         = 0;
static AK_S32 s32AudioFlag          = 0;
static AK_PTHREAD_T encode_tid;
static AK_PTHREAD_T get_tid;


//ak_rtsp_server_demo.c
int rtsp_server_init();
int rtsp_server_deinit();

#ifdef AI_DENOISE_ENABLE
/**
 * InitAIDenoise - when use AIDenoise, init npu
 * notes:
 */
static AK_S32 InitAIDenoise(AK_VOID)
{
    AK_MPI_AUDIO_RegisterAIDenoise();
    int s32Ret = AK_MPI_NPU_Init();
    if (s32Ret) {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "ERROR: AK_MPI_NPU_Init,  s32Ret = %d\n", s32Ret);
        return AK_FAILED;
    }
    return AK_SUCCESS;
}

/**
 * DestroyAIDenoise - close AIDenoise
 * notes:
 */
static AK_VOID DestroyAIDenoise(AK_VOID)
{
    AK_MPI_NPU_Deinit();
}
#endif

int aa_ls_get_audio_enc_param(rtsp_audio_enc_param* param)
{
    if(param == NULL || audio_enable == 0)
    {
        return -1;
    }

    param->iChannelNum = rtsp_param.audio.channel_num;
    param->eCodeType = rtsp_param.audio.code_type;
    param->iSampleBits = rtsp_param.audio.sample_bits;
    param->iSampleRate = rtsp_param.audio.sample_rate;

    return 0;
}


int aa_ls_get_video_enc_param(int dev, int chn, rtsp_video_enc_param* param)
{
    if(param == NULL || chn >= 2 || chn < 0)
    {
        return -1;
    }
    param->iWidth = rtsp_param.venc_info[chn].stChnAttrs.stVencAttrs.u16Width;
    param->iHeight = rtsp_param.venc_info[chn].stChnAttrs.stVencAttrs.u16Height;
    param->eCodeType = rtsp_param.venc_info[chn].stChnAttrs.stVencAttrs.enOutputType;
    if(param->eCodeType == H264_ENC_TYPE)
    {
        param->iFps = rtsp_param.venc_info[chn].stChnAttrs.stVencAttrs.stH264Attr.u16Framerate;
        if(rtsp_param.venc_info[chn].stChnAttrs.stRcAttrs.enRcMode == BR_MODE_VBR)
        {
            param->iBps = rtsp_param.venc_info[chn].stChnAttrs.stRcAttrs.stH264Vbr.u16MaxBitrate;
        }
        else if(rtsp_param.venc_info[chn].stChnAttrs.stRcAttrs.enRcMode == BR_MODE_CBR)
        {
            param->iBps = rtsp_param.venc_info[chn].stChnAttrs.stRcAttrs.stH264Cbr.u16TargetBitrate;
        }
        else if(rtsp_param.venc_info[chn].stChnAttrs.stRcAttrs.enRcMode == BR_MODE_AVBR)
        {
            param->iBps = rtsp_param.venc_info[chn].stChnAttrs.stRcAttrs.stH264Avbr.u16MaxBitrate;
        }
        else if(rtsp_param.venc_info[chn].stChnAttrs.stRcAttrs.enRcMode == BR_MODE_CAPPED_VBR)
        {
            param->iBps = rtsp_param.venc_info[chn].stChnAttrs.stRcAttrs.stH264CappedVbr.u16MaxBitRate;
        }
        else
        {
            param->iBps = 0;
        }
    }
    else
    {
        param->iFps = rtsp_param.venc_info[chn].stChnAttrs.stVencAttrs.stH265Attr.u16Framerate;
        if(rtsp_param.venc_info[chn].stChnAttrs.stRcAttrs.enRcMode == BR_MODE_VBR)
        {
            param->iBps = rtsp_param.venc_info[chn].stChnAttrs.stRcAttrs.stH265Vbr.u16MaxBitrate;
        }
        else if(rtsp_param.venc_info[chn].stChnAttrs.stRcAttrs.enRcMode == BR_MODE_CBR)
        {
            param->iBps = rtsp_param.venc_info[chn].stChnAttrs.stRcAttrs.stH265Cbr.u16TargetBitrate;
        }
        else if(rtsp_param.venc_info[chn].stChnAttrs.stRcAttrs.enRcMode == BR_MODE_AVBR)
        {
            param->iBps = rtsp_param.venc_info[chn].stChnAttrs.stRcAttrs.stH265Avbr.u16MaxBitrate;
        }
        else if(rtsp_param.venc_info[chn].stChnAttrs.stRcAttrs.enRcMode == BR_MODE_CAPPED_VBR)
        {
            param->iBps = rtsp_param.venc_info[chn].stChnAttrs.stRcAttrs.stH265CappedVbr.u16MaxBitRate;
        }
        else
        {
            param->iBps = 0;
        }

    }

    return 0;
}

//0:main stream ; 1:sub stream ; 2:audio stream
int aa_ls_reg_stream_client(int nDevId,  int stream_id, int *pnHandle,char *pReaderName)
{
    if(stream_id < 0 || stream_id >=3)
    {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "stream_id = %d error!\n",stream_id);
        return -1;
    }
//    char client[16] = {0};
//    sprintf(client,"client%d",stream_id);
    *pnHandle = cirbuf_reader_new(rtsp_param.ring_buffer_id[stream_id],pReaderName);
    return 0;
}

int aa_ls_unreg_stream_client(int nDevId, int stream_id, int nHandle)
{
    if(stream_id < 0 || stream_id >=3)
    {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "stream_id = %d error!\n",stream_id);
        return -1;
    }

    return cirbuf_reader_delete(rtsp_param.ring_buffer_id[stream_id], nHandle);
}

int aa_ls_force_i_frame(int dev, int chn)
{
    if(chn >= 2 || chn < 0)
    {
        return -1;
    }

    return AK_MPI_VENC_RequestIdr(rtsp_param.venc_info[chn].venc_handle);
}

int aa_ls_get_frame(int nDevId, int stream_id, int nHandle, rtsp_frame_info *pFrame)
{
    if(stream_id < 0 || stream_id >=3)
    {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "stream_id = %d error!\n",stream_id);
        return -1;
    }
    int data_len = 0;
    AK_BASE_ThreadMutexLock(&rtsp_param.lock_buffer[stream_id]);
    int result = 0;
    unsigned char *data = NULL;
    int ret = cirbuf_get_buff_data(rtsp_param.ring_buffer_id[stream_id], nHandle, &data,&data_len, 0);
    if( ret < 0)
    {
        pFrame->uiDataLen = 0;
        pFrame->pData = NULL;

        AK_BASE_ThreadMutexUnlock(&rtsp_param.lock_buffer[stream_id]);
        return ret;
    }

    memcpy(pFrame,data,sizeof(rtsp_frame_info));
    pFrame->pData = data + sizeof(rtsp_frame_info);
//    AK_BASE_PrintErrorEx(MODULE_ID_APP,
//         "stream_id=%d nHandle=%d pFrame->pData=%p pFrame->iCodeType=%d,\
//         pFrame->uiDataLen=%d,pFrame->iWidth=%d,pFrame->iHeight=%d u64Time=%d\n",
//        stream_id,nHandle,pFrame->pData,pFrame->iCodeType,
//        pFrame->uiDataLen,pFrame->iWidth,pFrame->iHeight,pFrame->u64Time);
    AK_BASE_ThreadMutexUnlock(&rtsp_param.lock_buffer[stream_id]);

    return 0;
}
//ak_rtsp_server_demo.c end

/**
 * calculate_adaptive_crop - Adaptive cropping parameter calculation function
 * Function: Automatically calculate optimal cropping parameters based on input and output resolutions
 * Supports arbitrary aspect ratio adaptation, ensuring maximum effective picture area
 */
static void calculate_adaptive_crop(
    unsigned int input_width,    // Input picture width (stViRectangle.u32Width)
    unsigned int input_height,   // Input picture height (stViRectangle.u32Height)
    unsigned int output_width,   // Output picture width (stVpssChnAttr.u32Width)
    unsigned int output_height,  // Output picture height (stVpssChnAttr.u32Height)
    unsigned int *crop_width,    // Output: Crop width
    unsigned int *crop_height,   // Output: Crop height
    unsigned int *crop_x,        // Output: Crop start X coordinate
    unsigned int *crop_y         // Output: Crop start Y coordinate
)
{
    // Calculate aspect ratio (use integer arithmetic to avoid floating point numbers, precision * 1000)
    unsigned int input_ratio = input_width * 1000 / input_height;   // Input aspect ratio * 1000
    unsigned int output_ratio = output_width * 1000 / output_height; // Output aspect ratio * 1000

    unsigned int max_crop_width = output_width * 2;   // Maximum crop width limit
    unsigned int max_crop_height = output_height * 2; // Maximum crop height limit

    printf("Adaptive Crop: Input(%u×%u, ratio=%u), Output(%u×%u, ratio=%u), Limits(%u×%u)\n",
	       input_width, input_height, input_ratio,
           output_width, output_height, output_ratio,
           max_crop_width, max_crop_height);

    if (input_ratio > output_ratio) {
        // Input picture is "wider", adapt by height, crop left and right
        *crop_height = (input_height < max_crop_height) ? input_height : max_crop_height;
        *crop_width = (*crop_height * output_width) / output_height;

        // Ensure crop width does not exceed input width and maximum limit
        if (*crop_width > input_width) {
            *crop_width = input_width;
            *crop_height = (*crop_width * output_height) / output_width;
        }
        if (*crop_width > max_crop_width) {
            *crop_width = max_crop_width;
            *crop_height = (*crop_width * output_height) / output_width;
        }

        // Center crop
        *crop_x = (input_width - *crop_width) / 2;
        *crop_y = (input_height - *crop_height) / 2;
    } else {
        // Input picture is "taller", adapt by width, crop top and bottom
        *crop_width = (input_width < max_crop_width) ? input_width : max_crop_width;
        *crop_height = (*crop_width * output_height) / output_width;

        // Ensure crop height does not exceed input height and maximum limit
        if (*crop_height > input_height) {
            *crop_height = input_height;
            *crop_width = (*crop_height * output_width) / output_height;
        }
        if (*crop_height > max_crop_height) {
            *crop_height = max_crop_height;
            *crop_width = (*crop_height * output_width) / output_height;
        }

        // Center crop
        *crop_x = (input_width - *crop_width) / 2;
        *crop_y = (input_height - *crop_height) / 2;
    }

    printf("Adaptive Crop Result: Crop(%ux%u) at (%u,%u)\n",
           *crop_width, *crop_height, *crop_x, *crop_y);
}
static int venc_param_set(VENC_ATTR_S *pstVencAttrs,
                            VENC_RC_ATTRS_S *pstRcAttrs,
                            unsigned short width,
                            unsigned short height,
                            int enc_type,
                            int br_mode,
                            unsigned short fps,
                            unsigned short gop_len,
                            unsigned short target_kbps,
                            unsigned short max_kbps)
{

    printf("#####width:%u height:%u enc_type:%d br_mode:%d fps:%u gop_len:%u target_kbps:%u max_kbps:%u\n",
        width,height,enc_type,br_mode,fps,gop_len,target_kbps,max_kbps);

    pstVencAttrs->u16Width = width;
    pstVencAttrs->u16Height = height;

    if(enc_type == H264_ENC_TYPE)
    {
        pstVencAttrs->enOutputType = H264_ENC_TYPE;                     //output type
        pstVencAttrs->u32MaxBufSize = 0;                                //ext buf size, KBytes
        pstVencAttrs->u32MaxDMASize = 0;                                //inner buf size, KBytes
        pstVencAttrs->enChromaMode = VENC_CHROMA_YUV420;                //yuv420sp
        pstVencAttrs->u32Priority = 0;                                  //priority

        pstVencAttrs->stH264Attr.enGdrMode = VENC_GDR_OFF;              //GDR
        pstVencAttrs->stH264Attr.enProfileMode = VENC_PROFILE_MAIN;     //profile
        pstVencAttrs->stH264Attr.enEncLevel = VENC_HEVC_AVC_LEVEL_3;    //encoder level
        pstVencAttrs->stH264Attr.u16Framerate = fps;         //fps
        pstVencAttrs->stH264Attr.u16GopLen = gop_len;          //gop len

        if(br_mode == BR_MODE_VBR)
        {
            pstRcAttrs->enRcMode = VENC_RC_MODE_H264_VBR;              //vbr
            pstRcAttrs->stH264Vbr.u16MaxBitrate = max_kbps;     //target kbps
            pstRcAttrs->stH264Vbr.u16Quality = 50;                     //quality
        }
        else if(br_mode == BR_MODE_CBR)
        {
            pstRcAttrs->enRcMode = VENC_RC_MODE_H264_CBR;              //cbr
            pstRcAttrs->stH264Cbr.u16TargetBitrate = target_kbps;  //target kbps
        }
        else if(br_mode == BR_MODE_AVBR)
        {
            pstRcAttrs->enRcMode = VENC_RC_MODE_H264_AVBR;             //avbr
            pstRcAttrs->stH264Avbr.u16MinQP = 20;                      //min qp
            pstRcAttrs->stH264Avbr.u16MaxQP = 48;                      //max qp
            pstRcAttrs->stH264Avbr.u16MaxBitrate = max_kbps;    //target Kbits

            //targetBitrate = maxBitrate*changePos/100
            pstRcAttrs->stH264Avbr.u16ChangePos = (target_kbps*100/max_kbps);
        }
        else if(br_mode == BR_MODE_CONST_QP)
        {
            pstRcAttrs->enRcMode = VENC_RC_MODE_H264_CONST_QP;         //const QP
            pstRcAttrs->stH264ConstQp.u16QP = 50;                      //qp
        }
        else if(br_mode == BR_MODE_CAPPED_VBR)
        {
            pstRcAttrs->enRcMode = VENC_RC_MODE_H264_CAPPED_VBR;         //const qp
            pstRcAttrs->stH264CappedVbr.u16MinQP = 20;                   //min qp
            pstRcAttrs->stH264CappedVbr.u16MaxQP = 50;                   //max qp
            pstRcAttrs->stH264CappedVbr.u16MaxBitRate = max_kbps; //max bitrate
            //targetBitrate = maxBitrate*changePos/100
            pstRcAttrs->stH264CappedVbr.u16ChangePos = (target_kbps*100/max_kbps);
            pstRcAttrs->stH264CappedVbr.d64MinPSNR = 25.0;               //min PSNR
            pstRcAttrs->stH264CappedVbr.d64MaxPSNR = 42.0;               //max PSNR
        }
    }
    /*decode type is h265*/
    else if(enc_type == HEVC_ENC_TYPE)
    {
        pstVencAttrs->enOutputType = HEVC_ENC_TYPE;                     //output type
        pstVencAttrs->u32MaxBufSize = 0;                                //ext buf size, KBytes
        pstVencAttrs->u32MaxDMASize = 0;                                //inner buf size, KBytes
        pstVencAttrs->enChromaMode = VENC_CHROMA_YUV420;                //yuv420sp
        pstVencAttrs->u32Priority = 0;                                  //priority

        pstVencAttrs->stH265Attr.enGdrMode = VENC_GDR_OFF;              //GDR
        pstVencAttrs->stH265Attr.enProfileMode = VENC_PROFILE_HEVC_MAIN;//profile
        pstVencAttrs->stH265Attr.enEncLevel = VENC_HEVC_AVC_LEVEL_3;    //encoder level
        pstVencAttrs->stH265Attr.u16Framerate = fps;         //fps
        pstVencAttrs->stH265Attr.u16GopLen = gop_len;          //gop len

        if(br_mode == BR_MODE_VBR)
        {
            pstRcAttrs->enRcMode = VENC_RC_MODE_H265_VBR;              //vbr
            pstRcAttrs->stH265Vbr.u16MaxBitrate = max_kbps;     //target kbps
            pstRcAttrs->stH265Vbr.u16Quality = 50;                     //quality
        }
        else if(br_mode == BR_MODE_CBR)
        {
            pstRcAttrs->enRcMode = VENC_RC_MODE_H265_CBR;              //cbr
            pstRcAttrs->stH264Cbr.u16TargetBitrate = target_kbps;  //target kbps
        }
        else if(br_mode == BR_MODE_AVBR)
        {
            pstRcAttrs->enRcMode = VENC_RC_MODE_H265_AVBR;             //VBR
            pstRcAttrs->stH265Avbr.u16MinQP = 20;                      //min qp
            pstRcAttrs->stH265Avbr.u16MaxQP = 48;                      //max qp
            pstRcAttrs->stH265Avbr.u16MaxBitrate = max_kbps;    //target Kbits
            //targetBitrate = maxBitrate*changePos/100
            pstRcAttrs->stH265Avbr.u16ChangePos = (target_kbps*100/max_kbps);
        }
        else if(br_mode == BR_MODE_CONST_QP)
        {
            pstRcAttrs->enRcMode = VENC_RC_MODE_H265_CONST_QP;         //const qp
            pstRcAttrs->stH265ConstQp.u16QP = 50;                      //qp
        }
        else if(br_mode == BR_MODE_CAPPED_VBR)
        {
            pstRcAttrs->enRcMode = VENC_RC_MODE_H265_CAPPED_VBR;         //const qp
            pstRcAttrs->stH265CappedVbr.u16MinQP = 20;                   //min qp
            pstRcAttrs->stH265CappedVbr.u16MaxQP = 50;                   //max qp
            pstRcAttrs->stH265CappedVbr.u16MaxBitRate = max_kbps; //max bitrate
            //targetBitrate = maxBitrate*changePos/100
            pstRcAttrs->stH265CappedVbr.u16ChangePos = (target_kbps*100/max_kbps);
            pstRcAttrs->stH265CappedVbr.d64MinPSNR = 25.0;               //min PSNR
            pstRcAttrs->stH265CappedVbr.d64MaxPSNR = 42.0;               //max PSNR
        }
    }
    /*decode type is jpeg*/
    else
    {
        pstVencAttrs->enOutputType = MJPEG_ENC_TYPE;                    //output type
        pstVencAttrs->u32MaxBufSize = 0;                                //ext buf size, KBytes
        pstVencAttrs->u32MaxDMASize = 0;                                //inner buf size, KBytes
        pstVencAttrs->enChromaMode = VENC_CHROMA_YUV420;                //yuv420sp
        pstVencAttrs->u32Priority = 0;                                  //priority

        pstRcAttrs->enRcMode = VENC_RC_MODE_MJPEG;                      //rc mode
        pstRcAttrs->stMjpegRc.u16Quality = 50;                          //quality
    }
    return 0;
}

int rtsp_ini_param(char *isp_conf_path)
{
    // ISP CONF
    snprintf(rtsp_param.isp_conf, sizeof(rtsp_param.isp_conf), "%s", isp_conf_path);

    // Video CHN0
    venc_param_set(&rtsp_param.venc_info[0].stChnAttrs.stVencAttrs,
                    &rtsp_param.venc_info[0].stChnAttrs.stRcAttrs,
                    gm_config.rtsp_param.main_chn_width,
                    gm_config.rtsp_param.main_chn_height,
                    gm_config.rtsp_param.main_chn_enc_type,
                    gm_config.rtsp_param.main_chn_br_mode,
                    gm_config.rtsp_param.main_chn_fps,
                    gm_config.rtsp_param.main_chn_gop_len,
                    gm_config.rtsp_param.main_chn_target_kbps,
                    gm_config.rtsp_param.main_chn_max_kbps);

    //Audio
    rtsp_param.audio.code_type = AK_AUDIO_TYPE_AAC;
    rtsp_param.audio.sample_rate = 16000;
    rtsp_param.audio.channel_num = 1;
    rtsp_param.audio.sample_bits = 16;

    return 0;
}

static int init_ai(struct audio_param *audio)
{
    AK_S32 s32Ret = 0;
    stAiParam.enSampleBits = audio->sample_bits;
    stAiParam.enSampleRate = audio->sample_rate;
    stAiParam.enSoundMode = audio->channel_num;
    stAiParam.u32SrcCnt = 1;
    stAiParam.u32ChnCnt = 1;
    stAiParam.unSrcConfig.stAdcConfig.enSourceType = AUDIO_ADC_SOURCE_MIC;

    if (AK_MPI_AI_SetPubAttr(s32AiDevId, &stAiParam)) {
        AK_BASE_PrintErrorEx(MODULE_ID_APP,"AK_MPI_AI_SetPubAttr failed \n");
        return AK_FAILED;
    }

    if (AK_MPI_AI_Enable(s32AiDevId)) {
        AK_BASE_PrintErrorEx(MODULE_ID_APP,"AK_MPI_AI_Enable failed \n");
        return AK_FAILED;
    }
    if (s32AiDevId != DEV_PDM) {
        s32Ret = AK_MPI_AI_SetGain(s32AiDevId, s32AiChnId, s32Gain);
        if(s32Ret){
            AK_BASE_PrintErrorEx(MODULE_ID_APP, "AK_MPI_AI_SetGain failed\n");
            return AK_FAILED;
        }
    }
    AK_S32  s32FrameLen = 0;
    AK_MPI_AI_GetFrmLen(s32AiDevId, s32AiChnId, &s32FrameLen);// use default frame length
    s32FrameInterval = AudioLenToInterval(stAiParam.enSampleRate, AK_AUDIO_SMPLE_BIT_16,
                                        stAiParam.enSoundMode, s32FrameLen);

    if(AK_MPI_AI_LoadConf(s32AiDevId, s32AiChnId, spchConfFile, spchConfSection)){
        AK_BASE_PrintWarningEx(MODULE_ID_APP,"AK_MPI_AI_LoadConf failed \n");
    }

    if (AK_MPI_AI_EnableChn(s32AiDevId,s32AiChnId)) {
        AK_BASE_PrintErrorEx(MODULE_ID_APP,"AK_MPI_AI_EnableChn failed \n");
        return AK_FAILED;
    }
    return AK_SUCCESS;
}

static void destroy_ai_aenc(void)
{
    AK_MPI_AENC_Close(s32AencUserId);
    AK_MPI_AI_DisableChn(s32AiDevId, s32AiChnId);
    AK_MPI_AI_Disable(s32AiDevId);
}

static int init_aenc(struct audio_param *audio)
{
    /*use to aenc open*/
    AENC_PARAM_S stAencParam = {0};

    stAencParam.enType          = audio->code_type;
    stAencParam.enSampleBits    = audio->sample_bits;
    stAencParam.enSampleRate    = audio->sample_rate;
    stAencParam.enSoundMode     = audio->channel_num;

    /*use to register*/
    AENC_ENCODE_S pstEncode = {0};
    pstEncode.enType = stAencParam.enType;
    switch (stAencParam.enType) {
        case AK_AUDIO_TYPE_MP3:
            printf("AK_AUDIO_TYPE_MP3\n");
            pstEncode.pfnEncOpen = AENC_OpenMp3;
            if (AK_MPI_AENC_RegisterEncoder(&pstEncode)) {
                AK_BASE_PrintErrorEx(MODULE_ID_APP, "AK_MPI_AENC_RegisterEncoder failed\n");
                return AK_FAILED;
            }
            break;
        case AK_AUDIO_TYPE_AAC:
            printf("AK_AUDIO_TYPE_AAC\n");
            /*set aac param*/
            stAencParam.unAencAttr.stAacParam.enAacFrmHead = AENC_AAC_SAVE_FRAME_HEAD;
            pstEncode.pfnEncOpen = AENC_OpenAac;
            if (AK_MPI_AENC_RegisterEncoder(&pstEncode)) {
                AK_BASE_PrintErrorEx(MODULE_ID_APP, "AK_MPI_AENC_RegisterEncoder failed\n");
                return AK_FAILED;
            }
            break;
        case AK_AUDIO_TYPE_AMR:
            printf("AK_AUDIO_TYPE_AMR\n");
            pstEncode.pfnEncOpen = AENC_OpenAmr;
            if (AK_MPI_AENC_RegisterEncoder(&pstEncode)) {
                AK_BASE_PrintErrorEx(MODULE_ID_APP, "AK_MPI_AENC_RegisterEncoder failed\n");
                return AK_FAILED;
            }
            break;
        case AK_AUDIO_TYPE_PCM_ALAW:
        case AK_AUDIO_TYPE_PCM_ULAW:
            break;
        default:
            break;
    }

    if(AK_MPI_AENC_Open(s32AencUserId, &stAencParam)) {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "AK_MPI_AENC_Open failed\n");
        return AK_FAILED;
    }

    return AK_SUCCESS;
}

/**
 * GetAudioStream - get stream from encoder
 * @arg[IN]: arg is NULL
 * notes:
 */
static AK_VOID* GetAudioStream(AK_VOID *arg)
{
    AK_BASE_ThreadSetName("[aenc stream]attachment");
    AUDIO_STREAM_S stAencStream = {0};
    AK_S32 s32Ret = -1;
    AK_S32 sleep_time = s32FrameInterval == 0 ? 16: (s32FrameInterval / 2);
    AK_BASE_PrintNormalEx(MODULE_ID_APP, "get_stream sleep_time=%d\n", sleep_time);
    while(1 == s32AudioFlag) {
        s32Ret = AK_MPI_AENC_GetStream(s32AencUserId,&stAencStream, 200);
        if (s32Ret) {
            if (ERROR_AENC_NO_DATA == s32Ret) {
                AK_BASE_SleepMs(sleep_time);
                continue;
            } else {
                AK_BASE_PrintErrorEx(MODULE_ID_APP,"AK_MPI_AENC_GetStream error s32Ret = %d \n",s32Ret);
                break;
            }
        } else {
            if (stAencStream.pu8Data && stAencStream.u32Len) {

                // if(s32SaveFileFlg) {
                //     fwrite(stAencStream.pu8Data, stAencStream.u32Len, 1, fp);
                // }
                // PrintPlayingDot();

                // Write to ring buffer
                rtsp_frame_info frame_info = {0};
                frame_info.u64Time = stAencStream.u64Ts;
                frame_info.uiSeqNo = stAencStream.u64SeqNo;
                frame_info.uiDataLen = stAencStream.u32Len;
                frame_info.pData = stAencStream.pu8Data;
                frame_info.iSampleBits  = rtsp_param.audio.sample_bits;
                frame_info.iSampleRate  = rtsp_param.audio.sample_rate;
                frame_info.iChannelNum = rtsp_param.audio.channel_num;
                frame_info.iCodeType = rtsp_param.audio.code_type;
                AK_BASE_ThreadMutexLock(&rtsp_param.lock_buffer[2]);

                int frame_len = sizeof(rtsp_frame_info);
                char *buf_tmp = cirbuf_get_write_buf(rtsp_param.ring_buffer_id[2],stAencStream.u32Len + frame_len);
                memcpy(buf_tmp,&frame_info,frame_len);
                memcpy(buf_tmp + frame_len,stAencStream.pu8Data,stAencStream.u32Len);
                cirbuf_put_write_buf(rtsp_param.ring_buffer_id[2],stAencStream.u32Len + frame_len);
                AK_BASE_ThreadMutexUnlock(&rtsp_param.lock_buffer[2]);

            } else {
                continue;
            }
        }
        AK_MPI_AENC_ReleaseStream(s32AencUserId, &stAencStream);
    }

    AK_BASE_PrintNoticeEx(MODULE_ID_APP, "%s exit\n", __func__);
    AK_BASE_ThreadExit();
}

/**
 * GetAudioFrameEncode - ai get frame and then send to aenc encode
 * @arg[IN]: arg is NULL
 * notes:
 */
static AK_VOID * GetAudioFrameEncode(AK_VOID *arg)
{
    AK_BASE_ThreadSetName("[aenc frame]attachment");
    static TIME_VAL_S cur_time = {0};

    AK_BASE_GetOstime(&cur_time);
    // AK_U64 start_ts = 0;
    FRAME_S AIFRAME  = {0};
    // AK_U64 end_ts = 0;
    AK_U32 read_len = 0;
    AK_U32 total_len = 0;

    AK_S32 s32Ret = AK_FAILED;
    AK_S32 sleep_time = s32FrameInterval == 0 ? 16: (s32FrameInterval / 2);
    AK_BASE_PrintNormalEx(MODULE_ID_APP, "sleep_time=%d\n", sleep_time);
    while(s32AudioFlag) {
        /*get pcm data*/
        s32Ret = AK_MPI_AI_GetFrame(s32AiDevId, s32AiChnId,&AIFRAME, 1000, 0);
        if (s32Ret) {
            if (ERROR_AI_NO_DATA == s32Ret) {
                AK_BASE_SleepMs(sleep_time);
                continue;
            } else {
                AK_BASE_PrintErrorEx(MODULE_ID_APP, "AK_MPI_AI_GetFrame error\n");
                /*use to stop get stream*/
                s32AudioFlag = 0;
                break;
            }
        }
        /*send frame to encoder*/
        s32Ret = AK_MPI_AENC_SendFrame(s32AencUserId, &AIFRAME, 2000);
        if(s32Ret) {
            AK_MPI_AI_ReleaseFrame(s32AiDevId, s32AiChnId, &AIFRAME, 0);
            AK_BASE_PrintErrorEx(MODULE_ID_APP, "send frame error\n");
            s32AudioFlag = 0;
            break;
        }

        // if (0 == start_ts) {
        //     start_ts = AIFRAME.u64Ts;
        // }
        // end_ts = AIFRAME.u64Ts;
        AK_MPI_AI_ReleaseFrame(s32AiDevId, s32AiChnId, &AIFRAME, 0);
        // if ((end_ts -start_ts) >= s32SaveTime) {
        //     AK_BASE_PrintNoticeEx(MODULE_ID_APP,"time is up \n");
        //     s32AudioFlag = 0;
        //     break;
        // }
    }
    AK_MPI_AENC_SendEndOfFrame(s32AencUserId);
    AK_BASE_PrintNoticeEx(MODULE_ID_APP, "%s get_frame_encode exit\n", __func__);
    AK_BASE_ThreadExit();
    return NULL;
}

static int init_vi(int dev_id)
{
    AK_S32 ret = 0;
    VI_DEV_ATTR_S stViDevAttr       = {0};
    VI_RECTANGLE_S stViRectangle    = {0};
    VI_CHN_ATTR_S stViChnAttr       = {0};
    VPSS_GRP_ATTR_S stGrpAttr       = {0};
    VPSS_CHN_ATTR_S stVpssChnAttr   = {0};

	vi_dev_info dev_info = {0};
	unsigned int frame_depth = 4;

	get_vi_dev_info_ex(0,&dev_info);

	if(get_vi_dev_info_ex(0,&dev_info) == 0)
	{
		frame_depth = dev_info.main_frame_depth;
		printf("get vi dev depth:%d frame_rate=%d\n",frame_depth,dev_info.frame_rate);
	}

    stViDevAttr.u32Width                = 320;
    stViDevAttr.u32Height               = 184;
    stViDevAttr.stCropInfo.u32PointX    = 0;
    stViDevAttr.stCropInfo.u32PointY    = 0;
    stViDevAttr.stCropInfo.u32Width     = stViDevAttr.u32Width;
    stViDevAttr.stCropInfo.u32Height    = stViDevAttr.u32Height;
    stViDevAttr.enIntfMode              = dev_id == 1?VI_INTF_MULTI_MIPI_2:VI_INTF_MIPI_2;
    stViDevAttr.enInputPixFmt           = PIXEL_FORMAT_YUV420SP;
    stViDevAttr.u32Fps                  =  rtsp_param.venc_info[0].stChnAttrs.stVencAttrs.enOutputType == H264_ENC_TYPE?
                                            rtsp_param.venc_info[0].stChnAttrs.stVencAttrs.stH264Attr.u16Framerate:
                                            rtsp_param.venc_info[0].stChnAttrs.stVencAttrs.stH265Attr.u16Framerate;
    stViDevAttr.bMirror                 = 0;
    stViDevAttr.bFlip                   = 0;
    ret = AK_MPI_VI_CreateDev(dev_id,&stViDevAttr);
    if(ret) {
        AK_BASE_PrintErrorEx(MODULE_ID_APP,"Create vi device fail \n");
        return -1;
    }

    ret = AK_MPI_VI_GetSensorRes(dev_id, &stViRectangle);
    if(ret) {
        AK_BASE_PrintErrorEx(MODULE_ID_APP,"Get sensor res fail \n");
        return -1;
    }
    else
    {
        printf("Sensor Real Resolution: %u*%u\n",stViRectangle.u32Width,stViRectangle.u32Height);
    }

    stViDevAttr.u32Width = stViRectangle.u32Width;
    stViDevAttr.u32Height = stViRectangle.u32Height;
    stViDevAttr.stCropInfo.u32Width = stViRectangle.u32Width;
    stViDevAttr.stCropInfo.u32Height = stViRectangle.u32Height;
    ret = AK_MPI_VI_SetDevAttr(dev_id, &stViDevAttr);
    if(ret) {
        AK_BASE_PrintErrorEx(MODULE_ID_APP,"set device attr fail \n");
        return -1;
    }

    stViChnAttr.enMode              = VI_CHN_MODE_AUTO;
    stViChnAttr.enOutputPixFmt      = PIXEL_FORMAT_YUV420SP;
    stViChnAttr.u32Fps              = rtsp_param.venc_info[0].stChnAttrs.stVencAttrs.enOutputType == H264_ENC_TYPE?
                                            rtsp_param.venc_info[0].stChnAttrs.stVencAttrs.stH264Attr.u16Framerate:
                                            rtsp_param.venc_info[0].stChnAttrs.stVencAttrs.stH265Attr.u16Framerate;

#ifdef VENC_BIND_MODE_ENABLE
    stViChnAttr.u32FrameDepth       = 4;
#else
    stViChnAttr.u32FrameDepth       = frame_depth;
#endif

    stViChnAttr.u32Width            = stViRectangle.u32Width;
    stViChnAttr.u32Height           = stViRectangle.u32Height;
    stViChnAttr.u32MaxPerFrameSize  = 0;
    ret = AK_MPI_VI_SetChnAttr(dev_id,0, &stViChnAttr);
    if(ret) {
        AK_BASE_PrintErrorEx(MODULE_ID_APP,"Set vi chn attr fail \n");
        return -1;
    }

    // stViChnAttr.enMode              = VI_CHN_MODE_USER;
    // stViChnAttr.enOutputPixFmt      = PIXEL_FORMAT_YUV420SP;
    // stViChnAttr.u32Fps              = STREAM_FPS;
    // stViChnAttr.u32FrameDepth       = 2;
    // stViChnAttr.u32Width            = SUB_CHN_WIDTH;
    // stViChnAttr.u32Height           = SUB_CHN_HEIGHT;
    // stViChnAttr.u32MaxPerFrameSize  = 0;
    // ret = AK_MPI_VI_SetChnAttr(dev_id,1, &stViChnAttr);
    // if(ret) {
    //     AK_BASE_PrintErrorEx(MODULE_ID_APP,"Set vi chn attr fail \n");
    //     return -1;
    // }

    stGrpAttr.u32Width = stViRectangle.u32Width; //sensor resolution
    stGrpAttr.u32Height = stViRectangle.u32Height; //sensor resolution
    stGrpAttr.bEnGrpBypass = 0;
    stGrpAttr.bFlip = 0;
    stGrpAttr.bMirror = 0;
    stGrpAttr.enInputPixFmt = PIXEL_FORMAT_YUV420SP;
    stGrpAttr.enWorkMode = VPSS_WORK_AUTO;
    strcpy(stGrpAttr.pu8IspCfgPath, rtsp_param.isp_conf);
    AK_U32 u32SensorId = 0;
    AK_MPI_VI_GetSensorId(dev_id, &u32SensorId);
    stGrpAttr.s32AttachSensorId = u32SensorId;
    ret = AK_MPI_VPSS_CreateGrp(dev_id, &stGrpAttr);
    if(ret) {
        AK_BASE_PrintErrorEx(MODULE_ID_APP,"Create vpss Grp fail \n");
        return AK_FAILED;
    }

    stVpssChnAttr.u32Width                  = gm_config.rtsp_param.main_vi_width; //rtsp_param.venc_info[0].stChnAttrs.stVencAttrs.u16Width;
    stVpssChnAttr.u32Height                 = gm_config.rtsp_param.main_vi_height; //rtsp_param.venc_info[0].stChnAttrs.stVencAttrs.u16Height;
    stVpssChnAttr.enOutputPixFmt            = PIXEL_FORMAT_YUV420SP;

    // Use adaptive cropping algorithm to calculate optimal cropping parameters
    calculate_adaptive_crop(
        stViRectangle.u32Width,     // Input picture width
        stViRectangle.u32Height,    // Input picture height
        stVpssChnAttr.u32Width,     // Output picture width
        stVpssChnAttr.u32Height,    // Output picture height
        &stVpssChnAttr.stCropInfo.u32Width,   // Output: Crop width
        &stVpssChnAttr.stCropInfo.u32Height,  // Output: Crop height
        &stVpssChnAttr.stCropInfo.u32PointX,  // Output: Crop start X coordinate
        &stVpssChnAttr.stCropInfo.u32PointY   // Output: Crop start Y coordinate
    );

    stVpssChnAttr.u32MaxBufSize             = 0;
    stVpssChnAttr.s32Fps                    = rtsp_param.venc_info[0].stChnAttrs.stVencAttrs.enOutputType == H264_ENC_TYPE?
                                            rtsp_param.venc_info[0].stChnAttrs.stVencAttrs.stH264Attr.u16Framerate:
                                            rtsp_param.venc_info[0].stChnAttrs.stVencAttrs.stH265Attr.u16Framerate;
#ifdef VENC_BIND_MODE_ENABLE
    stVpssChnAttr.u32FrameDepth             = 4;
    stVpssChnAttr.enChnMode                 = VPSS_CHN_SLICE_MODE; //slice mode
#else
    stVpssChnAttr.u32FrameDepth             = frame_depth;
    stVpssChnAttr.enChnMode                 = VPSS_CHN_FRAME_MODE; //frame mode
#endif

    ret = AK_MPI_VPSS_SetChnAttr(dev_id, 0, &stVpssChnAttr);
    if(ret) {
        AK_BASE_PrintErrorEx(MODULE_ID_APP,"set vpss  chn attr fail \n");
        return AK_FAILED;
    }

    EXT_CHN_S stSrcChn = {0};
    EXT_CHN_S stDstChn = {0};
    EXT_BIND_PARAM_S stBindParam = {0};

    stSrcChn.enModId = MODULE_ID_VI;
    stSrcChn.s32DevId = dev_id;
    stSrcChn.s32ChnId = 0;

    stDstChn.enModId = MODULE_ID_VPSS;
    stDstChn.s32DevId = dev_id;
    stDstChn.s32ChnId = 0;

    stBindParam.s32BPFrameRate = rtsp_param.venc_info[0].stChnAttrs.stVencAttrs.enOutputType == H264_ENC_TYPE?
                                            rtsp_param.venc_info[0].stChnAttrs.stVencAttrs.stH264Attr.u16Framerate:
                                            rtsp_param.venc_info[0].stChnAttrs.stVencAttrs.stH265Attr.u16Framerate;
    stBindParam.s32BPFrameDepth = 10;
    stBindParam.s32BPMode = AUTO_MODE;
    ret = AK_EXT_VIDEO_BindChn(&stSrcChn, &stDstChn, &stBindParam);
    if(ret) {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "Vi bind to vpss error .\n");
        return 0;
    }

    return 0;
}

static int start_vi(int dev_id)
{
    int ret = 0;

    ret = AK_MPI_VI_EnableDev(dev_id);
    if(ret) {
        AK_BASE_PrintErrorEx(MODULE_ID_APP,"enable vi device fail \n");
        return AK_FAILED;
    }
    ret = AK_MPI_VI_EnableChn(dev_id,0);
    if(ret) {
        AK_BASE_PrintErrorEx(MODULE_ID_APP,"enable vi channel fail \n");
        return AK_FAILED;
    }
    // ret = AK_MPI_VI_EnableChn(dev_id,1);
    // if(ret) {
    //     AK_BASE_PrintErrorEx(MODULE_ID_APP,"enable vi channel fail \n");
    //     return AK_FAILED;
    // }

    ret = AK_MPI_VPSS_StartGrp(dev_id);
    if(ret) {
        AK_BASE_PrintErrorEx(MODULE_ID_APP,"start vpss grp fail \n");
        return AK_FAILED;
    }
    ret = AK_MPI_VPSS_EnableChn(dev_id, 0);
    if(ret) {
        AK_BASE_PrintErrorEx(MODULE_ID_APP,"enable vpss chn fail \n");
        return AK_FAILED;
    }

    return 0;
}
/* end of func */

static int stop_destory_vi(int dev_id)
{
    int ret = -1;

    /*
     * step 1: disable vpss chn
     */
    ret = AK_MPI_VPSS_DisableChn(dev_id, 0);
    if(ret) {
        AK_BASE_PrintErrorEx(MODULE_ID_APP,"Disable vpss chn1 fail .\n");
        return AK_FAILED;
    }

    // ret = AK_MPI_VPSS_DisableChn(dev_id, 2);
    // if(ret) {
    //     AK_BASE_PrintErrorEx(MODULE_ID_APP,"Disable vpss chn1 fail .\n");
    //     return AK_FAILED;
    // }

    /*
     * step 2: stop vpss group
     */
    ret = AK_MPI_VPSS_StopGrp(dev_id);
    if(ret) {
        AK_BASE_PrintErrorEx(MODULE_ID_APP,"AK_MPI_VPSS_StopGrp fail \n");
        return ret;
    }
    /*
     * step 3: destroy vpss group
     */
    ret = AK_MPI_VPSS_DestroyGrp(dev_id);
    if(ret) {
        AK_BASE_PrintErrorEx(MODULE_ID_APP,"AK_MPI_VPSS_DestroyGrp fail \n");
        return ret;
    }

    /*
     * step 1: disable vi chn
     */
    ret = AK_MPI_VI_DisableChn(dev_id,0);
    if(ret) {
        AK_BASE_PrintErrorEx(MODULE_ID_APP,"Disable vi chn fail .\n");
        return -1;
    }

    /*
     * step 2: disable vi dev
     */
    ret = AK_MPI_VI_DisableDev(dev_id);
    if(ret) {
        AK_BASE_PrintErrorEx(MODULE_ID_APP,"Disable vi device fail .\n");
        return -1;
    }

    /*
     * step 3: destroy vi dev
     */
    ret = AK_MPI_VI_DestroyDev(dev_id);
    if(ret) {
        AK_BASE_PrintErrorEx(MODULE_ID_APP,"Destroy vi device fail .\n");
        return -1;
    }
    return ret;
}

static AK_VOID *VencStreamThread(AK_VOID *pvArg)
{
    int venc_handle_id = *((int *)pvArg);
    int ret = 0;

    int prev_rtsp_status    = -2;
    int need_iframe_flag    = 0;
    int rtsp_status         = 0;
#ifdef VENC_BIND_MODE_ENABLE
    EXT_CHN_S stSrcChn = {0};
    stSrcChn.enModId = MODULE_ID_VPSS;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = venc_handle_id;

    EXT_CHN_S stDstChn = {0};
    stDstChn.enModId = MODULE_ID_VENC;
    stDstChn.s32DevId = 0;
    stDstChn.s32ChnId = venc_handle_id;
    EXT_BIND_PARAM_S stBindParam = {0};
    stBindParam.s32BPFrameRate = 0;
    stBindParam.s32BPFrameDepth = 100;
    if(venc_handle_id == 1)
        stBindParam.s32BPMode = USER_MODE; //user mode
    else
        stBindParam.s32BPMode = AUTO_MODE; //kernel mode

    ret = AK_EXT_VIDEO_BindChn(&stSrcChn, &stDstChn, &stBindParam);
    if(ret) {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "Vpss bind to venc error .\n");
        goto exit;
    }

    ret = AK_EXT_VIDEO_SetDstChnActive(&stDstChn, AK_TRUE);
    if(ret) {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "venc active faile.\n");
        goto exit;
    }
#else
    VPSS_FRAME_INFO_S vpss_frame = {0};
#endif

    VIDEO_STREAM_S *pstStream = AK_BASE_MemAlloc(MODULE_ID_APP,sizeof(VIDEO_STREAM_S));
    if(NULL == pstStream)
    {
        AK_BASE_PrintErrorEx(MODULE_ID_APP,
            "Can't alloc memory for stream buffer!\n");
        goto exit;
    }

    while(s32VideoFlag)
    {
#ifdef VENC_BIND_MODE_ENABLE
        ret = AK_EXT_VIDEO_VencGetStream(&stDstChn, pstStream);
#else
        ret = AK_MPI_VPSS_GetFrame(0, venc_handle_id, &vpss_frame, 2000);
        if(ret != AK_SUCCESS)
        {
            continue;
        }


        static int vi = 0;
        if(vi++== 0)
        {
			printf("%s_%d vit=%" PRIu64 "",__FUNCTION__, __LINE__, vpss_frame.stFrame.u64Ts);
        }

        ret = AK_MPI_VENC_EncodeFrame(venc_handle_id, vpss_frame.stFrame.pu8Data,    \
                                        vpss_frame.stFrame.u32Len, NULL, pstStream);
        pstStream->u64Ts = vpss_frame.stFrame.u64Ts;
#endif
        if(ret == AK_SUCCESS) {
            if(pstStream->enFrameType ==FRAME_TYPE_I)
            {
                printf("---venc%d_th Get Stream! type:%s len:%8u ts:%8" PRIu64 "\n",
                            venc_handle_id,
                            pstStream->enFrameType==FRAME_TYPE_P?"P":
                            pstStream->enFrameType==FRAME_TYPE_I?"I":"P",
                            pstStream->u32Len,pstStream->u64Ts);
            }

            // Write to ring buffer
            rtsp_frame_info frame_info = {0};
            frame_info.iCodeType = rtsp_param.venc_info[venc_handle_id].stChnAttrs.stVencAttrs.enOutputType;
            frame_info.iWidth  = rtsp_param.venc_info[venc_handle_id].stChnAttrs.stVencAttrs.u16Width;
            frame_info.iHeight = rtsp_param.venc_info[venc_handle_id].stChnAttrs.stVencAttrs.u16Height;
            frame_info.iFrameType = pstStream->enFrameType;
            frame_info.u64Time = pstStream->u64Ts;
            frame_info.uiSeqNo = pstStream->u64SeqNo;
            frame_info.uiDataLen = pstStream->u32Len;
            frame_info.pData = pstStream->pu8Data;
            AK_BASE_ThreadMutexLock(&rtsp_param.lock_buffer[venc_handle_id]);
            int frame_len = sizeof(rtsp_frame_info);
            char *buf_tmp = cirbuf_get_write_buf(rtsp_param.ring_buffer_id[venc_handle_id],pstStream->u32Len + frame_len);
            memcpy(buf_tmp,&frame_info,frame_len);
            memcpy(buf_tmp + frame_len,pstStream->pu8Data,pstStream->u32Len);
            cirbuf_put_write_buf(rtsp_param.ring_buffer_id[venc_handle_id],pstStream->u32Len + frame_len);
            AK_BASE_ThreadMutexUnlock(&rtsp_param.lock_buffer[venc_handle_id]);

            AK_MPI_VENC_ReleaseStream(venc_handle_id, pstStream);
        }
        else {
            if(ret != ERROR_EXT_VIDEO_ENCODRE_HAS_NO_DATA && ret != ERROR_VENC_GET_STREAM_FAILED)
                AK_BASE_PrintErrorEx(MODULE_ID_APP,
                                    "Video get stream failed .venc_handle_id:%d ret:%d %3x\n",
                                    venc_handle_id,ret,ret);

            AK_BASE_SleepMs(10);
        }
#ifndef VENC_BIND_MODE_ENABLE
        AK_MPI_VPSS_ReleaseFrame(0, venc_handle_id, &vpss_frame);
#endif
    }

exit:
    if(pstStream != NULL) {
        AK_BASE_MemFree(pstStream);
    }
#ifdef VENC_BIND_MODE_ENABLE
    ret = AK_EXT_VIDEO_SetDstChnActive(&stDstChn, AK_FALSE);
    if(ret) {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "video inactive faile.\n");
        return NULL;
    }
    ret = AK_EXT_VIDEO_UnbindChn(&stSrcChn, &stDstChn);
    if(ret) {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "video unbind faile.\n");
        return NULL;
    }
#endif
    return NULL;
}


/* end of func */
static int start_venc(int chn,int venc_thread_run_flag)
{
    int s32Ret = -1;
    static int g_VencChnId[2] = {0,1};
    VENC_ATTR_S *pstVencAttrs       = &rtsp_param.venc_info[chn].stChnAttrs.stVencAttrs;   //venc attr
    // VENC_RC_ATTRS_S *pstRcAttrs     = &rtsp_param.venc_info[chn].stChnAttrs.stRcAttrs;     //rc attr
    VENC_EXT_ATTRS_S *pstExtAttrs   = &rtsp_param.venc_info[chn].stChnAttrs.stExtAttrs;    //ext attr

    pstExtAttrs->stAdvance.enCuBlock = VENC_CUSize32x32;                //CUSize
    for(AK_S32 i = 0; i < 3; i++)
    {
        /* I/P/B min, max qp value */
        pstExtAttrs->stAdvance.u16MinQP[i] = 20;
        pstExtAttrs->stAdvance.u16MaxQP[i] = 48;
    }
    pstExtAttrs->stAdvance.s16IPDelta = 0;             //Delta
    pstExtAttrs->stAdvance.u32MaxPictureISize = 0;     //MAX I Frame, Kbits
    pstExtAttrs->stAdvance.u32MaxPicturePSize = 0;     //MAX P Frame, Kbits
    pstExtAttrs->stAdvance.u32MaxPictureBSize = 0;     ///Max B Frame, Kbits
    pstExtAttrs->stAdvance.bEnableSRD = AK_FALSE;      //SRD
    pstExtAttrs->stAdvance.u8SRDThreshold = 0;         //SRDThreshold

#ifdef ENABLE_VENC_MMA
    pstExtAttrs->stAdvance.bEnableMMA = AK_TRUE;       //Minimum Memory Allocation mode
#endif

    s32Ret = AK_MPI_VENC_Open(g_VencChnId[chn], &rtsp_param.venc_info[chn].stChnAttrs);
    if (s32Ret )
    {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "open venc chn %d failed\n", g_VencChnId[chn]);
        return s32Ret;
    }

    rtsp_param.venc_info[chn].venc_handle = g_VencChnId[chn];
    rtsp_param.venc_info[chn].chn_id = chn;

    if(venc_thread_run_flag)
    {
        s32VideoFlag = 1;
        s32Ret = AK_BASE_ThreadCreate(&venc_stream_th[chn],VencStreamThread, (void*)&rtsp_param.venc_info[chn].chn_id, ANYKA_THREAD_MIN_STACK_SIZE, THREAD_PRIO);
        if(s32Ret)
        {
            AK_BASE_PrintErrorEx(MODULE_ID_APP_VIDEO,"Create venc stream thread failed.");
        }
    }
    return 0;
}
/* end of func */

int send_stream_to_rtsp(int venc_handle_id,VIDEO_STREAM_S *pstStream)
{
    // Write to ring buffer
    rtsp_frame_info frame_info = {0};
    frame_info.iCodeType = rtsp_param.venc_info[venc_handle_id].stChnAttrs.stVencAttrs.enOutputType;
    frame_info.iWidth  = rtsp_param.venc_info[venc_handle_id].stChnAttrs.stVencAttrs.u16Width;
    frame_info.iHeight = rtsp_param.venc_info[venc_handle_id].stChnAttrs.stVencAttrs.u16Height;
    frame_info.iFrameType = pstStream->enFrameType;
    frame_info.u64Time = pstStream->u64Ts;
    frame_info.uiSeqNo = pstStream->u64SeqNo;
    frame_info.uiDataLen = pstStream->u32Len;
    frame_info.pData = pstStream->pu8Data;
    AK_BASE_ThreadMutexLock(&rtsp_param.lock_buffer[venc_handle_id]);
    int frame_len = sizeof(rtsp_frame_info);
    char *buf_tmp = cirbuf_get_write_buf(rtsp_param.ring_buffer_id[venc_handle_id],pstStream->u32Len + frame_len);
    memcpy(buf_tmp,&frame_info,frame_len);
    memcpy(buf_tmp + frame_len,pstStream->pu8Data,pstStream->u32Len);
    cirbuf_put_write_buf(rtsp_param.ring_buffer_id[venc_handle_id],pstStream->u32Len + frame_len);
    AK_BASE_ThreadMutexUnlock(&rtsp_param.lock_buffer[venc_handle_id]);

    return 0;
}

int ak_rtsp_demo_audio_env_init()
{
    int ret = 0;

    audio_enable = 1;
#ifdef AI_DENOISE_ENABLE
    if (InitAIDenoise()) {
        AK_BASE_PrintErrorEx(MODULE_ID_APP, "init AIDenoise failed\n");
        return AK_FAILED;
    }
#endif
    ret = init_ai(&rtsp_param.audio);
    ret = init_aenc(&rtsp_param.audio);
    s32AudioFlag = 1;
    AK_BASE_ThreadCreate(&(get_tid), GetAudioStream, NULL,ANYKA_THREAD_MIN_STACK_SIZE, 19);
    AK_BASE_ThreadCreate(&(encode_tid),GetAudioFrameEncode, NULL, ANYKA_THREAD_MIN_STACK_SIZE, 19);

    return 0;
}

int ak_rtsp_demo_audio_env_destory()
{
    //Audio Stop
    s32AudioFlag = 0;
    if(encode_tid)
    {
        AK_BASE_ThreadJoin(encode_tid);
    }

    if(get_tid)
    {
        AK_BASE_ThreadJoin(get_tid);
    }

    destroy_ai_aenc();

#ifdef AI_DENOISE_ENABLE
    DestroyAIDenoise();
#endif

    return 0;
}

int ak_rtsp_demo_video_env_init()
{
    int ret = 0;

    AK_BASE_ThreadMutexInit(&rtsp_param.lock_buffer[0], NULL);
    AK_BASE_ThreadMutexInit(&rtsp_param.lock_buffer[2], NULL);

    //main stream ring buffer
    rtsp_param.ring_buffer_id[0] = cirbuf_new(4* 1024 *1024);

    //audio stream ring buffer
    rtsp_param.ring_buffer_id[2] = cirbuf_new(256 *1024);

    ret = init_vi(VIDEO_DEV0);
    ret = start_vi(VIDEO_DEV0);
    s32VideoFlag = 1;
    ret = start_venc(0,g_venc_thread_run_flag);

    return 0;
}

int ak_rtsp_demo_video_env_destory()
{
    //Video Stop
    if(g_venc_thread_run_flag)
    {
        s32VideoFlag = 0;
        if (venc_stream_th[0])
        {
            AK_BASE_ThreadJoin(venc_stream_th[0]);
            venc_stream_th[0] = 0;
        }
    }

    AK_MPI_VENC_Close(rtsp_param.venc_info[0].venc_handle);
    stop_destory_vi(VIDEO_DEV0);

    cirbuf_delete(rtsp_param.ring_buffer_id[0]);
    cirbuf_delete(rtsp_param.ring_buffer_id[2]);

    AK_BASE_ThreadMutexDestroy(&rtsp_param.lock_buffer[0]);
    AK_BASE_ThreadMutexDestroy(&rtsp_param.lock_buffer[2]);
    return 0;
}


int ak_rtsp_demo_env_init(char* isp_conf_path,int venc_thread_run_flag, rtsp_event_callback_t callback)
{
    int ret = 0;

    g_venc_thread_run_flag = venc_thread_run_flag;
    rtsp_ini_param(isp_conf_path);

    AK_BASE_ThreadMutexInit(&rtsp_param.lock_buffer[0], NULL);
    AK_BASE_ThreadMutexInit(&rtsp_param.lock_buffer[2], NULL);

    //main stream ring buffer
    rtsp_param.ring_buffer_id[0] = cirbuf_new(2* 1024 *1024);
    //audio stream ring buffer
    rtsp_param.ring_buffer_id[2] = cirbuf_new(256 *1024);
    rtsp_server_init(callback);

    ret = init_vi(VIDEO_DEV0);
    ret = start_vi(VIDEO_DEV0);
    s32VideoFlag = 1;
    ret = start_venc(0,venc_thread_run_flag);

    ret = init_ai(&rtsp_param.audio);
    ret = init_aenc(&rtsp_param.audio);
    s32AudioFlag = 1;
    AK_BASE_ThreadCreate(&(get_tid), GetAudioStream, NULL,ANYKA_THREAD_MIN_STACK_SIZE, 19);
    AK_BASE_ThreadCreate(&(encode_tid),GetAudioFrameEncode, NULL, ANYKA_THREAD_MIN_STACK_SIZE, 19);

    return 0;
}

int ak_rtsp_demo_env_destory(int venc_thread_run_flag)
{
    //Video Stop
    if(venc_thread_run_flag)
    {
        s32VideoFlag = 0;
        if (venc_stream_th[0])
        {
            AK_BASE_ThreadJoin(venc_stream_th[0]);
            venc_stream_th[0] = 0;
        }
    }

    destroy_ai_aenc();
    AK_MPI_VENC_Close(rtsp_param.venc_info[0].venc_handle);
    stop_destory_vi(VIDEO_DEV0);

    rtsp_server_deinit();

    cirbuf_delete(rtsp_param.ring_buffer_id[0]);
    cirbuf_delete(rtsp_param.ring_buffer_id[2]);

    AK_BASE_ThreadMutexDestroy(&rtsp_param.lock_buffer[0]);
    AK_BASE_ThreadMutexDestroy(&rtsp_param.lock_buffer[2]);

    return 0;
}
