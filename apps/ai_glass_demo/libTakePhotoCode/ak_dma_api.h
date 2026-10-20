/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd. */
#ifndef __AK_DMA_API_H__
#define __AK_DMA_API_H__

#include "ak_common.h"
#include "ak_thread.h"
#include "ak_ipu.h"


/**
 * @brief Prints IPU DMA structure information.
 *
 * @param pIpuDma Pointer to the IPU DMA structure.
 * @return AK_VOID
 */
AK_VOID print_dma(IPU_DMA_S *pIpuDma);

/**
 * @brief Initializes an IPU surface.
 *
 * @param pstIpuDma Pointer to the IPU DMA structure.
 * @param pstSurface Pointer to the IPU surface to initialize.
 * @return AK_S32 AK_SUCCESS on success, error code on failure.
 */
AK_S32 init_surface(IPU_DMA_S *pstIpuDma, IPU_SURFACE_S *pstSurface);

/**
 * @brief Initializes the DMA image.
 *
 * @param pstIpuDma Pointer to the IPU DMA structure.
 * @param s8LoadFile Flag indicating whether to load a file (1 for yes, 0 for no).
 * @param s8NeedMalloc Flag indicating whether memory allocation is needed (1 for yes, 0 for no).
 * @return AK_S32 AK_SUCCESS on success, error code on failure.
 */
AK_S32 init_dma_img(IPU_DMA_S *pstIpuDma, AK_S8 s8LoadFile, AK_S8 s8NeedMalloc);

/**
 * @brief Frees IPU DMA resources.
 *
 * @param pstIpuDma Pointer to the IPU DMA structure.
 * @param u8FreeExt Flag indicating whether to free external resources (1 for yes, 0 for no).
 * @return AK_S32 AK_SUCCESS on success, error code on failure.
 */
AK_S32 IPU_Dma_Free(IPU_DMA_S *pstIpuDma, AK_U8 u8FreeExt);

/**
 * @brief Allocates memory for IPU DMA.
 *
 * @param pstIpuDma Pointer to the IPU DMA structure.
 * @param u32Size Size of memory to allocate in bytes.
 * @return AK_S32 AK_SUCCESS on success, error code on failure.
 */
AK_S32 IPU_Dma_Alloc(IPU_DMA_S *pstIpuDma, AK_U32 u32Size);

/**
 * @brief Loads an image file into IPU DMA memory.
 *
 * @param pstIpuDma Pointer to the IPU DMA structure.
 * @param pcFile Path to the image file.
 * @return AK_S32 AK_SUCCESS on success, error code on failure.
 */
AK_S32 IPU_Dma_Load(IPU_DMA_S *pstIpuDma, AK_CHAR *pcFile);

#endif