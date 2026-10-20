/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd. */
#ifndef _AK_OSD_H_
#define _AK_OSD_H_

#include "ak_types.h"


typedef enum akOSD_ERRNO_E {
    ERROR_OSD_NOT_INIT                  = ( MODULE_ID_OSD << 24 ) + 0,
    ERROR_OSD_CREATE_CANVAS_FAILED		= ( MODULE_ID_OSD << 24 ) + 1,
    ERROR_OSD_CREATE_REGIONS_FAILED     = ( MODULE_ID_OSD << 24 ) + 2,
    ERROR_OSD_SET_COLOR_FAILED          = ( MODULE_ID_OSD << 24 ) + 3,
    ERROR_OSD_BIND_FAILED               = ( MODULE_ID_OSD << 24 ) + 4,
    ERROR_OSD_UPDATE_FAILED             = ( MODULE_ID_OSD << 24 ) + 5,
}OSD_ERRNO_E;

typedef enum akOSD_COLOR_MODE_E
{
    AK_OSD_COLOR_FMT_YUV = 0,
    AK_OSD_COLOR_FMT_RGB,
    AK_OSD_COLOR_FMT_SUPPORT_NUM
}OSD_COLOR_MODE_E;

typedef enum akOSD_COLOR_TYPE_E
{
    AK_OSD_TYPE_COLOR = 0,
    AK_OSD_TYPE_MONO,
    AK_OSD_TYPE_SUPPORT_NUM
}OSD_COLOR_TYPE_E;

typedef struct akOSD_ATTR_S
{
    OSD_COLOR_MODE_E enColorModeIndex;          // YUV or RGB
    AK_U32 *pu32ColorTable;                     // color table，can be null
    OSD_COLOR_TYPE_E enColorType;               //color or mono, default color
}OSD_ATTR_S;

typedef struct akOSD_VECTOR_S
{
    AK_U32 u32X;
    AK_U32 u32Y;
}OSD_VECTOR_S;

typedef struct akOSD_CANVAS_INFO_S
{
    OSD_VECTOR_S stPosInYuv;      //position in yuv
    AK_S32 s32CanvasSizeHeight;   //height of canvas
    AK_S32 s32CanvasSizeWidth;    //width of canvas
    AK_S32 s32CanvasColorIndex;   //background color of canvas
    AK_S32 s32Alpha;              //unsupport
    AK_S32 s32BitWidth;           //bitwidth
}OSD_CANVAS_INFO_S;

typedef enum akOSD_BIND_TYPE_E
{
    OSD_BIND_TYPE_CAVNVAS = 0,
    OSD_BIND_TYPE_REGION,
}OSD_BIND_TYPE_E;

typedef struct akOSD_BIND_CHN_INFO_S
{
    AK_S32  s32BindChannelId; //vpss channel id
    AK_S32  s32DeviceId;      //device id of channel
}OSD_BIND_CHN_INFO_S;

// Canvas bind info
typedef struct akOSD_BIND_CANVAS_INFO_S
{
    AK_S32  s32CanvasId;          //canvas id
    AK_S32  s32BindRegionNum;     //total region number
    AK_S32  s32BindChannelNum;    //total channel number
    AK_S32  s32BindRgionId[64];   //region id
    OSD_BIND_CHN_INFO_S stBindChnInfo[64]; //bind info
}OSD_BIND_CANVAS_INFO_S;


typedef struct akOSD_BIND_REGION_INFO_S
{
    AK_S32  s32RegionId;          //region id that want to query
    AK_S32  s32BindCanvasNum;     //total canvas number that had been bind
    AK_S32  s32CoverRegionNum;    //total cover region number
    AK_S32  s32BindCanvasId[64];  //canvas id of region bind
    AK_S32  s32CoverRegionId[64]; //cover region id
    OSD_VECTOR_S stPos;           //region location
}OSD_BIND_REGION_INFO_S;

typedef struct akOSD_BIND_INFO_S{
    OSD_BIND_TYPE_E     enType; //type：canvas/region
    union{
        OSD_BIND_CANVAS_INFO_S stCanvasBindInfo; //canvas bind info
        OSD_BIND_REGION_INFO_S stRegionBindInfo; //region bind info
    }info;
}OSD_BIND_INFO_S;


typedef struct akOSD_REGION_ATTR_S{
    AK_S32 s32Priority;                         //priority of cover region
    AK_S32 s32Width;                            //width of region
    AK_S32 s32Height;                           //hegiht of region
}OSD_REGION_ATTR_S;

/* OSD region type */
typedef enum akOSD_REG_TYPE_E {
    OSD_REG_INV = 0,     //undefine
    OSD_REG_LINE = 1,    //line
    OSD_REG_FONT = 2,    //font
    OSD_REG_BITMAP = 3,  //bitmap
}OSD_REG_TYPE_E;

/* OSD bitmap info */
typedef struct akOSD_BITMAP_ATTR_S
{
    AK_S32 s32BitmapHeight;
    AK_S32 s32BitmapWidth;
    AK_S32 s32BitPerPixel;
}OSD_BITMAP_ATTR_S;

/* OSD bitmap info */
typedef struct akOSD_BITMAP_INFO_S
{
    AK_U8                   *pu8Buffer;
    AK_U32                  u32BufferLength;
    OSD_BITMAP_ATTR_S       stOsdBmpAttr;
}OSD_BITMAP_INFO_S;

typedef struct akOSD_CHANGE_FONT_COLOR_S{
    AK_S32 s32FontColor;            //font color
    AK_S32 s32BgColor;              //font background color
    AK_S32 s32EdgeColor;            //font edge color
}OSD_CHANGE_FONT_COLOR_S;

/* font attribute struct */
typedef struct akOSD_FONT_ATTR_S{
    AK_S32 s32FontFileSize;         //font file size
    const AK_CHAR *pcfile;          //font file path
    AK_S32 s32FontSize;             //font size in screen
    AK_S32 s32FontInterval;         //font interval
    const AK_U16 *pu16Str;          //font string
    AK_S32 s32StrLen;               //string len
    OSD_CHANGE_FONT_COLOR_S *pstFontColor; //font color
}OSD_FONT_ATTR_S;

/*
 * line data
 * unsupport
 */
typedef struct akLINE_POSITION_ATTR_S{
    AK_S32          s32Num;
    OSD_VECTOR_S    *pstPoint;
    AK_S32          s32Color;
    AK_S32          s32Thickness;
}LINE_POSITION_ATTR_S;


typedef struct akOSD_REGION_DATA_S{
    OSD_REG_TYPE_E enType;                   //OSD region type
    AK_S32 s32ForceWriteFlag;                //force write font data when the bitmap size bigger than canvas size.
    union{
        OSD_BITMAP_INFO_S stOsdBitmapInfo;          //bitmap data
        OSD_FONT_ATTR_S stFontAttr;                 //string data
        LINE_POSITION_ATTR_S stLinePositionAttr;    //line data
    }data;
}OSD_REGION_DATA_S;


/**
 *@brief AK_MPI_OSD_Init : init color table .
 *@param pstOsdAttrValue[IN] : init param.
 *@return 0 success, others failed.
 *@note call it at the fist time .
*/
AK_S32 AK_MPI_OSD_Init(OSD_ATTR_S *pstOsdAttrValue);

/**
 *@brief AK_MPI_OSD_CreateCanvas : 1.create canvas,
                                   2.request canvas buffer .
 *@param s32CanvasId[IN] : canvas id from user, rang: 0-15.
 *@param pstOsdCanvasInfo[IN] : canvas param
 *@return 0 success, others failed.
 *@note API call: 1.AK_MPI_OSD_Init.
                  2.AK_MPI_OSD_CreateCanvas.
*/
AK_S32 AK_MPI_OSD_CreateCanvas(AK_S32 s32CanvasId, OSD_CANVAS_INFO_S *pstOsdCanvasInfo);

/**
 *@brief AK_MPI_OSD_BindVi : 1.bind vpss channel and canvas id.
                             2.set osd buffer info to driver.
                             3.get osd buffer vaddr from driver.
                             4.set canvas background color.
 *@param s32CanvasId[IN] : canvas id from user, rang: 0-15.
 *@param s32ChannelId[IN] : vpss channel id.
 *@param s32DeviceId[IN] : device id of vpss channel.
 *@return 0 success, others failed.
 *@note API call: 1.AK_MPI_OSD_CreateCanvas.
                  2.AK_MPI_OSD_BindVi.
*/
 AK_S32 AK_MPI_OSD_BindVi(AK_S32 s32CanvasId, AK_S32 s32ChannelId, AK_S32 s32DeviceId);

/**
 *@brief AK_MPI_OSD_UnbindVi : 1.unbind vpss channel and canvas id.
                               2.clean bind info in platform.
 *@param s32CanvasId[IN] : canvas id from user, rang: 0-15.
 *@param s32ChannelId[IN] : vpss channel id.
 *@param s32DeviceId[IN] : device id of vpss channel.
 *@return 0 success, others failed.
 *@note API call: 1.AK_MPI_OSD_BindVi.
                  2.AK_MPI_OSD_UnbindVi.
*/
AK_S32 AK_MPI_OSD_UnbindVi(AK_S32 s32CanvasId, AK_S32 s32ChannelId, AK_S32 s32DeviceId);

/**
 *@brief AK_MPI_OSD_DestroyCanvas : 1.destroy canvas.
                                    2.clear canvas to driver.
                                    3.free canvas buffer.
 *@param s32CanvasId[IN] : canvas id from user, rang: 0-15.
 *@return 0 success, others failed.
 *@note API call: 1.AK_MPI_OSD_CreateCanvas.
                  2.AK_MPI_OSD_DestroyCanvas.
*/
AK_S32 AK_MPI_OSD_DestroyCanvas(AK_S32 s32CanvasId);

/**
 *@brief AK_MPI_OSD_CleanCanvas : set color 0 to canvas.
 *@param s32CanvasId[IN] : canvas id from user, rang: 0-15.
 *@return 0 success, others failed.
 *@note API call: 1.AK_MPI_OSD_CreateCanvas.
                  2.AK_MPI_OSD_CleanCanvas.
                  3.AK_MPI_OSD_ShowCanvas.
*/
AK_S32 AK_MPI_OSD_CleanCanvas(AK_S32 s32CanvasId);

/**
 *@brief AK_MPI_OSD_GetCanvasAttr : get canvas attr.
 *@param s32CanvasId[IN] : canvas id from user, rang: 0-15.
 *@param pstCanvasInfo[OUT] : canvas attr.
 *@return 0 success, others failed.
 *@note API call: 1.AK_MPI_OSD_CreateCanvas .
                  2.AK_MPI_OSD_GetCanvasAttr.
*/
AK_S32 AK_MPI_OSD_GetCanvasAttr(AK_S32 s32CanvasId, OSD_CANVAS_INFO_S *pstCanvasInfo);

/**
 *@brief AK_MPI_OSD_SetCanvasAttr : set canvas attr.
 *@param s32CanvasId[IN] : canvas id from user, rang: 0-15.
 *@param pstCanvasInfo[IN] : canvas attr
 *@return 0 success, others failed.
 *@note API call: 1.AK_MPI_OSD_CreateCanvas.
                  2.AK_MPI_OSD_SetCanvasAttr.
*/
AK_S32 AK_MPI_OSD_SetCanvasAttr(AK_S32 s32CanvasId, OSD_CANVAS_INFO_S *pstCanvasInfo);

/**
 *@brief AK_MPI_OSD_ShowCanvas : set canvas data to driver and show it on screen.
 *@param s32CanvasId[IN] : canvas id from user, rang: 0-15.
 *@return 0 success, others failed.
 *@note API call: 1.AK_MPI_OSD_UpdateRegion.
                  2.AK_MPI_OSD_ShowCanvas.
*/
AK_S32 AK_MPI_OSD_ShowCanvas(AK_S32 s32CanvasId);

/**
 *@brief AK_MPI_OSD_GetVersion ：Get osd mpi version
 *@param
 *@return 0 success, others failed.
 *@note
*/
const AK_CHAR* AK_MPI_OSD_GetVersion(AK_VOID);

/**
 *@brief AK_MPI_OSD_GetBindInfo : get canvas or region bind info.
 *@param s32Id[IN] : canvas id or region id
 *@param pstOsdBindInfo[OUT] : bind info
 *@return 0 success, others failed.
 *@note API call:  1.AK_MPI_OSD_CreateCanvas(CanvasId);  or   1.AK_MPI_OSD_CreateRegion(RegionId);
                   2.AK_MPI_OSD_GetBindInfo(CanvasId);        2.AK_MPI_OSD_GetBindInfo(RegionId);

*/
AK_S32 AK_MPI_OSD_GetBindInfo(AK_S32 s32Id,OSD_BIND_INFO_S *pstOsdBindInfo);

/**
 *@brief AK_MPI_OSD_CreateRegion : 1.crete region.
                                   2.record OSD_REGION_ATTR_S in platform.
 *@param s32RegionId[IN] : region id no range
 *@param pstRegionAttr[IN] : region attr
 *@return 0 success, others failed.
 *@note API call: 1.AK_MPI_OSD_Init .
                  2.AK_MPI_OSD_CreateRegion.
*/
AK_S32 AK_MPI_OSD_CreateRegion(AK_S32 s32RegionId,OSD_REGION_ATTR_S *pstRegionAttr);

/**
 *@brief AK_MPI_OSD_AttachRegion : 1.attatch canvas and region.
                                   2.record stPos in platform.
 *@param stPos[IN] : region location in canvas.(bitmap begin location in canvas).
 *@param s32RegionId[IN] : region id.
 *@param s32CanvasId[IN] : canvas id.
 *@return 0 success, others failed.
 *@note API call:  1.AK_MPI_OSD_CreateCanvas.
                   2.AK_MPI_OSD_CreateRegion.
                   3.AK_MPI_OSD_AttachRegion.
*/
AK_S32 AK_MPI_OSD_AttachRegion(OSD_VECTOR_S stPos,AK_S32 s32RegionId,AK_S32 s32CanvasId);

/**
 *@brief AK_MPI_OSD_DetachRegion : 1.detach canvas and region.
                                   2.clean attach info in platform.
 *@param s32RegionId[IN] : region id.
 *@param s32CanvasId[IN] : canvas id.
 *@return 0 success, others failed.
 *@note API call:  1.AK_MPI_OSD_AttachRegion.
                   2.AK_MPI_OSD_DetachRegion.
*/
AK_S32 AK_MPI_OSD_DetachRegion(AK_S32 s32RegionId,AK_S32 s32CanvasId);

/**
 *@brief AK_MPI_OSD_GetRegionAttr : get region attr.
 *@param s32RegionId[IN] : region id.
 *@param pstRegionAttr[OUT] : region attr.
 *@return 0 success, others failed.
 *@note API call: 1.AK_MPI_OSD_CreateRegion .
                  2.AK_MPI_OSD_GetRegionAttr.
*/
AK_S32 AK_MPI_OSD_GetRegionAttr(AK_S32 s32RegionId, OSD_REGION_ATTR_S *pstRegionAttr);

/**
 *@brief AK_MPI_OSD_SetRegionAttr : set region attr.
 *@param s32RegionId[IN] : region id.
 *@param pstRegionAttr[IN] : region attr
 *@return 0 success, others failed.
 *@note API call: 1.AK_MPI_OSD_CreateRegion.
                  2.AK_MPI_OSD_SetRegionAttr.
*/
AK_S32 AK_MPI_OSD_SetRegionAttr(AK_S32 s32RegionId, OSD_REGION_ATTR_S *pstRegionAttr);

/**
 *@brief AK_MPI_OSD_DestroyRegion : 1.destroy region.
                                    2.clean region data(bitmap) in canvas.
 *@param s32RegionId[IN] : region id.
 *@return 0 success, others failed.
 *@note API call: 1.AK_MPI_OSD_CreateRegion .
                  2.AK_MPI_OSD_DestroyRegion.
*/
AK_S32 AK_MPI_OSD_DestroyRegion(AK_S32 s32RegionId);

/**
 *@brief AK_MPI_OSD_Destroy : osd exit.
                              1.destroy canvas.
                              2.destroy region.
                              3.InitFlag = AK_FALSE.
 *@param
 *@return 0 success, others failed.
 *@note API call: 1.AK_MPI_OSD_Init.
                  2.AK_MPI_OSD_Destroy.
*/
AK_S32 AK_MPI_OSD_Destroy(AK_VOID);

/**
 *@brief AK_MPI_OSD_GetStrBmp : get font bitmap.
 *@param pstFontAttr[IN] : font attr
 *@param pstOutBitmapInfo[OUT] : bitmap
 *@return 0 success, others failed.
 *@note
*/
AK_S32 AK_MPI_OSD_GetStrBmp(OSD_FONT_ATTR_S *pstFontAttr, OSD_BITMAP_INFO_S *pstOutBitmapInfo);

/**
 *@brief AK_MPI_OSD_ReleaseBmpBuf : release bitmap memory.
 *@param pstOutBitmapInfo[IN] : bitmap
 *@param s32Len[IN] : font len
 *@return 0 success, others failed.
 *@note API call: 1.AK_MPI_OSD_GetStrBmp.
                  2.AK_MPI_OSD_ReleaseBmpBuf.
*/
AK_S32 AK_MPI_OSD_ReleaseBmpBuf(OSD_BITMAP_INFO_S *pstOutBitmapInfo, AK_S32 s32Len);

/**
 *@brief AK_MPI_OSD_UpdateRegion : update region data.
 *@param s32RegionId[IN] : region id
 *@param s32Status[IN] : 1:enable. (wirte bitmap data to canvas).
                         0: disable..(clean region data to canvas).
 *@param pstRegionData[IN] : region data.
 *@param s32RegionId[IN] : region id.
 *@return 0 success, others failed.
 *@note API call: 1.AK_MPI_OSD_CreateRegion.
                  2.AK_MPI_OSD_UpdateRegion.
                  3.AK_MPI_OSD_ShowCanvas.
*/
AK_S32 AK_MPI_OSD_UpdateRegion(AK_S32 s32RegionId, AK_S32 s32Status, OSD_REGION_DATA_S *pstRegionData);


#endif
/* end of file */
