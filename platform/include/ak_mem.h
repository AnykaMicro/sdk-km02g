/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd. */
#ifndef _AK_MEM_H_
#define _AK_MEM_H_

#include "ak_common.h"

#define LEFT_ALIGN(a,b)  ((a/b)*b)
#define RIGHT_ALIGN(a,b) ((a+b-1)/b*b)

typedef struct akUSED_MEM_INFO_S {
    AK_S32 s32ModuleId;
    AK_CHAR acFunc[20];
    AK_S64 s64Time;
    AK_U32 u32Size;
    AK_VOID* pvAddr;
    struct akUSED_MEM_INFO_S *pstNext;
}USED_MEM_INFO_S;

/**
 * AK_BASE_MemGetVersion: Get mem module version info
 * return: mem module version info
 */
const AK_CHAR* AK_BASE_MemGetVersion(AK_VOID);

/**
 * ak_mem_malloc_: re-wrap malloc function for detecting memory leak
 * @s32ModuleId[IN]: which module apply memory?
 * @size[IN]: what size of the ring buffer is?
 * @func_name[IN]: function entry where you apply ring buffer
 * @s64Time[IN]: what s64Time the ring buffer was applied?
 * return: mem blk address or NULL
 */
AK_VOID * AK_BASE_MemAlloc_(AK_S32 s32ModuleId, AK_U32 u32Size, \
    const AK_CHAR *pvFuncName, AK_S64 s64Time);
#define AK_BASE_MemAlloc(s32ModuleId, u32Size) \
    AK_BASE_MemAlloc_(s32ModuleId, u32Size, __FUNCTION__, AK_BASE_GetOsTimeStamp())

/**
 * AK_BASE_MemFree: re-wrap free function for detecting memory leak
 * @ptr[IN]: malloced pointer
 * return: AK_SUCCESS or error_code
 */
AK_S32 AK_BASE_MemFree(AK_VOID *pvPtr);

/**
 * AK_BASE_MemPoolInit_: init a memory pool
 * @ps32PoolId[OUT]: the memory pool id you applied
 * @s32ModuleId[IN]: which module apply memory pool?
 * @u32Nmemb[IN]: how many memroy blk does the memory pool contains?
 * @u32Size[IN]: what size of the memory blk is?
 * @pcFuncName[IN]: function entry where you apply memory pool
 * @s64Time[IN]: what s64Time the memory pool was applied?
 * return: AK_SUCCESS or error_code
 */
AK_S32 AK_BASE_MemPoolInit_(AK_S32 *ps32PoolId, AK_S32 s32ModuleId, AK_U32 u32Nmemb, \
    AK_U32 u32Size, const AK_CHAR *pcFuncName, AK_S64 s64Time);
#define AK_BASE_MemPoolInit(ps32PoolId, s32ModuleId, u32Nmemb, u32Size) \
    AK_BASE_MemPoolInit_(ps32PoolId, s32ModuleId, u32Nmemb, u32Size, __FUNCTION__,\
    AK_BASE_GetOsTimeStamp())

/**
 * AK_BASE_MemPoolDestroy: destroy a memory pool
 * @s32MemPoolId[IN]: which mem pool you will operate?
 * return: AK_SUCCESS or error_code
 */
AK_S32 AK_BASE_MemPoolDestroy(AK_S32 s32MemPoolId);

/**
 * AK_BASE_MemPoolAlloc: alloc a memory pool blk
 * @p_mem_blk[OUT]: memory pool blk pointer
 * @s32MemPoolId[IN]: which mem pool you will use?
 * return: AK_SUCCESS or error_code
 */
AK_S32 AK_BASE_MemPoolAlloc(AK_VOID **ppMemBlk, AK_S32 s32MemPoolId);

/**
 * AK_BASE_MemPoolFree: free a memory pool blk
 * @ptr[IN]: memory pool blk pointer
 * @s32MemPoolId[IN]: which mem pool you will use?
 * return: AK_SUCCESS or error_code
 */
AK_S32 AK_BASE_MemPoolFree(AK_VOID *pvPtr, AK_S32 s32MemPoolId);

/**
 * AK_BASE_MemRbInit_: init a ring buffer
 * @ps32RbId[OUT]: ring buffer id
 * @s32ModuleId[IN]: which module apply memory?
 * @u32Size[IN]: what size of the ring buffer is?
 * @pcFuncName[IN]: function entry where you apply ring buffer
 * @s64Time[IN]: what s64Time the ring buffer was applied?
 * return: AK_SUCCESS or error_code
 */
AK_S32 AK_BASE_MemRbInit_(AK_S32 *ps32RbId, AK_S32 s32ModuleId, AK_U32 u32Size, \
    const AK_CHAR *pcFuncName, AK_S64 s64Time);
#define AK_BASE_MemRbInit(ps32RbId, s32ModuleId, u32Size) \
    AK_BASE_MemRbInit_(ps32RbId, s32ModuleId, u32Size, __FUNCTION__, \
    AK_BASE_GetOsTimeStamp())

/**
 * AK_BASE_MemRbDestroy: destroy a ring buffer
 * @s32RbId[IN]: which ring buffer will you operate?
 * return: AK_SUCCESS or error_code
 */
AK_S32 AK_BASE_MemRbDestroy(AK_S32 s32RbId);

/**
 * AK_BASE_MemRbReset: reset a ring buffer
 * @s32RbId[IN]: which ring buffer will you operate?
 * return: AK_SUCCESS or error_code
 */
AK_S32 AK_BASE_MemRbReset(AK_S32 s32RbId);

/**
 * AK_BASE_MemRbWrite: write data to ring buffer
 * @s32RbId[IN]: which ring buffer will you operate?
 * @from[IN]: where is the write data from?
 # @s32WriteLen[IN]: write data len
 * return: AK_SUCCESS or error_code
 */
AK_S32 AK_BASE_MemRbWrite(AK_S32 s32RbId, AK_U8 *pu8From, AK_S32 s32WriteLen);

/**
 * AK_BASE_MemRbReadRequest: request a read ptr
 * @s32RbId[IN]: which ring buffer will you operate?
 * @ps32ReadptrId[OUT]: read ptr output to user
 * return: AK_SUCCESS or error_code
 */
AK_S32 AK_BASE_MemRbReadRequest(AK_S32 s32RbId, AK_S32 *ps32ReadptrId);

/**
 * AK_BASE_MemRbReadCancel: cancel a read ptr
 * @s32RbId[IN]: which ring buffer will you operate?
 * @s32ReadptrId[IN]: which read_ptr will you cancel?
 * return: AK_SUCCESS or error_code
 */
AK_S32 AK_BASE_MemRbReadCancel(AK_S32 s32RbId, AK_S32 s32ReadptrId);

/**
 * AK_BASE_MemRbRead: read data from ring buffer
 * @s32RbId[IN]: which ring buffer will you read?
 * @s32ReadptrId[IN]: which read_ptr will you use when read data from ring buffer?
 * @to[OUT]: where will be writed when read data?
 * @read_len[IN]: plan read data len
 * @result[OUT]: read data len when actual read
 * return: AK_SUCCESS or error_code
 */
AK_S32 AK_BASE_MemRbRead(AK_S32 s32RbId, AK_S32 s32ReadptrId, AK_U8 *pu8To, \
    AK_S32 s32ReadLen, AK_S32 *ps32Result);

/**
 * AK_BASE_MemRbGetDataSize: Get the size of ring buffer
 * @s32RbId[IN]: ring buffer id
 * @size[OUT]: the data length in ring buffer
 * return: AK_SUCCESS or error_code
 */
AK_S32 AK_BASE_MemRbGetDataSize(AK_S32 s32RbId, AK_S32 *ps32Size);

/**
 * AK_BASE_MemDmaAlloc_: alloc one dma memory blk
 * @s32ModuleId[IN]: which module apply memory?
 * @u32Size[IN]: what size of the memory blk is?
 * @pcFuncName[IN]: function entry where you apply memory blk
 * @s64Time[IN]: what s64Time the memory blk was applied?
 * return: dma memory blk pointer or NULL
 */
AK_VOID * AK_BASE_MemDmaAlloc_(AK_S32 s32ModuleId, AK_U32 u32Size, \
    const AK_CHAR *pcFuncName, AK_S64 s64Time);
#define AK_BASE_MemDmaAlloc(s32ModuleId, u32Size) \
    AK_BASE_MemDmaAlloc_(s32ModuleId, u32Size, __FUNCTION__, AK_BASE_GetOsTimeStamp())

/*
 * AK_BASE_MemSyncBegin: begin to sync dma cache;
 * @pvVaddr[IN]: vaddr
 * @u32Size[IN]: size
 */
AK_S32 AK_BASE_MemSyncBegin(AK_VOID *pvVaddr, AK_U32 u32Size);

/*
 * AK_BASE_MemSyncEnd: end to sync dma cache;
 * @pvVaddr[IN]: vaddr
 * @u32Size[IN]: size
 */
AK_S32 AK_BASE_MemSyncEnd(AK_VOID *pvVaddr, AK_U32 u32Size);

/**
 * AK_BASE_MemDmaAllocCache_: alloc one dma memory blk can be cacheable,improve cpu copy performance;
 * @s32ModuleId[IN]: which module apply memory?
 * @u32Size[IN]: what size of the memory blk is?
 * @pcFuncName[IN]: function entry where you apply memory blk
 * @time[IN]: what time the memory blk was applied?
 * return: dma memory blk pointer or NULL
 */
AK_VOID * AK_BASE_MemDmaAllocCache_(AK_S32 s32ModuleId, AK_U32 u32Size, \
    const AK_CHAR *pcFuncName, AK_S64 s64Time);
#define AK_BASE_MemDmaAllocCache(s32ModuleId, u32Size) \
    AK_BASE_MemDmaAllocCache_(s32ModuleId, u32Size, __FUNCTION__, AK_BASE_GetOsTimeStamp())


/**
 * AK_BASE_MemDmaFree: free one dma memory blk
 * @ptr[IN]: virtual address
 * return: AK_SUCCESS or error_code
 */
AK_S32 AK_BASE_MemDmaFree(AK_VOID *pvPtr);

/**
 * AK_BASE_MemDmaFreeCache: free one dma memory blk
 * @pvPtr[IN]: virtual address
 * return: AK_SUCCESS or error_code
 */
AK_S32 AK_BASE_MemDmaFreeCache(AK_VOID *pvPtr);

/**
 * AK_BASE_MemDmaVaddr2Paddr: virtual address switch to physical address
 * @ptr[IN]: virtual address
 * @paddr[OUT] : physical address
 * return: AK_SUCCESS or error_code
 */
AK_S32 AK_BASE_MemDmaVaddr2Paddr(AK_VOID *pvPtr, AK_U64 *pu64Paddr);

/**
 * AK_BASE_MemDmaGetUsed: get dma memory used total
 * @s32ModuleId[IN]: which module you want to query?
 * @p_total_used[OUTPUT] : where the result will be placed?
 * return: AK_SUCCESS or error_code
 */
AK_S32 AK_BASE_MemDmaGetUsed(AK_S32 s32ModuleId, AK_U64 *pu64TotalUsed);

/**
 * AK_BASE_MemGetUsedTotal: get memory used total
 * @s32ModuleId[IN]: which module you want to query?
 * @p_total_used[OUT] : where the result will be placed?
 * return: AK_SUCCESS or error_code
 */
AK_S32 AK_BASE_MemGetUsedTotal(AK_S32 s32ModuleId, AK_U64 *pu64TotalUsed);

/**
 * AK_BASE_MemGetUsedDetail: get memory used detail
 * @s32ModuleId[IN]: which module you want to query?
 * @ppstMemUsed[OUTPUT]: just give me a USED_MEM_INFO_S point, that will be ok
 * return: AK_SUCCESS or error_code
 */
AK_S32 AK_BASE_MemGetUsedDetail(AK_S32 s32ModuleId, USED_MEM_INFO_S **ppstMemUsed);

/**
 * AK_BASE_MemReleaseUsedDetail: release memory used detail info
 * @pstMemUsed[OUTPUT]: memory used detail info given by AK_BASE_MemGetUsedTotal interface
 * return: AK_SUCCESS or error_code
 */
AK_S32 AK_BASE_MemReleaseUsedDetail(USED_MEM_INFO_S *pstMemUsed);

/* AK_BASE_MemDmaPoolCreate 		--		create dma memory pool
 * @mid[IN]		:	module id that system used
 * @u32Size[IN]	:	size info of the DMA pool
 * return AK_SUCCESS if success, AK_FAILED if failed
 * */
AK_S32 AK_BASE_MemDmaPoolCreate(MODULE_ID_E enMid, AK_U32 u32Size);

/*
 * AK_BASE_MemDmaPoolActivate: init the dma memory pool
 * return: AK_SUCCESS if success, AK_FAILED if failed
 * */
AK_S32 AK_BASE_MemDmaPoolActivate(AK_VOID);

    /**
 * AK_BASE_MemDmaPoolExit: exit memory module
 * return: AK_SUCCESS
 */
AK_S32 AK_BASE_MemDmaPoolExit(AK_VOID);

/* AK_BASE_MemDmaPoolGetUsedMem	-- get the used dma pool info
 * @mid[IN]		:		module id
 * @total[OUT]	:		pointer to record the mem info
 * return AK_SUCCESS if successful, and AK_FAILED if failure.
 * */
AK_S32 AK_BASE_MemDmaPoolGetUsedMem(MODULE_ID_E enMid, AK_U64 *pu64Total);

/* AK_BASE_MemDmaPoolGetFreeMem	-- get the free dma pool info
 * @mid[IN]		:		module id
 * @total[OUT]	:		pointer to record the mem info
 * return AK_SUCCESS if successful, and AK_FAILED if failure.
 * */
AK_S32 AK_BASE_MemDmaPoolGetFreeMem(MODULE_ID_E enMid, AK_U64 *pu64Total);

/* AK_BASE_MemDmaPoolSetThreshold	---- set the dma pool alloc threadhold
 * @mid[IN]		:		module id
 * @threadhold[OUT]	:		threshold of the dma pool alloc
 * return AK_SUCCESS if successful, and AK_FAILED if failure.
 */
AK_S32 AK_BASE_MemDmaPoolSetThreshold(MODULE_ID_E enMid, AK_U64 u64Threshold);

/* AK_BASE_MemDmaPoolCreateReservedObj	---	create reserved dma dma obj for specific usage
 * @mid[IN]					:			module id
 * @size[IN]				:		 	size of the dma buf obj
 * @alloc_threadhold[IN]	:			threshold of the dma buf obj alloc
 * return AK_SUCCESS if successful, and AK_FAILED if failure.
 */
AK_S32 AK_BASE_MemDmaPoolCreateReservedObj(MODULE_ID_E enMid, \
    AK_U64 u64Size, AK_U64 u64AllocThreshold);

/* AK_BASE_MemDmaGetReservedSize		--- get the system reserved total dma size
 * @size[OUT]		:	record the system reserved total dma size
 * return AK_SUCCESS if succesful, and AK_FAILED if failure
 * */
AK_S32 AK_BASE_MemDmaGetReservedSize(AK_U64 *pu64Size);

/* AK_BASE_MemGetTotalSize		--- get the system total memory size
 * @size[OUT]		:	record the system total memory size
 * return AK_SUCCESS if succesful, and AK_FAILED if failure
 * */
AK_S32 AK_BASE_MemGetTotalSize(AK_U64 *pu64Size);
#endif

/* end of file */
