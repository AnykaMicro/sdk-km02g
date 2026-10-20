/* SPDX-License-Identifier: Apache-2.0 */
/**
 * @file ring_buffer.c
 * @brief Thread-safe ring buffer for frame data read/write.
 * @copyright Copyright (c) 2025 Guangzhou Anyka Microelectronics Co., Ltd.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "ring_buffer.h"
#if 0
#include "aas_common.h"
#else
	#define aas_malloc(x) malloc(x)
	#define aas_free(x) free(x)
#endif

typedef enum
{
    RB_Frame_State_Idle = 0,    // Idle state, read data completed
    RB_Frame_State_Writing = 1, // Writing data
    RB_Frame_State_Written = 2, // Write data completed
    RB_Frame_State_Reading = 3, // Reading data
    RB_Frame_State_Turn_Around = 4, // Turn around flag, used when not enough space at the end to write a frame
}RB_Frame_State;

typedef struct _RB_Frame_Head
{
    RB_Frame_State state;						// Data state
	unsigned int unDataLen; 		// Data length
}RB_Frame_Head;

typedef struct
{
	pthread_mutex_t	hMutex;									// Management lock
	unsigned char *	pMemStart;								// Ring buffer start position
	unsigned int	unTotalSize;							// Ring buffer total size
    unsigned int	unReadPos;							    // Read position
	unsigned int	unCurPos;								// Current write position
    unsigned int    unFrameCount;                           // Number of data frames in buffer
} RingBuf_Manager;

/**
 * @brief Create a ring buffer of the given size.
 * @param unMemSize Buffer size in bytes
 * @return Handle, or NULL on failure
 */
RingBuf_Handle RingBuf_New(unsigned int unMemSize)
{
	if(unMemSize == 0)
	{
		printf("invalid arguments\n");
		return NULL;
	}

	void * pCirBufStart = (void *)aas_malloc(unMemSize);
	if(pCirBufStart == NULL)
	{
		printf("Out of memory!unMemSize=%d \n",unMemSize);
		return NULL;
	}

    RingBuf_Manager *pCirularHandle = {0};
	pCirularHandle = aas_malloc(sizeof(RingBuf_Manager));
	if(pCirularHandle == NULL)
	{
		if(pCirBufStart != NULL)
		{
			aas_free(pCirBufStart);
			pCirBufStart = NULL;
		}
		printf("Out of memory!\n");
		return NULL;
	}
	memset(pCirularHandle,0,sizeof(RingBuf_Manager));
	pCirularHandle->pMemStart = (unsigned char *)pCirBufStart;
	pCirularHandle->unTotalSize = unMemSize;
	pthread_mutex_init(&pCirularHandle->hMutex, NULL);

	return (void *)pCirularHandle;
}

/**
 * @brief Destroy ring buffer and free resources.
 * @param pCirBufHandle Handle returned by RingBuf_New
 * @return 0 on success
 */
int RingBuf_Delete(RingBuf_Handle pCirBufHandle)
{
    RingBuf_Manager *pHandle = (RingBuf_Manager*)pCirBufHandle;

	if(pHandle)
	{
		pthread_mutex_destroy(&pHandle->hMutex);

		void *ptmp = pHandle->pMemStart ;

		if(pHandle != NULL)
		{
			aas_free(pHandle);
			pHandle = NULL;
		}

		if(ptmp != NULL)
		{
			aas_free(ptmp);
			ptmp = NULL;
		}
	}

	return 0;
}

unsigned char* RingBuf_GetWriteBuf(RingBuf_Handle pCirBufHandle, unsigned int unWriteLen)
{
    RingBuf_Manager*pHandle = (RingBuf_Manager*)pCirBufHandle;

	if(pHandle == NULL || unWriteLen == 0)
	{
		printf("invalid arguments\n");
		return NULL;
	}
	if(unWriteLen > pHandle->unTotalSize)
	{
		printf("outof the BuffSize(%d < %d) .\n",pHandle->unTotalSize,unWriteLen);
		return NULL;
	}

    unsigned char* pRet = NULL;

	pthread_mutex_lock(& pHandle->hMutex);

    if (pHandle->unFrameCount == 0) // Data is empty
    {
        pHandle->unCurPos = 0;
        pHandle->unReadPos = 0;
        pRet = pHandle->pMemStart + sizeof(RB_Frame_Head);
    }
    else if (pHandle->unCurPos > pHandle->unReadPos) // Write pointer is behind
    {
        if (pHandle->unCurPos + sizeof(RB_Frame_Head) + unWriteLen <= pHandle->unTotalSize) // Length behind is enough
        {
            pRet = pHandle->pMemStart + sizeof(RB_Frame_Head) + pHandle->unCurPos;
        }
        else // Length behind is not enough, need to try turn around
        {
            if (unWriteLen + sizeof(RB_Frame_Head) < pHandle->unReadPos) // After turn around, write pointer will not overwrite read pointer
            {
                if (pHandle->unCurPos + sizeof(RB_Frame_Head) <= pHandle->unTotalSize) /// Enough length for a head behind, need to write turn around flag
                {
                    RB_Frame_Head* ph = (RB_Frame_Head*)(pHandle->pMemStart + pHandle->unCurPos);
                    ph->unDataLen = 0;
                    ph->state = RB_Frame_State_Turn_Around;
                }
                pRet = pHandle->pMemStart + sizeof(RB_Frame_Head);
                pHandle->unCurPos = 0;
            }
            else // After turn around, will still overwrite data
            {
				pthread_mutex_unlock(& pHandle->hMutex);
                return NULL;
            }
         }
    }
    else // Write pointer is ahead
    {
        if (pHandle->unCurPos + unWriteLen < pHandle->unReadPos) // Write pointer will not overwrite read pointer
        {
            pRet = pHandle->pMemStart + sizeof(RB_Frame_Head) + pHandle->unCurPos;
        }
        else
        {
			pthread_mutex_unlock(& pHandle->hMutex);
            return NULL;
        }
    }

    RB_Frame_Head* ph = (RB_Frame_Head*)(pRet - sizeof(RB_Frame_Head));
    ph->state = RB_Frame_State_Writing;
    ph->unDataLen = unWriteLen;

	pthread_mutex_unlock(& pHandle->hMutex);
    return pRet;
}

int RingBuf_PutWriteData(RingBuf_Handle pCirBufHandle, unsigned int unWriteLen)
{
    RingBuf_Manager*pHandle = (RingBuf_Manager*)pCirBufHandle;
	if(pHandle == NULL || unWriteLen == 0)
	{
		printf("invalid arguments\n");
		return -1;
	}
	if(unWriteLen > pHandle->unTotalSize)
	{
		printf("unWriteLen=%d outof unTotalSize=%d \n",unWriteLen,pHandle->unTotalSize);
		return -1;
	}

	pthread_mutex_lock(& pHandle->hMutex);

    RB_Frame_Head* ph = (RB_Frame_Head*)(pHandle->pMemStart + pHandle->unCurPos);
    if ( unWriteLen > ph->unDataLen) // Actual written data is larger than requested in CirBuf_GetWriteBuf, illegal
    {
        pthread_mutex_unlock(&pHandle->hMutex);
        return -1;
    }

    ph->unDataLen = unWriteLen;
    ph->state = RB_Frame_State_Written;
    pHandle->unCurPos += (unWriteLen + sizeof(RB_Frame_Head));
    pHandle->unFrameCount++;

	pthread_mutex_unlock(& pHandle->hMutex);
	return 0;
}

int RingBuf_GetBuffData(RingBuf_Handle pCirBufHandle, unsigned char** ppDataBuf, unsigned int* pDataSize, unsigned int unTimeOutms)
{
    RingBuf_Manager*pHandle = (RingBuf_Manager*)pCirBufHandle;

	unsigned int unOutTimeMs = 0;
	if(pHandle == NULL || ppDataBuf == NULL || pDataSize == NULL)
	{
		printf("Invalid Input Arguments\n");
		return -1;
	}

    RB_Frame_Head* ph = NULL;
	while(1)
	{
		pthread_mutex_lock(& pHandle->hMutex);

        if (pHandle->unFrameCount > 0) // Buffer has data
        {
            if (pHandle->unReadPos + sizeof(RB_Frame_Head) > pHandle->unTotalSize) // Not enough space for a header at the end
            {
                pHandle->unReadPos = 0;
            }
            else // Enough space for a header at the end
            {
                ph = (RB_Frame_Head*)(pHandle->pMemStart + pHandle->unReadPos);
                if (ph->state == RB_Frame_State_Turn_Around) // If it is a turn-around flag, adjust the read position
                {
                    pHandle->unReadPos = 0;
                }
            }

            ph = (RB_Frame_Head*)(pHandle->pMemStart + pHandle->unReadPos);

            if (ph->state == RB_Frame_State_Written && ph->unDataLen > 0)
            {
                ph->state = RB_Frame_State_Reading;
                break;
            }
            else // Should not happen normally
            {
                if (ph->state = RB_Frame_State_Reading) // Should not happen if interface is used correctly
                {
                    printf("%s_%d==fatal error. GetBuffData and ReleaseBuf must pair ==\n", __FUNCTION__, __LINE__);
                }
				pthread_mutex_unlock(& pHandle->hMutex);
                printf("%s_%d===========fatal error.=========\n", __FUNCTION__, __LINE__);
                return -1;
            }
        }

		pthread_mutex_unlock(& pHandle->hMutex);

		if(unTimeOutms > 0)
		{
			if(unTimeOutms <= unOutTimeMs)
			{
				return -2; // Timeout
			}
			usleep(5*1000);
			unOutTimeMs += 5;
		}
		else
		{
			//printf("======================getstream data error.================\n");
			return -3; // No data
		}
	}

	*ppDataBuf = pHandle->pMemStart + pHandle->unReadPos + sizeof(RB_Frame_Head);
	*pDataSize = ph->unDataLen;

	pthread_mutex_unlock(& pHandle->hMutex);
	return 0;
}

int RingBuf_ReleaseBuf(RingBuf_Handle pCirBufHandle)
{
    RingBuf_Manager* pHandle = (RingBuf_Manager*)pCirBufHandle;

    if (pHandle == NULL)
    {
        printf("Invalid Input Arguments\n");
        return -1;
    }

    RB_Frame_Head* ph = NULL;
    pthread_mutex_lock(&pHandle->hMutex);

    ph = (RB_Frame_Head*)(pHandle->pMemStart + pHandle->unReadPos);
    if (ph->state != RB_Frame_State_Reading) // Exception handling, should not happen if interface is used correctly
    {
        pthread_mutex_unlock(&pHandle->hMutex);
        printf("%s_%d===========fatal error.=========\n", __FUNCTION__, __LINE__);
        return -1;
    }
    ph->state = RB_Frame_State_Idle;

    pHandle->unReadPos += (ph->unDataLen + sizeof(RB_Frame_Head));
    pHandle->unFrameCount--;

    pthread_mutex_unlock(&pHandle->hMutex);
    return 0;
}

int RingBuf_GetFrameCont(RingBuf_Handle pCirBufHandle)
{
	if(pCirBufHandle == NULL)
	{
		return 0;
	}

    RingBuf_Manager*pHandle = (RingBuf_Manager*)pCirBufHandle;

	return pHandle->unFrameCount;
}
