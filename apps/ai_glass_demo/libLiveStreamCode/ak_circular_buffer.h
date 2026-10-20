/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd. */



#ifndef _CIRBUF_H_
#define _CIRBUF_H_


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Circular buffer handle type definition.
 */
typedef void* CIRBUF_HANDLE;

//#define CirularHeadInfoMax 	(48)		//Maximum size of data header information

/**
 * @brief Circular buffer process overview.
 *
 * The process is divided into two parts:
 * 1. Writing data into the circular buffer:
 *    cirbuf_new() -> cirbuf_get_write_buf() -> cirbuf_put_write_buf()
 *
 * 2. Reading data from the circular buffer (requires registering a reader):
 *    cirbuf_reader_new() -> cirbuf_get_buff_data()
 *
 * @note
 * - cirbuf_reader_new() returns a reader ID, which must be passed to cirbuf_get_buff_data().
 * - cirbuf_reader_new() and cirbuf_reader_delete() are a pair.
 * - cirbuf_new() and cirbuf_delete() are a pair.
 */

/**
 * @brief Creates a new circular buffer.
 *
 * @param unMemSize Size of the buffer memory to allocate.
 * @return CIRBUF_HANDLE Handle to the created circular buffer, or NULL on failure.
 */
CIRBUF_HANDLE cirbuf_new(unsigned int unMemSize);

/**
 * @brief Deletes a circular buffer.
 *
 * Releases all resources associated with the circular buffer.
 *
 * @param pCirBufHandle Handle to the circular buffer to delete.
 * @return int 0 on success, negative error code on failure.
 */
int cirbuf_delete(CIRBUF_HANDLE pCirBufHandle);

/**
 * @brief Gets the write address for the circular buffer.
 *
 * @param pCirBufHandle Handle to the circular buffer.
 * @param unWriteLen Length of the data to write.
 * @return unsigned char* Pointer to the write buffer, or NULL if insufficient space.
 */
unsigned char * cirbuf_get_write_buf(CIRBUF_HANDLE pCirBufHandle, unsigned int unWriteLen);

/**
 * @brief Updates the buffer state after writing data.
 *
 * @param pCirBufHandle Handle to the circular buffer.
 * @param unWriteLen Length of the data actually written.
 * @return int 0 on success, negative error code on failure.
 */
int cirbuf_put_write_buf(CIRBUF_HANDLE pCirBufHandle, unsigned int unWriteLen);

/**
 * @brief Registers a new reader for the circular buffer.
 *
 * @param pCirBufHandle Handle to the circular buffer.
 * @param pName Name of the reader (for debugging or identification).
 * @return int Reader ID (non-negative) on success, negative error code on failure.
 */
int cirbuf_reader_new(CIRBUF_HANDLE pCirBufHandle, char *pName);

/**
 * @brief Deletes a registered reader.
 *
 * @param pCirBufHandle Handle to the circular buffer.
 * @param unReaderId ID of the reader to delete.
 * @return int 0 on success, negative error code on failure.
 */
int cirbuf_reader_delete(CIRBUF_HANDLE pCirBufHandle, unsigned int unReaderId);

/**
 * @brief Retrieves data from the circular buffer.
 *
 * @param pCirBufHandle Handle to the circular buffer.
 * @param unReaderId ID of the reader.
 * @param ppDataBuf Double pointer to store the address of the data buffer.
 * @param pDataSize Pointer to store the size of the retrieved data.
 * @param unTimeOutms Timeout in milliseconds to wait for data.
 * @return int 0 on success, negative error code on failure.
 */
int cirbuf_get_buff_data(CIRBUF_HANDLE pCirBufHandle, unsigned int unReaderId, unsigned char **ppDataBuf, unsigned int *pDataSize, unsigned int unTimeOutms);

#ifdef __cplusplus
}
#endif

#endif
