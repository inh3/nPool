#ifndef _SYNCHRONIZE_H_
#define _SYNCHRONIZE_H_

/*---------------------------------------------------------------------------*/
/* FILE INCLUSION */
/*---------------------------------------------------------------------------*/

#ifdef _WIN32

#include <Windows.h>

// _beginthreadex
#include <process.h>

#else

#include <pthread.h>

#endif

/*---------------------------------------------------------------------------*/
/* MACRO DEFINITIONS */
/*---------------------------------------------------------------------------*/

/* N/A */

/*---------------------------------------------------------------------------*/
/* ENUMERATIONS */
/*---------------------------------------------------------------------------*/

/* N/A */

/*---------------------------------------------------------------------------*/
/* TYPE DECLARATIONS */
/*---------------------------------------------------------------------------*/

#ifdef _WIN32

// typedefs
typedef CONDITION_VARIABLE  THREAD_COND;
typedef CRITICAL_SECTION    THREAD_MUTEX;
typedef HANDLE              THREAD;
typedef DWORD               THREAD_FUNC;

// defines
#define THREAD_FUNC_RETURN  0

#else

#define WINAPI

typedef pthread_cond_t      THREAD_COND;
typedef pthread_mutex_t     THREAD_MUTEX;
typedef pthread_t           THREAD;
typedef void*               THREAD_FUNC;

// defines
#define THREAD_FUNC_RETURN  NULL

#endif

/*---------------------------------------------------------------------------*/
/* FUNCTION PROTOTYPES */
/*---------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

/* Thread Functions */

unsigned int        SyncGetThreadId();

int                 SyncCreateThread(THREAD *threadRef, void* threadAttr, THREAD_FUNC (WINAPI *threadFunction)(void *), void *threadContext);

int                 SyncJoinThread(THREAD threadRef, void** returnValue);

/* Mutex Functions */

int                 SyncCreateMutex(THREAD_MUTEX *mutexRef, void* mutexAttr);

int                 SyncDestroyMutex(THREAD_MUTEX *mutexRef);

int                 SyncLockMutex(THREAD_MUTEX *mutexRef);

int                 SyncUnlockMutex(THREAD_MUTEX *mutexRef);

/* Conditional Functions */

int                 SyncCreateCond(THREAD_COND *condRef, void* condAttr);

int                 SyncDestroyCond(THREAD_COND *condRef);

int                 SyncWaitCond(THREAD_COND *condRef, THREAD_MUTEX *mutexRef);

int                 SyncSignalCond(THREAD_COND *condRef);

int                 SyncBroadcastCond(THREAD_COND *condRef);

#ifdef __cplusplus
}
#endif

/*---------------------------------------------------------------------------*/
/* OBJECT DECLARATIONS */
/*---------------------------------------------------------------------------*/

#ifndef _SYNCHRONIZE_C_

/* N/A */

#endif /* _SYNCHRONIZE_C_ */

/*****************************************************************************/

#endif /* _SYNCHRONIZE_H_ */