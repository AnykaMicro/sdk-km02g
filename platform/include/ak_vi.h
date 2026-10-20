/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd. */
#ifndef _AK_VI_H_
#define _AK_VI_H_
#include "ak_types.h"
#include "ak_common_video.h"

#define VI_DEV_MAX  4

/* vi module errno list */
typedef enum akVI_ERRNO_E {
    ERROR_VI_INVALID_DEVID					= ( MODULE_ID_VI << 24 ) + 0,
    ERROR_VI_DEVID_NOT_OPEN					= ( MODULE_ID_VI << 24 ) + 1,
    ERROR_VI_DEVID_NOT_MATCH				= ( MODULE_ID_VI << 24 ) + 2,
    ERROR_VI_ATTR_PARM_WRONG				= ( MODULE_ID_VI << 24 ) + 3,
    ERROR_VI_INVALID_CHNID					= ( MODULE_ID_VI << 24 ) + 4,
    ERROR_VI_DEV_NOT_ENABLE					= ( MODULE_ID_VI << 24 ) + 5,
    ERROR_VI_DEV_NOT_DISABLE				= ( MODULE_ID_VI << 24 ) + 6,
    ERROR_VI_DEV_REGISTER_FAIL				= ( MODULE_ID_VI << 24 ) + 7,
    ERROR_VI_CHANNEL_REGISTER_FAIL			= ( MODULE_ID_VI << 24 ) + 8,
    ERROR_VI_CHANNEL_DEVICE_NOT_OPEN		= ( MODULE_ID_VI << 24 ) + 9,
    ERROR_VI_CHANNEL_NOT_EXIST				= ( MODULE_ID_VI << 24 ) + 10,
    ERROR_VI_CHANNEL_NOT_ENABLE				= ( MODULE_ID_VI << 24 ) + 11,
    ERROR_VI_CHANNEL_NOT_DISABLE			= ( MODULE_ID_VI << 24 ) + 12,
    ERROR_VI_CHANNEL_ID_NOT_MATCH			= ( MODULE_ID_VI << 24 ) + 13,
    ERROR_VI_SET_BOX_COLOR_TABLE_ERROR     	= ( MODULE_ID_VI << 24 ) + 14,
    ERROR_VI_DRAW_BOX_ERROR     			= ( MODULE_ID_VI << 24 ) + 15,
    ERROR_VI_ISP_CONF_NOT_LOADED   			= ( MODULE_ID_VI << 24 ) + 16,
    ERROR_VI_SET_CHN_STATUS		   			= ( MODULE_ID_VI << 24 ) + 17,
    ERROR_VI_RESET_CHN_CROP_FAIL            = ( MODULE_ID_VI << 24 ) + 18,
    ERROR_VI_CHANNEL_ALLOC_BUFF_FAIL        = ( MODULE_ID_VI << 24 ) + 19,
    ERROR_VI_ERROR_MODE                     = ( MODULE_ID_VI << 24 ) + 20,
    ERROR_VI_NO_WORK                        = ( MODULE_ID_VI << 24 ) + 21,
    ERROR_VI_DEV_CREATED_YET                = ( MODULE_ID_VI << 24 ) + 22,
    ERROR_VI_DEV_GET_PAD_ID                 = ( MODULE_ID_VI << 24 ) + 23,
    ERROR_VI_BIND_VPSS_GRP                  = ( MODULE_ID_VI << 24 ) + 24,
}VI_ERRNO_E;

//interface mode.
typedef enum akVI_INTF_MODE_E {
    VI_INTF_DVP = 0,								//DVP Parallel interface
    VI_INTF_MIPI_1,									//MIPI Serial Single-Line Interface.
    VI_INTF_MIPI_2,									//MIPI Serial Two-Line Interface.
    VI_INTF_MULTI_DVP,                              //Multiple camera DVP interface.
    VI_INTF_MULTI_MIPI_1,                           //Multiple camera MIPI Single-Line Interface.
    VI_INTF_MULTI_MIPI_2,                           //Multiple camera MIPI Two-Line Interface.
    VI_INTF_TOTAL
}VI_INTF_MODE_E;

//channel mode.
//VI_CHN_MODE_USER : this mode is used to get raw data from AK_MPI_VI_GetFrame, this mode will allocate
//w*h*3/2*frameDepth KB of space to store raw data.
//VI_CHN_MODE_AUTO : without get raw data. other is the same as VI_CHN_MODE_USER.
typedef enum akVI_CHN_MODE_E
{
    VI_CHN_MODE_USER,
    VI_CHN_MODE_AUTO,
    VI_CHN_MODE_TOTAL
}VI_CHN_MODE_E;

/**
 * @struct  VI_DEV_ATTR_S
 * @brief   vi device attribute.
 * @note
 * @example
 */
typedef struct akVI_DEV_ATTR_S
{
    AK_U32				u32Width;           //vi dev width
    AK_U32				u32Height;          //vi dev height
    CROP_INFO_S			stCropInfo;         //vi crop info
    VI_INTF_MODE_E		enIntfMode;         //vi interface mode
    PIXEL_FORMAT_E 	    enInputPixFmt;      //vi dev input pixel formate
    AK_U32 				u32Fps;             //sensor input fps
    AK_BOOL				bMirror;            //if mirror
    AK_BOOL				bFlip;              //if flip
}VI_DEV_ATTR_S;

/**
 * @struct  VI_CHN_ATTR_S
 * @brief   vi channel attribute.
 * @note
 * @example
 */
typedef struct akVI_CHN_ATTR_S
{
    AK_U32				u32Width;           //vi chn width
    AK_U32				u32Height;          //vi chn height
    AK_U32				u32MaxPerFrameSize; //vi chn buffer size
    PIXEL_FORMAT_E 	    enOutputPixFmt;     //vi chn ouput pixel formate
    AK_U32 				u32Fps;             //vi chn output fps
    AK_U32				u32FrameDepth;      //vi chn frame depth
    VI_CHN_MODE_E		enMode;             //vi chn mode
}VI_CHN_ATTR_S;

/**
 * @struct  VI_FRAME_INFO_S
 * @brief   vi channel frame info.
 * @note
 * @example
 */
typedef struct akVI_FRAME_INFO_S
{
    FRAME_S				stFrame;            //vi chn frame data
    AK_U64				u64PhyAddr;         //vi frame phy address
    PIXEL_FORMAT_E 	    enOutputPixFmt;     //vi ouput pixel formate type
    AK_VOID				*pvMdInfo;          //vi move detection info
}VI_FRAME_INFO_S;


/**
 * @struct  VI_RECTANGLE_S
 * @brief   vi res.
 * @note
 * @example
 */
typedef struct akVI_RECTANGLE_S{
    AK_U32	u32Width;									//width lenght of resolution
    AK_U32	u32Height;									//height lenght of resolution
}VI_RECTANGLE_S;

/**
 *@brief AK_MPI_VI_GetVersion ：Get vi mpi version
 *@param
 *@return 0 success, others failed.
 *@note
*/
const AK_CHAR *AK_MPI_VI_GetVersion(void);

/**
 *@brief AK_MPI_VI_CreateDev ：create vi device
 *@param s32DevId[IN] : device id
 *@param pstViDevAttr[IN] : device attributes.
 *@return 0 success, others failed.
 *@note call it at the fist time .
*/
AK_S32 AK_MPI_VI_CreateDev(AK_S32 s32DevId,const VI_DEV_ATTR_S *pstViDevAttr);

/**
 *@brief AK_MPI_VI_CreateDev : destroy vi device
 *@param s32DevId[IN] : device id
 *@return 0 success, others failed.
 *@note call it at the final time .
*/
AK_S32 AK_MPI_VI_DestroyDev(AK_S32 s32DevId);

/**
 *@brief AK_MPI_VI_SetDevAttr : set vi device attribute
 *@param s32DevId[IN] : device id
 *@param pstViDevAttr[IN] : pinter to device attribute
 *@return 0 success, others failed.
 *@note call it after AK_MPI_VI_CreateDev .
*/
AK_S32 AK_MPI_VI_SetDevAttr(AK_S32 s32DevId, const VI_DEV_ATTR_S *pstViDevAttr);

/**
 *@brief AK_MPI_VI_GetDevAttr : get device attribute.
 *@param s32DevId[IN] : device id
 *@param pstViDevAttr[OUT] : pinter to device attribute
 *@return 0 success, others failed.
 *@note call it after AK_MPI_VI_SetDevAttr .
*/
AK_S32 AK_MPI_VI_GetDevAttr(AK_S32 s32DevId, VI_DEV_ATTR_S *pstViDevAttr);

/**
 *@brief AK_MPI_VI_EnableDev : enable vicap device .
 *@param s32DevId[IN] : device id
 *@return 0 success, others failed.
 *@note call it after AK_MPI_VI_SetDevAttr .
*/
AK_S32 AK_MPI_VI_EnableDev(AK_S32 s32DevId);

/**
 *@brief AK_MPI_VI_DisableDev : disable vicap device .
 *@param s32DevId[IN] : device id
 *@return 0 success, others failed.
 *@note call it after AK_MPI_VI_SetDevAttr .
*/
AK_S32 AK_MPI_VI_DisableDev(AK_S32 s32DevId);

/**
 *@brief AK_MPI_VI_SetChnAttr : set vicap chn attribute.
 *@param s32DevId[IN] : device id
 *@param s32ChnId[IN] : Channel id
 *@param pstViChnAttr[IN] : pinter to chn attribute.
 *@return 0 success, others failed.
 *@note call it after AK_MPI_VI_CreateDev .
*/
AK_S32 AK_MPI_VI_SetChnAttr(AK_S32 s32DevId, AK_S32 s32ChnId, const VI_CHN_ATTR_S *pstViChnAttr);

/**
 *@brief AK_MPI_VI_GetChnAttr : Get vicap chn attribute.
 *@param s32DevId[IN] : device id
 *@param s32ChnId[IN] : Channel id
 *@param pstViChnAttr[OUT] : pinter to chn attribute.
 *@return 0 success, others failed.
 *@note call it after AK_MPI_VI_CreateDev .
*/
AK_S32 AK_MPI_VI_GetChnAttr(AK_S32 s32DevId, AK_S32 s32ChnId, VI_CHN_ATTR_S *pstViChnAttr);

/**
 *@brief AK_MPI_VI_BindVpssGrp ：bind vpss group.
 *@param s32DevId[IN] : vi device id.
 *@param s32ChnId[IN]: vi chn id.
 *@param u32VpssGrp[IN]: vpss grp id.
 *@return 0 success, others failed.
 *@note
*/
AK_S32 AK_MPI_VI_BindVpssGrp(AK_S32 s32DevId, AK_S32 s32ChnId, AK_S32 s32VpssGrp);

/**
 *@brief AK_MPI_VI_EnableChn : enable vicap chn .
 *@param s32DevId[IN] : device id
 *@param s32ChnId[IN] : Channel id
 *@return 0 success, others failed.
 *@note call it after AK_MPI_VI_SetChnAttr .
*/
AK_S32 AK_MPI_VI_EnableChn(AK_S32 s32DevId, AK_S32 s32ChnId);

/**
 *@brief AK_MPI_VI_DisableChn : disable vicap chn .
 *@param s32DevId[IN] : device id
 *@param s32ChnId[IN] : Channel id
 *@return 0 success, others failed.
 *@note call it after AK_MPI_VI_SetChnAttr .
*/
AK_S32 AK_MPI_VI_DisableChn(AK_S32 s32DevId, AK_S32 s32ChnId);

/**
 *@brief AK_MPI_VI_GetFrame : get vicap raw frame .
 *@param s32DevId[IN] : device id
 *@param s32ChnId[IN] : Channel id
 *@param pstFrameInfo[OUT] : frame info
 *@param s32MilliSec[IN] : -1：blocking and waiting, 0:non-blocking, >0 : it represent a waiting time, and if it
    exceeds the set time, it times out and exits, with the unit of time being milliseconds.
 *@return 0 success, others failed.
 *@note call it after AK_MPI_VI_EnableChn, only work in user mode .
*/
AK_S32 AK_MPI_VI_GetFrame(AK_S32 s32DevId, AK_S32 s32ChnId, VI_FRAME_INFO_S *pstFrameInfo, AK_S32 s32MilliSec);

/**
 *@brief AK_MPI_VI_ReleaseFrame : release frame.
 *@param s32DevId[IN] : device id
 *@param s32ChnId[IN] : Channel id
 *@param pstFrameInfo[IN] : frame info
 *@return 0 success, others failed.
 *@note call it after AK_MPI_VI_GetFrame, only work in user mode .
*/
AK_S32 AK_MPI_VI_ReleaseFrame(AK_S32 s32DevId, AK_S32 s32ChnId, VI_FRAME_INFO_S *pstFrameInfo);

/**
 *@brief AK_MPI_VI_GetSensorId get sensor id from vi device driver.
 *@param s32DevId[IN] : device id.
 *@param pu32SensorId[OUT]: sensor id.
 *@return 0 success, others failed.
 *@note vpss device attribute need call this to catch sensorId.
*/
AK_S32 AK_MPI_VI_GetSensorId(AK_S32 s32DevId, AK_U32 *pu32SensorId);

/**
 *@brief AK_MPI_VI_GetSensorFps get sensor fps from vi device driver.
 *@param s32DevId[IN] : device id.
 *@param pu32SensorFps[OUT]: sensor fps.
 *@return 0 success, others failed.
 *@note
*/
AK_S32 AK_MPI_VI_GetSensorFps(AK_S32 s32DevId, AK_U32 *pu32SensorFps);

/**
 *@brief AK_MPI_VI_SetSensorFps set sensor fps from vi device driver.
 *@param s32DevId[IN] : device id.
 *@param u32SensorFps[IN]: sensor fps.
 *@return 0 success, others failed.
 *@note
*/
AK_S32 AK_MPI_VI_SetSensorFps(AK_S32 s32DevId, AK_U32 u32SensorFps);

/**
 *@brief AK_MPI_VI_GetSensorRes get sensor res.
 *@param s32DevId[IN] : device id.
 *@param res[OUT]: sensor res.
 *@return 0 success, others failed.
 *@note
*/
AK_S32 AK_MPI_VI_GetSensorRes(AK_S32 s32DevId, VI_RECTANGLE_S *pstRes);

#endif