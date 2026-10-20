/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd. */
#ifndef __AK_IPU_H__
#define __AK_IPU_H__

#define AK_IPU_MAX_QUE 2

typedef enum akIPU_ERROR_E {
    ERROR_IPU_OPEN_FAILURE              = ( MODULE_ID_IPU << 24 ) + 0,
    ERROR_IPU_ALREADY_OPEN              = ( MODULE_ID_IPU << 24 ) + 1,
    ERROR_IPU_NOT_OPENED                = ( MODULE_ID_IPU << 24 ) + 2,
    ERROR_IPU_IPU_FLUSH_CACHE_FAILURE   = ( MODULE_ID_IPU << 24 ) + 3,
    ERROR_IPU_CLOSE_FAILURE             = ( MODULE_ID_IPU << 24 ) + 4,
    ERROR_IPU_MUTEX_INIT_FAILURE        = ( MODULE_ID_IPU << 24 ) + 5,
    ERROR_IPU_MUTEX_CLOSE_FAILURE       = ( MODULE_ID_IPU << 24 ) + 6,
    ERROR_IPU_IOCTL_FAILURE             = ( MODULE_ID_IPU << 24 ) + 7,
    ERROR_IPU_QUEUE_NUM                 = ( MODULE_ID_IPU << 24 ) + 8,
    ERROR_IPU_VALUE_ILLEGAL             = ( MODULE_ID_IPU << 24 ) + 9,
}IPU_ERROR_E;

typedef struct akIPU_ATTR_S                     // IPU device attributes
{
    AK_U32 au32QueSize[AK_IPU_MAX_QUE];         // Queue memory size for each queue
    AK_U32 u32RespSize;                         // Response queue memory size
    AK_U32 u32TransBlendQueSize;                // DMA size for distortion correction, derived from conf
}IPU_ATTR_S;

typedef enum akIPU_LAYER_E
{
    IPU_LAYER_TMP,
}IPU_LAYER_E;


typedef enum akIPU_BLEND_MODE_E
{
    IPU_BLEND_MODE_NORMAL      = 0x00, // Normal
    IPU_BLEND_MODE_LIGHTEN     = 0x01, // Lighten
    IPU_BLEND_MODE_DARKEN      = 0x02, // Darken
    IPU_BLEND_MODE_MULTIPLY    = 0x03, // Multiply
    IPU_BLEND_MODE_AVERAGE     = 0x04, // Average
    IPU_BLEND_MODE_DIFFERENCE  = 0x05, // Difference
    IPU_BLEND_MODE_NEGATION    = 0x06, // Negation
    IPU_BLEND_MODE_SCREEN      = 0x07, // Screen
    IPU_BLEND_MODE_EXCLUSION   = 0x08, // Exclusion
    IPU_BLEND_MODE_OVERLAY     = 0x09, // Overlay
    IPU_BLEND_MODE_SOFTLIGHT   = 0x0A, // Soft light
    IPU_BLEND_MODE_HARDLIGHT   = 0x0B, // Hard light
    IPU_BLEND_MODE_LINEARDODGE = 0x0C, // Linear dodge
    IPU_BLEND_MODE_LINEARBURN  = 0x0D, // Linear burn
    IPU_BLEND_MODE_LINEARLIGHT = 0x0E, // Linear light
    IPU_BLEND_MODE_PINLIGHT    = 0x0F, // Pin light
}IPU_BLEND_MODE_E;

typedef enum akIPU_COLOR_FMT_E
{
    IPU_COLOR_FMT_FULLPLANE_GREY  = 0X01, // Planar grey (separate planes)
    IPU_COLOR_FMT_FULLPLANE_RGB   = 0X02,
    IPU_COLOR_FMT_FULLPLANE_YV24  = 0X03,
    IPU_COLOR_FMT_FULLPLANE_YV16  = 0X04,
    IPU_COLOR_FMT_FULLPLANE_YV12  = 0X05, //YUV420P

    IPU_COLOR_FMT_SEMIPLANE_NV24  = 0X10, // Y plane separate, UV interleaved
    IPU_COLOR_FMT_SEMIPLANE_NV42  = 0X11,
    IPU_COLOR_FMT_SEMIPLANE_NV16  = 0X12,
    IPU_COLOR_FMT_SEMIPLANE_NV61  = 0X13,
    IPU_COLOR_FMT_SEMIPLANE_NV12  = 0X14, //YUV420SP
    IPU_COLOR_FMT_SEMIPLANE_NV21  = 0X15,

    IPU_COLOR_FMT_PACKED_ARGB32   = 0X20, // Packed ARGB (all components interleaved)
    IPU_COLOR_FMT_PACKED_ABGR32   = 0X21,
    IPU_COLOR_FMT_PACKED_BGRA32   = 0X22,
    IPU_COLOR_FMT_PACKED_RGBA32   = 0X23,
    IPU_COLOR_FMT_PACKED_RGB24    = 0X24,
    IPU_COLOR_FMT_PACKED_BGR24    = 0X25,
    IPU_COLOR_FMT_PACKED_RGB565   = 0X26,
    IPU_COLOR_FMT_PACKED_BGR565   = 0X27,
    IPU_COLOR_FMT_PACKED_RGB555   = 0X28,
    IPU_COLOR_FMT_PACKED_BGR555   = 0X29,

    IPU_COLOR_FMT_PACKED_AYUV     = 0X30,
    IPU_COLOR_FMT_PACKED_AVUY     = 0X31,
    IPU_COLOR_FMT_PACKED_VUYA     = 0X32,
    IPU_COLOR_FMT_PACKED_YUVA     = 0X33,
    IPU_COLOR_FMT_PACKED_YUV      = 0X34,
    IPU_COLOR_FMT_PACKED_VUY      = 0X35,
    IPU_COLOR_FMT_PACKED_IYU2     = 0X36,
    IPU_COLOR_FMT_PACKED_YUYV     = 0X37,
    IPU_COLOR_FMT_PACKED_YVYU     = 0X38,
    IPU_COLOR_FMT_PACKED_UYVY     = 0X39,
    IPU_COLOR_FMT_PACKED_VYUY     = 0X3A,

    IPU_COLOR_FMT_INDEX_1BIT      = 0X60, // input
    IPU_COLOR_FMT_INDEX_2BITS     = 0X61, // input
    IPU_COLOR_FMT_INDEX_4BITS     = 0X62, // input
    IPU_COLOR_FMT_INDEX_8BITS     = 0X63, // input
}IPU_COLOR_FMT_E;

typedef enum akIPU_COLOR_TYPE_E
{
    IPU_COLOR_TYPE_UNKNOWN     = 0,
    IPU_COLOR_TYPE_RGB         = 1,
    IPU_COLOR_TYPE_YUV601_PC   = 2,
    IPU_COLOR_TYPE_YUV601_TV   = 3,
    IPU_COLOR_TYPE_YUV709_PC   = 4,
    IPU_COLOR_TYPE_YUV709_TV   = 5,
    IPU_COLOR_TYPE_YUV2020_PC  = 6,
    IPU_COLOR_TYPE_YUV2020_TV  = 7,
}IPU_COLOR_TYPE_E;

typedef enum akIPU_DITHER_E
{
    IPU_DITHER_NONE   = 0,
    IPU_DITHER_BLUE16 = 1,

}IPU_DITHER_E;

typedef enum akIPU_ALPHA_CHN_E      // Alpha channel selection for image
{
    IPU_ALPHA_CHN_NONE         = 0, // No alpha or not used; all pixels alpha = 255
    IPU_ALPHA_CHN_PACKED       = 1, // Use Alpha in Packed ARGB or palette; else alpha = 255
    IPU_ALPHA_CHN_TRANS_IDX    = 2, // Use transparent index as mask: that index alpha=0, others 255; Indexed only
    IPU_ALPHA_CHN_PLANE_1BIT   = 4, // Use Plane 3 as 1-bit mask
    IPU_ALPHA_CHN_PLANE_2BIT   = 5, // Use Plane 3 as 2-bit mask
    IPU_ALPHA_CHN_PLANE_4BIT   = 6, // Use Plane 3 as 4-bit mask
    IPU_ALPHA_CHN_PLANE_8BIT   = 7, // Use Plane 3 as 8-bit mask

}IPU_ALPHA_CHN_E;

typedef enum akIPU_DIRECTION_E     // Image rotation/flip for preprocessing
{
    IPU_DIRECTION_UNCHANGE    = 0,  // No change
    IPU_DIRECTION_FLIP_VER    = 1,  // Vertical flip
    IPU_DIRECTION_FLIP_HOR    = 2,  // Horizontal flip (mirror)
    IPU_DIRECTION_ROTATE_180  = 3,  // Rotate 180 degrees CW
    IPU_DIRECTION_ROTATE_90   = 4,  // Rotate 90 degrees CW
    IPU_DIRECTION_ROTATE_270  = 5,  // Rotate 270 degrees CW

}IPU_DIRECTION_E;

typedef struct akIPU_RECT_S  // Rectangle for region of interest
{
    AK_S32 s32X;    // X coordinate
    AK_S32 s32Y;    // Y coordinate
    AK_U32 u32W;    // Width
    AK_U32 u32H;    // Height
}IPU_RECT_S;

typedef struct akIPU_DMA_S                      // IPU DMA buffer descriptor
{
    AK_U64 u64PhyAddr;                          // Physical address
    AK_VOID *pvAddr;                            // Virtual address
    AK_U32 u32Len;                              // Length
    AK_U32 u32PhyAddrLow;                       // Physical address low 32 bits (distortion correction, optional)
    AK_U8 u8PhyAddrHigh;                        // Physical address high 8 bits (distortion correction, optional)
    AK_U32 u32W;                                // Layer width (optional)
    AK_U32 u32H;                                // Layer height (optional)
    IPU_COLOR_FMT_E enColorFmt;                 // Layer pixel format (optional)
    AK_U32 u32PlantArrange;                     // YUV/BGR plane arrangement (optional)
    IPU_DIRECTION_E enDirection;                // Source rotation/flip (optional)
    AK_VOID *pvExt;                             // Extra info (optional)
}IPU_DMA_S;

typedef struct akIPU_CMD_QUEUE_S                // Single command queue management
{
    AK_S32 s32InitFlag;                         // Init flag
    AK_RWLOCK_T rwMutex;                        // Read-write lock
    AK_VOID *pvCmdStart;                        // Command buffer start (virtual)
    AK_VOID *pvCmdOffset;                       // Current write offset
    AK_U32 u32LenCmd;                           // Command buffer length
    IPU_DMA_S stIpuAddrCmd;                     // Command queue DMA descriptor
    IPU_DMA_S stIpuAddrResp;                    // Response queue DMA descriptor
    AK_U16 u16CmdId;                            // Command ID
    //AK_U8 u8ImgId;                              // Image ID
}IPU_CMD_QUEUE_S;

typedef enum akIPU_BLEND_OPT_TYPE_E
{
    IPU_OPT_SOLID_COLOR_BLENDING     = 0x30, // Blend foreground onto solid color background
    IPU_OPT_COLOR_KEY_BLENDING       = 0x31, // Color key (chroma key) blend
    IPU_OPT_SPECIFIED_ALPHA_BLENDING = 0x32, // Specified alpha blend
    IPU_OPT_SOLID_BG_BLENDING        = 0x33, // Foreground onto solid background
    IPU_OPT_SOLID_FG_BLENDING        = 0x34, // Solid foreground onto background
    IPU_OPT_GRADIENT_BLENDING        = 0x35, // Gradient blend onto solid background
}IPU_BLEND_OPT_TYPE_E;

typedef struct akIPU_SOLID_COLOR_BLEND_ATTR_S
{
    IPU_BLEND_MODE_E enBlendMode;
    AK_U8 u8Opacity;
}IPU_SOLID_COLOR_BLEND_ATTR_S;                  // Solid color blend

typedef struct akIPU_COLOR_SCOPE_BLEND_ATTR_S
{
    AK_U8 u8OpacityPositive;
    AK_U8 u8OpacityNegative;
    IPU_LAYER_E enRefLayer;
    AK_U32 u32RefColor;
    AK_U8 u8Differ;
}IPU_COLOR_SCOPE_BLEND_ATTR_S;                  // Color scope blend

typedef struct akIPU_SPECI_ALPHA_BLEND_ATTR_S
{
    AK_U8 u8gAlphaBg;
    AK_U8 u8gAlphaFg;
}IPU_SPECI_ALPHA_BLEND_ATTR_S;                 // Specified alpha blend

typedef struct akIPU_SOLID_BG_BLEND_ATTR_S
{
    IPU_BLEND_MODE_E enBlendMode;
    AK_U32 u32ColorBG;
    AK_U8 u8Opacity;
}IPU_SOLID_BG_BLEND_ATTR_S;                    // Solid background blend

typedef struct akIPU_SOLID_FG_BLEND_ATTR_S
{
    IPU_BLEND_MODE_E enBlendMode;
    AK_U32 u32ColorFG;
    AK_U8 u8Opacity;
}IPU_SOLID_FG_BLEND_ATTR_S;                    // Solid foreground blend

typedef struct akIPU_GRADIENT_BLEND_ATTR_S
{
    IPU_BLEND_MODE_E enBlendMode;
    AK_U8 au8OpacityPoints[4];// Alpha at 4 corners for gradient
}IPU_GRADIENT_BLEND_ATTR_S;

typedef struct akIPU_BLEND_OPT_S
{
    IPU_BLEND_OPT_TYPE_E enOpt;
    union
    {
        IPU_SOLID_COLOR_BLEND_ATTR_S stSolidColorBlendAttr; // Solid color blend
        IPU_COLOR_SCOPE_BLEND_ATTR_S stColorScopeBlendAttr; // Color scope blend
        IPU_SPECI_ALPHA_BLEND_ATTR_S stSpeciAlphaBlendAttr;  // Specified alpha blend
        IPU_SOLID_BG_BLEND_ATTR_S stSolidBgBlendAttr;       // Solid BG blend
        IPU_SOLID_FG_BLEND_ATTR_S stSolidFgBlendAttr;       // Solid FG blend
        IPU_GRADIENT_BLEND_ATTR_S stGradientBlendAttr;      // Gradient blend
    };
}IPU_BLEND_OPT_S;

typedef struct akIPU_PLANE_S
{
    AK_U64 u64PhyAddr;   /* Base address of plane. 0 if plane unused.
                           For vertically flipped layout (e.g. BMP), point to last line start. */
    AK_U8  u8PixStride;  /* Byte stride between same-row pixels (1-4). 0 if plane unused. */
    AK_U32 u32PitchLine; /* Line pitch (signed), range [-0x80000, 0x80000). Negative for flipped. 0 if unused. */
}IPU_PLANE_S;

#if 0
typedef struct akIPU_TRANS_BLEND_PICS_ATTR_S
{
    AK_VOID *pvConfData;
    AK_U32 u32ConfLen;
}IPU_TRANS_BLEND_PICS_ATTR_S;
#endif

typedef struct akIPU_SURFACE_S               // Input/output image layer info
{
    AK_U32 u32Width;                         // Width and height
    AK_U32 u32Height;
    IPU_COLOR_FMT_E enColorFmt;              // Pixel format; conversion done if in/out differ
    IPU_COLOR_TYPE_E enColorTypeImage;       // Source color standard
    IPU_COLOR_TYPE_E enColorTypeWork;        // Cache color standard
    IPU_ALPHA_CHN_E enAlphaChnChoice;        // Alpha channel selection
    IPU_DIRECTION_E enDirection;             // Rotation/flip
    AK_U8 u8ColorPadding;                    // Out-of-bounds: 0x0=edge, 0x1=black, 0x2=white (set Cache Color)
    AK_U8 u8AlphaChnInv;                     // Invert alpha or plane3 mask
    AK_U8 u8AlphaDither;                     // Dither when plane3 has fewer than 8 bits
    AK_U8 u8AlphaValue;                      //
    AK_U16 u16PaletteOffset;                 // Palette offset for indexed color
    AK_U16 u16PaletteTransp;                 // When alpha chn=2, transparent index in palette
    IPU_PLANE_S stPlant[4];                  // plane3 can be mask data

    AK_U32 u32Reserve1;                      // Reserved
    AK_U32 u32Reserve2;                      // Reserved
}IPU_SURFACE_S;


/*
 * AK_MPI_IPU_Open - open ipu module.
 * @pstIpuAttr[IN]:  IPU attributes
 * return: 0 success, others error code.
 */
AK_S32 AK_MPI_IPU_Open(const IPU_ATTR_S *pstIpuAttr);

/*
 * AK_MPI_IPU_Open - close ipu module.
 * return: 0 success, others error code.
 */
AK_S32 AK_MPI_IPU_Close(AK_VOID);


/*
 * AK_MPI_IPU_StartQue - start queue for ipu process.
 * @s32QueId[IN]:  queue id.
 * return: 0 success, others error code.
 */
AK_S32 AK_MPI_IPU_StartQue( AK_S32 s32QueId );

/*
 * AK_MPI_IPU_EndQue - close queue for ipu process.
 * @s32QueId[IN]:  queue id.
 * return: 0 success, others error code.
 */
AK_S32 AK_MPI_IPU_EndQue( AK_S32 s32QueId, AK_BOOL bSync, AK_BOOL bBlock );

/*
 * AK_MPI_IPU_WaitForDone - close queue for ipu process.
 * @s32QueId[IN]:  queue id.
 * return: 0 success, others error code.
 */
AK_S32 AK_MPI_IPU_WaitForDone(AK_S32 s32QueId, AK_U32 u32Timeout);

/*
 * AK_MPI_IPU_QuickCopy - copy src layer to dst layer.
 * @s32QueId[IN]:  queue id.
 * @pstIpuSurfaceSrc[IN]:  source surface.
 * @pstIpuRectSrc[IN]:  source rect.
 * @pstIpuSurfaceDst[IN]:  destination surface.
 * @pstIpuRectDst[IN]:  destination rect.
 * return: 0 success, others error code.
 */
AK_S32 AK_MPI_IPU_QuickCopy( AK_S32 s32QueId, IPU_SURFACE_S *pstIpuSurfaceSrc, IPU_RECT_S *pstIpuRectSrc,
                                              IPU_SURFACE_S *pstIpuSurfaceDst, IPU_RECT_S *pstIpuRectDst );
/*
 * AK_MPI_IPU_QuickFill - use a color to fill rect of a surface.
 * @s32QueId[IN]:  queue id.
 * @pstDstImg[IN]:  destination surface.
 * @pstDstRect[IN]:  destination rect.
 * @u32FillColor[IN]:  color
 * return: 0 success, others error code.
 */
AK_S32 AK_MPI_IPU_QuickFill(AK_S32 s32QueId, IPU_SURFACE_S *pstDstImg, IPU_RECT_S *pstDstRect, AK_U32 u32FillColor);

/*
 * AK_MPI_IPU_CompositeResize - resize rect of source surface to destination surface.
 * @s32QueId[IN]:  queue id.
 * @pstSrcImg[IN]:  source surface.
 * @pstSrcRect[IN]: source rect.
 * @pstDstImg[IN]:  destination surface.
 * @pstDstRect[IN]:  destination rect.
 * return: 0 success, others error code.
 */
AK_S32 AK_MPI_IPU_CompositeResize(AK_S32 s32QueId, IPU_SURFACE_S *pstSrcImg, IPU_RECT_S *pstSrcRect,
                                  IPU_SURFACE_S *pstDstImg, IPU_RECT_S *pstDstRect );
/*
 * AK_MPI_IPU_Blend - blend opt.
 * @s32QueId[IN]:  queue id.
 * @pstBackGround[IN]: background surface.
 * @pstBackGroundRect[IN]: background rect
 * @pstForeGround[IN]:  foreground surface.
 * @pstForeGroundRect[IN]: foreground rect.
 * @pstDst[IN]: output surface.
 * @pstDstRect[IN]: output rect.
 * @pstBlendOpt[IN]: blend option.
 * return: 0 success, others error code.
 */
AK_S32 AK_MPI_IPU_Blend( AK_S32 s32QueId, IPU_SURFACE_S *pstBackGround, IPU_RECT_S *pstBackGroundRect,
                                          IPU_SURFACE_S *pstForeGround, IPU_RECT_S *pstForeGroundRect,
                                          IPU_SURFACE_S *pstDst,        IPU_RECT_S *pstDstRect,
                                          IPU_BLEND_OPT_S *pstBlendOpt );

/*
 * AK_MPI_IPU_Trans4 - trans4 cmd.
 * @s32QueId[IN]:  queue id.
 * @pstSrcImg[IN]:  source surface.
 * @pstDstImg[IN]: destination surface.
 * @pstIpuDmaBranch[IN]:  branch trans4 cmd
  * return: 0 success, others error code.
 */
AK_S32 AK_MPI_IPU_Trans4( AK_S32 s32QueId, IPU_SURFACE_S *pstSrcImg, IPU_SURFACE_S *pstDstImg,
                          IPU_DMA_S *pstIpuDmaBranch );

/*
 * AK_MPI_IPU_GetVersion - get version
  * return: 0 success, others error code.
 */
const AK_CHAR* AK_MPI_IPU_GetVersion(void);

/*
 * AK_MPI_IPU_GetCmdLen - get cmd len
 * return: 0 success, others error code.
 */
AK_S32 AK_MPI_IPU_GetCmdLen( AK_U8 cmd );

/*
 * AK_MPI_IPU_Reset - Reset IPU .
 * return: 0 success, others error code.
 */
AK_S32 AK_MPI_IPU_Reset( );

/*
 * AK_MPI_IPU_SetColorTable - set global palette color table.
 * @u8Num[IN]:  color number
 * @u8Offset[IN]:  color array offset
 * @pu32Argb[IN]: set color value array.
  * return: 0 success, others error code.
 */
AK_S32 AK_MPI_IPU_SetColorTable( AK_U8 u8Num, AK_U8 u8Offset, AK_U32 *pu32Argb );
#endif