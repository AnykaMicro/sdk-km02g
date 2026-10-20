/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd. */
#ifndef _RingBuf_H_
#define _RingBuf_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef void* RingBuf_Handle;

/**
 * Calling process, divided into two parts: 1) Put data into ring buffer; 2) Read data from ring buffer
 * 1:RingBuf_New()
 * 2:RingBuf_GetWriteBuf()---->RingBuf_PutWriteBuf()
 * 3:RingBuf_GetBuffData()---->RingBuf_ReleaseBuf();
 * 4:RingBuf_Delete()
 *
 * Note:
 */

/**
 * @brief Creates a new ring buffer.
 *
 * @param unMemSize Size of the memory to allocate for the buffer.
 * @return Handle to the created ring buffer, or NULL if allocation fails.
 */
RingBuf_Handle RingBuf_New(unsigned int unMemSize);

/**
 * @brief Deletes a ring buffer and frees resources.
 *
 * @param pCirBufHandle Handle to the ring buffer.
 * @return 0 on success, negative error code on failure.
 */
int RingBuf_Delete(RingBuf_Handle pCirBufHandle);

/**
 * @brief Gets the write address for the ring buffer.
 *
 * @param pCirBufHandle Handle to the ring buffer.
 * @param unWriteLen Length of data to be written.
 * @return Pointer to the write buffer, or NULL if insufficient space.
 */
unsigned char * RingBuf_GetWriteBuf(RingBuf_Handle pCirBufHandle, unsigned int unWriteLen);

/**
 * @brief Updates the ring buffer after writing data.
 *
 * @param pCirBufHandle Handle to the ring buffer.
 * @param unWriteLen Length of data actually written.
 * @return 0 on success, negative error code on failure.
 */
int RingBuf_PutWriteData(RingBuf_Handle pCirBufHandle, unsigned int unWriteLen);


/**
 * @brief Retrieves data from the ring buffer.
 *
 * @param pCirBufHandle Handle to the ring buffer.
 * @param ppDataBuf Pointer to a pointer to store the data buffer address.
 * @param pDataSize Pointer to store the size of the retrieved data.
 * @param unTimeOutms Timeout in milliseconds.
 * @return 0 on success, negative error code on failure or timeout.
 */
int RingBuf_GetBuffData(RingBuf_Handle pCirBufHandle, unsigned char **ppDataBuf, unsigned int *pDataSize, unsigned int unTimeOutms);

/**
 * @brief Releases the buffer after data processing is complete.
 *
 * @param pCirBufHandle Handle to the ring buffer.
 * @return 0 on success, negative error code on failure.
 */
int RingBuf_ReleaseBuf(RingBuf_Handle pCirBufHandle);


/**
 * @brief Gets the current frame count in the buffer.
 *
 * @param pCirBufHandle Handle to the ring buffer.
 * @return Number of frames in the buffer.
 */
int RingBuf_GetFrameCont(RingBuf_Handle pCirBufHandle);


#ifdef __cplusplus
}
#endif

#endif
