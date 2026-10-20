/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd. */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "ak_common.h"
#include "ak_log.h"
#include "ak_common_video.h"
#include "ak_mem.h"
#include "ak_thread.h"
#include "ak_ipu.h"


#include "ak_dma_api.h"

AK_VOID print_dma( IPU_DMA_S *pIpuDma )
{
    AK_BASE_PrintNormalEx(MODULE_ID_APP,
            "pIpuDma= %p => u64PhyAddr= %lx pvAddr= %p u32Len= %u "
            "u32W= %u u32H= %u enColorFmt= %d u32PlantArrange= %d enDirection= %d pvExt= '%s'\n",
            pIpuDma,
            pIpuDma->u64PhyAddr,
            pIpuDma->pvAddr,
            pIpuDma->u32Len,
            pIpuDma->u32W,
            pIpuDma->u32H,
            pIpuDma->enColorFmt,
            pIpuDma->u32PlantArrange,
            pIpuDma->enDirection,
            ( char * )pIpuDma->pvExt );
}

AK_S32 IPU_Dma_Free( IPU_DMA_S *pstIpuDma, AK_U8 u8FreeExt )
{
    if( pstIpuDma != NULL ) {
        if( ( pstIpuDma->u32Len > 0 ) && ( pstIpuDma->pvAddr != NULL ) ) {
            AK_BASE_MemDmaFree( pstIpuDma->pvAddr );
            pstIpuDma->u64PhyAddr = 0;
            pstIpuDma->u32Len = 0;
            pstIpuDma->pvAddr = NULL;
        }
        if ( ( u8FreeExt == AK_TRUE ) && ( pstIpuDma->pvExt != NULL ) ){
            free( pstIpuDma->pvExt );
        }

        return AK_SUCCESS;
    }
    else {
        return AK_FAILED;
    }
}

AK_S32 init_surface( IPU_DMA_S *pstIpuDma, IPU_SURFACE_S *pstSurface )
{
    if ( ( pstIpuDma != NULL ) && ( pstSurface != NULL ) ) { //Init surface struct based on DMA params
        memset( pstSurface, 0, sizeof( IPU_SURFACE_S ) );
        pstSurface->u32Width = pstIpuDma->u32W;
        pstSurface->u32Height = pstIpuDma->u32H;
        pstSurface->enColorFmt = pstIpuDma->enColorFmt;
        AK_BASE_PrintNormalEx(MODULE_ID_APP,
                "pstSurface->u32Width= %u pstSurface->u32Height= %u pstSurface->enColorFmt= %d(0x%0x)\n",
                pstSurface->u32Width, pstSurface->u32Height, pstSurface->enColorFmt, pstSurface->enColorFmt );

        pstSurface->enDirection = pstIpuDma->enDirection;
        switch( pstIpuDma->enColorFmt ) {
            case IPU_COLOR_FMT_FULLPLANE_RGB:
                switch( pstIpuDma->u32PlantArrange ) {
                    case AK_FALSE: //bgr format
                        pstSurface->stPlant[ 2 ].u64PhyAddr = pstIpuDma->u64PhyAddr;
                        pstSurface->stPlant[ 2 ].u8PixStride = 1;
                        pstSurface->stPlant[ 2 ].u32PitchLine = pstIpuDma->u32W;

                        pstSurface->stPlant[ 1 ].u64PhyAddr = pstIpuDma->u64PhyAddr + pstIpuDma->u32W * pstIpuDma->u32H;
                        pstSurface->stPlant[ 1 ].u8PixStride = 1;
                        pstSurface->stPlant[ 1 ].u32PitchLine = pstIpuDma->u32W;

                        pstSurface->stPlant[ 0 ].u64PhyAddr = pstIpuDma->u64PhyAddr + pstIpuDma->u32W * pstIpuDma->u32H * 2;
                        pstSurface->stPlant[ 0 ].u8PixStride = 1;
                        pstSurface->stPlant[ 0 ].u32PitchLine = pstIpuDma->u32W;
                        break;
                    case AK_TRUE:
                    default: //rgb format
                        pstSurface->stPlant[ 0 ].u64PhyAddr = pstIpuDma->u64PhyAddr;
                        pstSurface->stPlant[ 0 ].u8PixStride = 1;
                        pstSurface->stPlant[ 0 ].u32PitchLine = pstIpuDma->u32W;

                        pstSurface->stPlant[ 1 ].u64PhyAddr = pstIpuDma->u64PhyAddr + pstIpuDma->u32W * pstIpuDma->u32H;
                        pstSurface->stPlant[ 1 ].u8PixStride = 1;
                        pstSurface->stPlant[ 1 ].u32PitchLine = pstIpuDma->u32W;

                        pstSurface->stPlant[ 2 ].u64PhyAddr = pstIpuDma->u64PhyAddr + pstIpuDma->u32W * pstIpuDma->u32H * 2;
                        pstSurface->stPlant[ 2 ].u8PixStride = 1;
                        pstSurface->stPlant[ 2 ].u32PitchLine = pstIpuDma->u32W;
                        break;
                }
                break;
            case IPU_COLOR_FMT_FULLPLANE_YV24:
                switch( pstIpuDma->u32PlantArrange ) {
                    case AK_FALSE: //yu24 format
                        pstSurface->stPlant[ 0 ].u64PhyAddr = pstIpuDma->u64PhyAddr;
                        pstSurface->stPlant[ 0 ].u8PixStride = 1;
                        pstSurface->stPlant[ 0 ].u32PitchLine = pstIpuDma->u32W;

                        pstSurface->stPlant[ 2 ].u64PhyAddr = pstIpuDma->u64PhyAddr + pstIpuDma->u32W * pstIpuDma->u32H;
                        pstSurface->stPlant[ 2 ].u8PixStride = 1;
                        pstSurface->stPlant[ 2 ].u32PitchLine = pstIpuDma->u32W;

                        pstSurface->stPlant[ 1 ].u64PhyAddr = pstIpuDma->u64PhyAddr + pstIpuDma->u32W * pstIpuDma->u32H * 2;
                        pstSurface->stPlant[ 1 ].u8PixStride = 1;
                        pstSurface->stPlant[ 1 ].u32PitchLine = pstIpuDma->u32W;
                        break;
                    case AK_TRUE:
                    default: //YV24 format
                        pstSurface->stPlant[ 0 ].u64PhyAddr = pstIpuDma->u64PhyAddr;
                        pstSurface->stPlant[ 0 ].u8PixStride = 1;
                        pstSurface->stPlant[ 0 ].u32PitchLine = pstIpuDma->u32W;

                        pstSurface->stPlant[ 1 ].u64PhyAddr = pstIpuDma->u64PhyAddr + pstIpuDma->u32W * pstIpuDma->u32H;
                        pstSurface->stPlant[ 1 ].u8PixStride = 1;
                        pstSurface->stPlant[ 1 ].u32PitchLine = pstIpuDma->u32W;

                        pstSurface->stPlant[ 2 ].u64PhyAddr = pstIpuDma->u64PhyAddr + pstIpuDma->u32W * pstIpuDma->u32H * 2;
                        pstSurface->stPlant[ 2 ].u8PixStride = 1;
                        pstSurface->stPlant[ 2 ].u32PitchLine = pstIpuDma->u32W;
                        break;
                }
                break;
            case IPU_COLOR_FMT_FULLPLANE_YV16:
                switch( pstIpuDma->u32PlantArrange ) {
                    case AK_FALSE: //yu16 format
                        pstSurface->stPlant[ 0 ].u64PhyAddr = pstIpuDma->u64PhyAddr;
                        pstSurface->stPlant[ 0 ].u8PixStride = 1;
                        pstSurface->stPlant[ 0 ].u32PitchLine = pstIpuDma->u32W;

                        pstSurface->stPlant[ 2 ].u64PhyAddr = pstIpuDma->u64PhyAddr + pstIpuDma->u32W * pstIpuDma->u32H;
                        pstSurface->stPlant[ 2 ].u8PixStride = 1;
                        pstSurface->stPlant[ 2 ].u32PitchLine = pstIpuDma->u32W / 2;

                        pstSurface->stPlant[ 1 ].u64PhyAddr = pstIpuDma->u64PhyAddr + pstIpuDma->u32W * pstIpuDma->u32H +
                                                              pstIpuDma->u32W * ( pstIpuDma->u32H >> 1 );
                        pstSurface->stPlant[ 1 ].u8PixStride = 1;
                        pstSurface->stPlant[ 1 ].u32PitchLine = pstIpuDma->u32W / 2;
                        break;
                    case AK_TRUE: //yv16 format
                    default:
                        pstSurface->stPlant[ 0 ].u64PhyAddr = pstIpuDma->u64PhyAddr;
                        pstSurface->stPlant[ 0 ].u8PixStride = 1;
                        pstSurface->stPlant[ 0 ].u32PitchLine = pstIpuDma->u32W;

                        pstSurface->stPlant[ 1 ].u64PhyAddr = pstIpuDma->u64PhyAddr + pstIpuDma->u32W * pstIpuDma->u32H;
                        pstSurface->stPlant[ 1 ].u8PixStride = 1;
                        pstSurface->stPlant[ 1 ].u32PitchLine = pstIpuDma->u32W / 2;

                        pstSurface->stPlant[ 2 ].u64PhyAddr = pstIpuDma->u64PhyAddr + pstIpuDma->u32W * pstIpuDma->u32H +
                                                              pstIpuDma->u32W * ( pstIpuDma->u32H >> 1 );
                        pstSurface->stPlant[ 2 ].u8PixStride = 1;
                        pstSurface->stPlant[ 2 ].u32PitchLine = pstIpuDma->u32W / 2;
                        break;
                }
                break;
            case IPU_COLOR_FMT_FULLPLANE_YV12:
                switch( pstIpuDma->u32PlantArrange ) {
                    case AK_FALSE: //yu12
                        pstSurface->stPlant[ 0 ].u64PhyAddr = pstIpuDma->u64PhyAddr;
                        pstSurface->stPlant[ 0 ].u8PixStride = 1;
                        pstSurface->stPlant[ 0 ].u32PitchLine = pstIpuDma->u32W;

                        pstSurface->stPlant[ 2 ].u64PhyAddr = pstIpuDma->u64PhyAddr + pstIpuDma->u32W * pstIpuDma->u32H;
                        pstSurface->stPlant[ 2 ].u8PixStride = 1;
                        pstSurface->stPlant[ 2 ].u32PitchLine = pstIpuDma->u32W / 2;

                        pstSurface->stPlant[ 1 ].u64PhyAddr = pstIpuDma->u64PhyAddr + pstIpuDma->u32W * pstIpuDma->u32H +
                                                              ( pstIpuDma->u32W >> 1 ) * ( pstIpuDma->u32H >> 1 );
                        pstSurface->stPlant[ 1 ].u8PixStride = 1;
                        pstSurface->stPlant[ 1 ].u32PitchLine = pstIpuDma->u32W / 2;
                        break;
                    case AK_TRUE: //yv12
                    default:
                        pstSurface->stPlant[ 0 ].u64PhyAddr = pstIpuDma->u64PhyAddr;
                        pstSurface->stPlant[ 0 ].u8PixStride = 1;
                        pstSurface->stPlant[ 0 ].u32PitchLine = pstIpuDma->u32W;

                        pstSurface->stPlant[ 1 ].u64PhyAddr = pstIpuDma->u64PhyAddr + pstIpuDma->u32W * pstIpuDma->u32H;
                        pstSurface->stPlant[ 1 ].u8PixStride = 1;
                        pstSurface->stPlant[ 1 ].u32PitchLine = pstIpuDma->u32W / 2;

                        pstSurface->stPlant[ 2 ].u64PhyAddr = pstIpuDma->u64PhyAddr + pstIpuDma->u32W * pstIpuDma->u32H +
                                                              ( pstIpuDma->u32W >> 1 ) * ( pstIpuDma->u32H >> 1 );
                        pstSurface->stPlant[ 2 ].u8PixStride = 1;
                        pstSurface->stPlant[ 2 ].u32PitchLine = pstIpuDma->u32W / 2;
                        break;
                }
                break;
            case IPU_COLOR_FMT_SEMIPLANE_NV24:
            case IPU_COLOR_FMT_SEMIPLANE_NV42: //nv24 and nv42 need to swap y and uv components
                pstSurface->stPlant[ 1 ].u64PhyAddr = pstIpuDma->u64PhyAddr;
                pstSurface->stPlant[ 1 ].u8PixStride = 1;
                pstSurface->stPlant[ 1 ].u32PitchLine = pstIpuDma->u32W;

                pstSurface->stPlant[ 0 ].u64PhyAddr = pstIpuDma->u64PhyAddr + pstIpuDma->u32W * pstIpuDma->u32H;
                pstSurface->stPlant[ 0 ].u8PixStride = 2;
                pstSurface->stPlant[ 0 ].u32PitchLine = pstIpuDma->u32W * 2;
                break;
            case IPU_COLOR_FMT_SEMIPLANE_NV16:
            case IPU_COLOR_FMT_SEMIPLANE_NV61:
                pstSurface->stPlant[ 0 ].u64PhyAddr = pstIpuDma->u64PhyAddr;
                pstSurface->stPlant[ 0 ].u8PixStride = 1;
                pstSurface->stPlant[ 0 ].u32PitchLine = pstIpuDma->u32W;

                pstSurface->stPlant[ 1 ].u64PhyAddr = pstIpuDma->u64PhyAddr + pstIpuDma->u32W * pstIpuDma->u32H;
                pstSurface->stPlant[ 1 ].u8PixStride = 2;
                pstSurface->stPlant[ 1 ].u32PitchLine = pstIpuDma->u32W;
                break;
            case IPU_COLOR_FMT_SEMIPLANE_NV12:
            case IPU_COLOR_FMT_SEMIPLANE_NV21:
                pstSurface->stPlant[ 0 ].u64PhyAddr = pstIpuDma->u64PhyAddr;
                pstSurface->stPlant[ 0 ].u8PixStride = 1;
                pstSurface->stPlant[ 0 ].u32PitchLine = pstIpuDma->u32W;

                pstSurface->stPlant[ 1 ].u64PhyAddr = pstIpuDma->u64PhyAddr + pstIpuDma->u32W * pstIpuDma->u32H;
                pstSurface->stPlant[ 1 ].u8PixStride = 2;
                pstSurface->stPlant[ 1 ].u32PitchLine = pstIpuDma->u32W;
                break;
            case IPU_COLOR_FMT_PACKED_ARGB32:
            case IPU_COLOR_FMT_PACKED_ABGR32:
            case IPU_COLOR_FMT_PACKED_BGRA32:
            case IPU_COLOR_FMT_PACKED_RGBA32:
            case IPU_COLOR_FMT_PACKED_AYUV:
            case IPU_COLOR_FMT_PACKED_AVUY:
            case IPU_COLOR_FMT_PACKED_VUYA:
            case IPU_COLOR_FMT_PACKED_YUVA:
                pstSurface->stPlant[ 0 ].u64PhyAddr = pstIpuDma->u64PhyAddr;
                pstSurface->stPlant[ 0 ].u8PixStride = 4;
                pstSurface->stPlant[ 0 ].u32PitchLine = pstIpuDma->u32W * 4;
                break;
            case IPU_COLOR_FMT_PACKED_RGB24:
            case IPU_COLOR_FMT_PACKED_BGR24: //rgb bgr formats
            case IPU_COLOR_FMT_PACKED_YUV:
            case IPU_COLOR_FMT_PACKED_VUY:
            case IPU_COLOR_FMT_PACKED_IYU2:
                pstSurface->stPlant[ 0 ].u64PhyAddr = pstIpuDma->u64PhyAddr;
                pstSurface->stPlant[ 0 ].u8PixStride = 3;
                pstSurface->stPlant[ 0 ].u32PitchLine = pstIpuDma->u32W * 3;
                break;
            case IPU_COLOR_FMT_PACKED_RGB565:
            case IPU_COLOR_FMT_PACKED_BGR565:
            case IPU_COLOR_FMT_PACKED_RGB555:
            case IPU_COLOR_FMT_PACKED_BGR555:
                pstSurface->stPlant[ 0 ].u64PhyAddr = pstIpuDma->u64PhyAddr;
                pstSurface->stPlant[ 0 ].u8PixStride = 2;
                pstSurface->stPlant[ 0 ].u32PitchLine = pstIpuDma->u32W * 2;
                break;
            case IPU_COLOR_FMT_PACKED_YUYV:
            case IPU_COLOR_FMT_PACKED_YVYU:
            case IPU_COLOR_FMT_PACKED_UYVY:
            case IPU_COLOR_FMT_PACKED_VYUY:
                pstSurface->stPlant[ 0 ].u64PhyAddr = pstIpuDma->u64PhyAddr;
                pstSurface->stPlant[ 0 ].u8PixStride = 4;
                pstSurface->stPlant[ 0 ].u32PitchLine = pstIpuDma->u32W * 2;
                break;
            case IPU_COLOR_FMT_INDEX_1BIT:
                pstSurface->stPlant[ 0 ].u64PhyAddr = pstIpuDma->u64PhyAddr;
                pstSurface->stPlant[ 0 ].u8PixStride = 1;
                pstSurface->stPlant[ 0 ].u32PitchLine = pstIpuDma->u32W / 8;
                break;
            case IPU_COLOR_FMT_INDEX_2BITS:
                pstSurface->stPlant[ 0 ].u64PhyAddr = pstIpuDma->u64PhyAddr;
                pstSurface->stPlant[ 0 ].u8PixStride = 1;
                pstSurface->stPlant[ 0 ].u32PitchLine = pstIpuDma->u32W / 4;
                break;
            case IPU_COLOR_FMT_INDEX_4BITS:
                pstSurface->stPlant[ 0 ].u64PhyAddr = pstIpuDma->u64PhyAddr;
                pstSurface->stPlant[ 0 ].u8PixStride = 1;
                pstSurface->stPlant[ 0 ].u32PitchLine = pstIpuDma->u32W / 2;
                break;
            case IPU_COLOR_FMT_INDEX_8BITS:
            case IPU_COLOR_FMT_FULLPLANE_GREY:
                pstSurface->stPlant[ 0 ].u64PhyAddr = pstIpuDma->u64PhyAddr;
                pstSurface->stPlant[ 0 ].u8PixStride = 1;
                pstSurface->stPlant[ 0 ].u32PitchLine = pstIpuDma->u32W;
                break;
            default:
                AK_BASE_PrintErrorEx( MODULE_ID_APP,
                        "Not support pstIpuDma->enColorFmt= %d\n", pstIpuDma->enColorFmt );
                break;
        }
    }
    return 0;
}



AK_S32 IPU_Dma_Alloc( IPU_DMA_S *pstIpuDma, AK_U32 u32Size )
{
    if( ( pstIpuDma != NULL ) && ( u32Size > 0 ) &&
        ( ( pstIpuDma->pvAddr = AK_BASE_MemDmaAlloc( MODULE_ID_APP, u32Size ) ) != NULL ) &&
        ( AK_BASE_MemDmaVaddr2Paddr( pstIpuDma->pvAddr, &pstIpuDma->u64PhyAddr ) == AK_SUCCESS ) ) {
        pstIpuDma->u32Len = u32Size; // Length
        pstIpuDma->u32PhyAddrLow = pstIpuDma->u64PhyAddr & 0xFFFFFFFF;    // Low address
        pstIpuDma->u8PhyAddrHigh= ( pstIpuDma->u64PhyAddr >> 32 ) & 0XFF; // High address
        return AK_SUCCESS;
    }
    else {
        return AK_FAILED;
    }
}

AK_S32 IPU_Dma_Load( IPU_DMA_S *pstIpuDma, AK_CHAR *pcFile )
{
    AK_S32 s32Read = 0, ret= AK_FAILED;
    FILE *pFILE = NULL;

    if ( pcFile != NULL ) { //If not NULL, read content as filename
        if ( ( ( pFILE = fopen( ( char * )pcFile, "rb") ) != NULL ) &&
            ( ( s32Read = fread( pstIpuDma->pvAddr, 1, pstIpuDma->u32Len, pFILE ) ) == pstIpuDma->u32Len ) ){
            ret = AK_SUCCESS;
        }
        else { //Open failed
            AK_BASE_PrintErrorEx( MODULE_ID_APP,
                    "s32Read= %d != pstIpuDma->u32Len= %u pcFile= '%s' pFILE= %p pstIpuDma->enColorFmt= %d\n",
                    s32Read, pstIpuDma->u32Len, pcFile, pFILE, pstIpuDma->enColorFmt );
        }
        if( pFILE != NULL ) {
            fclose( pFILE );
        }
    }
    else {
        AK_BASE_PrintWarningEx( MODULE_ID_APP,
                "Not Load file: no file name\n" );
    }
    print_dma( pstIpuDma );
    return ret;
}

AK_S32 init_dma_img( IPU_DMA_S *pstIpuDma, AK_S8 s8LoadFile,  AK_S8 s8NeedMalloc)
{
    int ret = AK_FAILED;
    AK_S32 s32Size = 0;

    if( ( pstIpuDma != NULL ) &&
        ( pstIpuDma->u32W * pstIpuDma->u32H > 0 ) &&
        ( pstIpuDma->enColorFmt > 0 ) ){  //If dma struct is not initialized, allocate dma memory
        switch( pstIpuDma->enColorFmt ) { //Create dma memory based on image type and size
            case IPU_COLOR_FMT_PACKED_ARGB32:
            case IPU_COLOR_FMT_PACKED_ABGR32:
            case IPU_COLOR_FMT_PACKED_BGRA32:
            case IPU_COLOR_FMT_PACKED_RGBA32:
            case IPU_COLOR_FMT_PACKED_AYUV:
            case IPU_COLOR_FMT_PACKED_AVUY:
            case IPU_COLOR_FMT_PACKED_VUYA:
            case IPU_COLOR_FMT_PACKED_YUVA:
                s32Size = pstIpuDma->u32W * pstIpuDma->u32H * 4;
                break;
            case IPU_COLOR_FMT_FULLPLANE_RGB:
            case IPU_COLOR_FMT_FULLPLANE_YV24:
            case IPU_COLOR_FMT_SEMIPLANE_NV24:
            case IPU_COLOR_FMT_SEMIPLANE_NV42:
            case IPU_COLOR_FMT_PACKED_RGB24:
            case IPU_COLOR_FMT_PACKED_BGR24:
            case IPU_COLOR_FMT_PACKED_YUV:
            case IPU_COLOR_FMT_PACKED_VUY:
            case IPU_COLOR_FMT_PACKED_IYU2:
                s32Size = pstIpuDma->u32W * pstIpuDma->u32H * 3;
                break;
            case IPU_COLOR_FMT_PACKED_RGB565:
            case IPU_COLOR_FMT_PACKED_BGR565:
            case IPU_COLOR_FMT_PACKED_RGB555:
            case IPU_COLOR_FMT_PACKED_BGR555:
            case IPU_COLOR_FMT_FULLPLANE_YV16:
            case IPU_COLOR_FMT_SEMIPLANE_NV16:
            case IPU_COLOR_FMT_SEMIPLANE_NV61:
            case IPU_COLOR_FMT_PACKED_YUYV:
            case IPU_COLOR_FMT_PACKED_YVYU:
            case IPU_COLOR_FMT_PACKED_UYVY:
            case IPU_COLOR_FMT_PACKED_VYUY:
                s32Size = pstIpuDma->u32W * pstIpuDma->u32H * 2;
                break;
            case IPU_COLOR_FMT_FULLPLANE_YV12:
            case IPU_COLOR_FMT_SEMIPLANE_NV12:
            case IPU_COLOR_FMT_SEMIPLANE_NV21:
                s32Size = pstIpuDma->u32W * pstIpuDma->u32H * 3 / 2;
                break;
            case IPU_COLOR_FMT_INDEX_1BIT:
                s32Size = pstIpuDma->u32W * pstIpuDma->u32H / 8;
                break;
            case IPU_COLOR_FMT_INDEX_2BITS:
                s32Size = pstIpuDma->u32W * pstIpuDma->u32H / 4;
                break;
            case IPU_COLOR_FMT_INDEX_4BITS:
                s32Size = pstIpuDma->u32W * pstIpuDma->u32H / 2;
                break;
            case IPU_COLOR_FMT_INDEX_8BITS:
            case IPU_COLOR_FMT_FULLPLANE_GREY:
                s32Size = pstIpuDma->u32W * pstIpuDma->u32H;
                break;
            default:
                s32Size = 0;
        }
        //AK_BASE_PrintNormalEx(MODULE_ID_APP, "pstIpuDma= %p s32Size= %d\n", pstIpuDma, s32Size );
        if (s32Size > 0)
        {
            if (s8NeedMalloc)
            {
                if (AK_SUCCESS != IPU_Dma_Alloc( pstIpuDma, s32Size ))
                    ret = AK_FAILED;
            }

            if( s8LoadFile == AK_TRUE ) {
                IPU_Dma_Load( pstIpuDma, ( AK_CHAR * )pstIpuDma->pvExt );
            }
            if (s8NeedMalloc)
                print_dma( pstIpuDma );
            ret = AK_SUCCESS;
        }
        else {
            AK_BASE_PrintErrorEx( MODULE_ID_APP,
                    "s32Size= %d pstIpuDma-> pvExt= '%s' u32W= %u u32H= %u enColorFmt= %d\n",
                    s32Size, ( char * )pstIpuDma->pvExt, pstIpuDma->u32W, pstIpuDma->u32H, pstIpuDma->enColorFmt );
            pstIpuDma->u32Len = 0;
            pstIpuDma->pvAddr = NULL;
            pstIpuDma->u64PhyAddr = 0;
            ret = AK_FAILED;
        }
    }
    else {
        if( pstIpuDma != NULL ) {
            AK_BASE_PrintErrorEx( MODULE_ID_APP,
                    "pstIpuDma-> pvExt= '%s' u32W= %u u32H= %u enColorFmt= %d\n",
                    ( char * )pstIpuDma->pvExt, pstIpuDma->u32W, pstIpuDma->u32H, pstIpuDma->enColorFmt );
        }
    }
    return ret;
}
