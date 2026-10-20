/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd. */
#ifndef _AK_THREAD_H_
#define _AK_THREAD_H_

#include <semaphore.h>
#include <pthread.h>
#include "ak_types.h"

#define ANYKA_THREAD_NORMAL_STACK_SIZE 	(200*1024)
#define ANYKA_THREAD_MIN_STACK_SIZE 	(100*1024)

#define ANYKA_THREAD_PRIO_HIGH      40     // 40 ~ 59
#define ANYKA_THREAD_PRIO_NORMAL    60     // 60 ~ 79
#define ANYKA_THREAD_PRIO_LOW       80     // 80 ~ 99

typedef enum akMUTEX_ATTR_E {
    AK_MUTEX_ATTR_NORMAL,
    AK_MUTEX_ATTR_RECURSIVE,
}MUTEX_ATTR_E;

typedef sem_t                AK_SEM_T;

typedef pthread_t            AK_PTHREAD_T;

typedef pthread_attr_t       AK_PTHREAD_ATTR_T;

typedef pthread_mutex_t      AK_MUTEX_T;

typedef pthread_mutexattr_t	 AK_MUTEXATTR_T;

typedef pthread_rwlock_t     AK_RWLOCK_T;

typedef pthread_rwlockattr_t AK_RWLOCKATTR_T;

typedef pthread_cond_t       AK_COND_T;

typedef AK_VOID* (* pfnBASE_ThreadFunc)(AK_VOID *param);

#ifdef __cplusplus
extern "C" {
#endif

/**
 * AK_BASE_ThreadGetVersion - get thread module version
 * return: version string
 */
const AK_CHAR* AK_BASE_ThreadGetVersion(AK_VOID);

/**
 * AK_BASE_ThreadCreate - create a posix thread.
 * @ptId[OUT]: thread id
 * @func[IN]: thread function
 * @pvArg[IN]: argument for thread
 * @s32StackSize[IN]: thread stack size
 * @s32Priority[IN]: thread s32Priority, default -1
 * return: 0 success; otherwise error number
 */
AK_S32 AK_BASE_ThreadCreate(AK_PTHREAD_T *ptId, pfnBASE_ThreadFunc func, AK_VOID *pvArg,
        AK_S32 s32StackSize, AK_S32 s32Priority);

/**
 * AK_BASE_ThreadJoin - join the thread
 * @tId[IN]: thread id
 * return: 0 success; otherwise error number
 */
AK_S32 AK_BASE_ThreadJoin(AK_PTHREAD_T tId);

/**
 * AK_BASE_ThreadDetach - detach the thread
 * @tId[IN]: thread id
 * return: 0 success; otherwise error number
 */
AK_S32 AK_BASE_ThreadDetach(AK_PTHREAD_T tId);

/**
 * AK_BASE_ThreadCancel - cancel the thread
 * @tId[IN]: thread id
 * return: 0 success; otherwise error number
 */
AK_S32 AK_BASE_ThreadCancel(AK_PTHREAD_T tId);

/**
 * AK_BASE_ThreadExit - thread exit
 * return: do not return
 */
AK_VOID AK_BASE_ThreadExit(AK_VOID);

/**
 * AK_BASE_ThreadGetTid - get thread id
 * return: thread id belongs to call function
 */
AK_S64 AK_BASE_ThreadGetTid(AK_VOID);

/**
 * AK_BASE_ThreadMutexInit - init ptMutex
 * @ptMutex[OUT]: ptMutex pointer
 * @ptAttr[IN]: mutexattribute pointer
 * return: 0 success; otherwise error number
 */
AK_S32 AK_BASE_ThreadMutexInit(AK_MUTEX_T *ptMutex, AK_MUTEXATTR_T *ptAttr);

/**
 * AK_BASE_ThreadMutexLock - lock ptMutex
 * @ptMutex[IN]: ptMutex pointer
 * return: 0 success; otherwise error number
 */
AK_S32 AK_BASE_ThreadMutexLock(AK_MUTEX_T *ptMutex);

/**
 * AK_BASE_ThreadMutexUnlock - unlock ptMutex
 * @ptMutex[IN]: ptMutex pointer
 * return: 0 success; otherwise error number
 */
AK_S32 AK_BASE_ThreadMutexUnlock(AK_MUTEX_T *ptMutex);

/**
 * AK_BASE_ThreadMutexDestroy - destroy ptMutex
 * @ptMutex[IN]: ptMutex pointer
 * return: 0 success; otherwise error number
 */
AK_S32 AK_BASE_ThreadMutexDestroy(AK_MUTEX_T *ptMutex);

/**
 * AK_BASE_ThreadSemInit - init semaphore
 * @ptSem[IN]: semaphore pointer
 * @u32Value[IN]: semaphore initial value
 * return: 0 success; otherwise error number
 */
AK_S32 AK_BASE_ThreadSemInit(AK_SEM_T *ptSem, AK_U32 u32Value);

/**
 * AK_BASE_ThreadSemPost - post semaphore
 * @ptSem[IN]: semaphore pointer
 * return: 0 success; otherwise error number
 */
AK_S32 AK_BASE_ThreadSemPost(AK_SEM_T *ptSem);


/**
 * AK_BASE_ThreadSemWait - wait semaphore
 * @ptSem[IN]: semaphore pointer
 * return: 0 success; otherwise error number
 */
AK_S32 AK_BASE_ThreadSemWait(AK_SEM_T *ptSem);

/**
 * AK_BASE_ThreadSemDestroy - destroy semaphore
 * @ptSem[IN]: semaphore pointer
 * return: 0 success; otherwise error number
 */
AK_S32 AK_BASE_ThreadSemDestroy(AK_SEM_T *ptSem);

/**
 * AK_BASE_ThreadSemTrywait - try to wait semaphore
 * @ptSem[IN]: semaphore pointer
 * return: 0 success; otherwise error number
 */
AK_S32 AK_BASE_ThreadSemTrywait(AK_SEM_T *ptSem);

/**
 * AK_BASE_ThreadSemTimeWait - wait semaphore timeout
 * @ptSem[IN]: semaphore pointer
 * @abs_timeout[IN]: specific timeout
 * return: 0 success; otherwise error number
 */
AK_S32 AK_BASE_ThreadSemTimeWait(AK_SEM_T *ptSem, const struct timespec *abs_timeout);

/**
 * AK_BASE_ThreadSetName - set thread name
 * @pcName[IN]: pointer to 'thread name' no longer than 15B
 * return: 0 success; otherwise error number
 */
AK_S32 AK_BASE_ThreadSetName(const AK_CHAR *pcName);

/**
 * AK_BASE_ThreadCondInit - init condition
 * @ptCond[OUT]: condition pointer
 * return: 0 success; otherwise error number
 */
AK_S32 AK_BASE_ThreadCondInit(AK_COND_T *ptCond);


/**
 * AK_BASE_ThreadCondDestroy - destroy condition
 * @ptCond[IN]: condition pointer
 * return: 0 success; otherwise error number
 */
AK_S32 AK_BASE_ThreadCondDestroy(AK_COND_T *ptCond);


/**
 * AK_BASE_ThreadCondWait - wait condition
 * @ptCond[IN]: condition pointer
 * @ptMutex[IN]: mutext pointer
 * return: 0 success; otherwise error number
 */
AK_S32 AK_BASE_ThreadCondWait(AK_COND_T *ptCond, AK_MUTEX_T *ptMutex);

/**
 * AK_Base_ThreadCondTimewait - wait condition timeout
 * @ptCond[IN]: condition pointer
 * @ptMutex[IN]: mutext pointer
 * @abstime[IN]: timeout value
 * return: 0 success; otherwise error number
 */
AK_S32 AK_Base_ThreadCondTimewait(AK_COND_T *ptCond,
        AK_MUTEX_T *ptMutex, const struct timespec *abstime);

/**
 * AK_BASE_ThreadCondSignal - post signal on condition
 * @ptCond[IN]: condition pointer
 * return: 0 success; otherwise error number
 */
AK_S32 AK_BASE_ThreadCondSignal(AK_COND_T *ptCond);

/**
 * AK_BASE_ThreadCondBroadcast - post signal by broadcast on condition
 * @ptCond[IN]: condition pointer
 * return: 0 success; otherwise error number
 */
AK_S32 AK_BASE_ThreadCondBroadcast(AK_COND_T *ptCond);

/**
 * AK_BASE_ThreadMutexattrInit - init mutexattribute
 * @ptAttr[IN]: mutexattribute pointer
 * return: 0 success; otherwise error number
 */
AK_S32 AK_BASE_ThreadMutexattrInit(AK_MUTEXATTR_T *ptAttr);

/**
 * AK_BASE_ThreadMutexattrDestroy - destroy mutexattribute
 * @ptAttr[IN]: mutexattribute pointer
 * return: 0 success; otherwise error number
 */
AK_S32 AK_BASE_ThreadMutexattrDestroy(AK_MUTEXATTR_T *ptAttr);

/**
 * AK_BASE_ThreadMutexattrSettype - set mutexattribute type
 * @ptAttr[IN]: mutexattribute pointer
 * @s32Kind[IN]: mutexattribute type
 * return: 0 success; otherwise error number
 */
AK_S32 AK_BASE_ThreadMutexattrSettype(AK_MUTEXATTR_T *ptAttr, AK_S32 s32Kind);

/**
 * AK_BASE_ThreadMutexattrGettype - get mutexattribute type
 * @ptAttr[IN]: mutexattribute pointer
 * @ps32Kind[OUT]: mutexattribute type
 * return: 0 success; otherwise error number
 */
AK_S32 AK_BASE_ThreadMutexattrGettype(const AK_MUTEXATTR_T *ptAttr, AK_S32 *ps32Kind);

/**
 * AK_BASE_ThreadRWlockInit - init ptRWlock
 * @ptRWlock[IN] : ptRWlock pointer
 * @ptRWlockattr[IN] : ptRWlockattr pointer
 * return: 0 success; otherwise error number
 */
AK_S32 AK_BASE_ThreadRWlockInit(AK_RWLOCK_T *ptRWlock, AK_RWLOCKATTR_T *ptRWlockattr);


/**
 * AK_BASE_ThreadRWlockDestroy - destroy ptRWlock
 * @ptRWlock[IN] : ptRWlock pointer
 * return: 0 success; otherwise error number
 */
AK_S32 AK_BASE_ThreadRWlockDestroy(AK_RWLOCK_T *ptRWlock);

/**
 * AK_BASE_ThreadRWlockRDlock - get read lock
 * @ptRWlock[IN] : ptRWlock pointer
 * return: 0 success; otherwise error number
 */
AK_S32 AK_BASE_ThreadRWlockRDlock(AK_RWLOCK_T *ptRWlock);

/**
 * AK_BASE_ThreadRWlockWRlock - get write lock
 * @ptRWlock[IN] : ptRWlock pointer
 * return: 0 success; otherwise error number
 */
AK_S32 AK_BASE_ThreadRWlockWRlock(AK_RWLOCK_T *ptRWlock);

/**
 * AK_BASE_ThreadRWlockTryRDlock - try to get read lock
 * @ptRWlock[IN] : ptRWlock pointer
 * return: 0 success; otherwise error number
 */
AK_S32 AK_BASE_ThreadRWlockTryRDlock(AK_RWLOCK_T *ptRWlock);

/**
 * AK_BASE_ThreadRWlockTryWRlock - try to get write lock
 * @ptRWlock[IN] : ptRWlock pointer
 * return: 0 success; otherwise error number
 */
AK_S32 AK_BASE_ThreadRWlockTryWRlock(AK_RWLOCK_T *ptRWlock);

/**
 * AK_BASE_ThreadRWlockUnlock - unlock ptRWlock
 * @ptRWlock[IN] : ptRWlock pointer
 * return: 0 success; otherwise error number
 */
AK_S32 AK_BASE_ThreadRWlockUnlock(AK_RWLOCK_T *ptRWlock);

/**
 * AK_BASE_ThreadRWlockattrInit - init ptRWlock attribute
 * @rwlocakattr[IN] : ptRWlockattr pointer
 * return: 0 success; otherwise error number
 */
AK_S32 AK_BASE_ThreadRWlockattrInit(AK_RWLOCKATTR_T * ptRWlockattr);

/**
 * AK_BASE_ThreadRWlockattrDestroy - destroy ptRWlock attribute
 * @rwlocakattr[IN] : ptRWlockattr pointer
 * return: 0 success; otherwise error number
 */
AK_S32 AK_BASE_ThreadRWlockattrDestroy(AK_RWLOCKATTR_T * ptRWlockattr);

/**
 * AK_BASE_ThreadRWlockattrGetPshared - get process shared attribute
 * @rwlocakattr[IN] : ptRWlockattr pointer
 * @ps32Value[OUT] : value of process shared attribute
 * return: 0 success; otherwise error number
 */
AK_S32 AK_BASE_ThreadRWlockattrGetPshared(AK_RWLOCKATTR_T * ptRWlockattr, AK_S32 *ps32Value);

/**
 * AK_BASE_ThreadRWlockattrSetPshared - set process shared attribute
 * @rwlocakattr[IN] : ptRWlockattr pointer
 * @s32Value[IN] : value of process shared attribute
 * return: 0 success; otherwise error number
 */
AK_S32 AK_BASE_ThreadRWlockattrSetPshared(AK_RWLOCKATTR_T * ptRWlockattr, AK_S32 s32Value);

#ifdef __cplusplus
}
#endif
#endif	//end of _AK_THREAD_H_

/* end of file */
