#define _SYNCHRONIZE_C_

/*---------------------------------------------------------------------------*/
/* FILE INCLUSION */
/*---------------------------------------------------------------------------*/

#include "synchronize.h"

#include <stdio.h>

/*---------------------------------------------------------------------------*/
/* MACRO DEFINITIONS */
/*---------------------------------------------------------------------------*/

/* N/A */

/*---------------------------------------------------------------------------*/
/* TYPE DECLARATIONS */
/*---------------------------------------------------------------------------*/

/* N/A */

/*---------------------------------------------------------------------------*/
/* FUNCTION PROTOTYPES */
/*---------------------------------------------------------------------------*/

/* N/A */

/*---------------------------------------------------------------------------*/
/* OBJECT DECLARATIONS */
/*---------------------------------------------------------------------------*/

/* N/A */

/*---------------------------------------------------------------------------*/
/* STATIC FUNCTION DEFINITIONS */
/*---------------------------------------------------------------------------*/

/* N/A */

#ifdef _WIN32

/*---------------------------------------------------------------------------*/
/* WINDOWS FUNCTION DEFINITIONS */
/*---------------------------------------------------------------------------*/

// GetCurrentThreadId
unsigned int        SyncGetThreadId()
{
    return GetCurrentThreadId();
}

// CreateThread
int                 SyncCreateThread(THREAD *threadRef, void* threadAttr, THREAD_FUNC (WINAPI *threadFunction)(void *), void *threadContext)
{
    DWORD threadId = 0;
    
    // A thread in an executable that calls the C run-time library (CRT) 
    // should use the _beginthreadex and _endthreadex functions
    // http://msdn.microsoft.com/en-us/library/windows/desktop/ms682453(v=vs.85).aspx
    // http://msdn.microsoft.com/en-us/library/kdzttdcb(v=vs.110).aspx
    *threadRef = (THREAD)_beginthreadex(
            NULL,                   // default security attributes
            0,                      // use default stack size  
            threadFunction,         // thread function name
            threadContext,          // argument to thread function 
            0,                      // use default creation flags 
            &threadId);             // returns the thread identifier 

    return threadId;
}

// WaitForSingleObject
int                 SyncJoinThread(THREAD threadRef, void** returnValue)
{
    WaitForSingleObject(threadRef, INFINITE);
    CloseHandle(threadRef);

    return 0;
}

// InitializeCriticalSection
int                 SyncCreateMutex(THREAD_MUTEX *mutexRef, void* mutexAttr)
{
    InitializeCriticalSection(mutexRef);
    return 0;
}

// DeleteCriticalSection
int                 SyncDestroyMutex(THREAD_MUTEX *mutexRef)
{
    DeleteCriticalSection(mutexRef);
    return 0;
}

// EnterCriticalSection
int                 SyncLockMutex(THREAD_MUTEX *mutexRef)
{
    EnterCriticalSection(mutexRef);
    return 0;
}

// LeaveCriticalSection
int                 SyncUnlockMutex(THREAD_MUTEX *mutexRef)
{
    LeaveCriticalSection(mutexRef);
    return 0;
}

// InitializeConditionVariable
int                 SyncCreateCond(THREAD_COND *condRef, void* condAttr)
{
    InitializeConditionVariable(condRef);
    return 0;
}

// not defined for windows
int                 SyncDestroyCond(THREAD_COND *condRef)
{
    return 0;
}

// SleepConditionVariableCS
int                 SyncWaitCond(THREAD_COND *condRef, THREAD_MUTEX *mutexRef)
{
    SleepConditionVariableCS(condRef, mutexRef, INFINITE);
    return 0;
}

// WakeConditionVariable
int                 SyncSignalCond(THREAD_COND *condRef)
{
    WakeConditionVariable(condRef);
    return 0;
}


// WakeAllConditionVariable
int                 SyncBroadcastCond(THREAD_COND *condRef)
{
    WakeAllConditionVariable(condRef);
    return 0;
}

#else

/*---------------------------------------------------------------------------*/
/* POSIX FUNCTION DEFINITIONS */
/*---------------------------------------------------------------------------*/

// pthread_self
unsigned int        SyncGetThreadId()
{
    return (unsigned int)pthread_self();
}

// pthread_create
int                 SyncCreateThread(THREAD *threadRef, void *threadAttr, void *(*threadFunction)(void *), void *threadContext)
{
    return pthread_create(threadRef, threadAttr, threadFunction, threadContext);
}

// pthread_join
int                 SyncJoinThread(THREAD threadRef, void** returnValue)
{
    return pthread_join(threadRef, returnValue);
}

// pthread_mutex_init
int                 SyncCreateMutex(THREAD_MUTEX *mutexRef, void* mutexAttr)
{
    return pthread_mutex_init(mutexRef, mutexAttr);
}

// pthread_mutex_destroy
int                 SyncDestroyMutex(THREAD_MUTEX *mutexRef)
{
    return pthread_mutex_destroy(mutexRef);
}

// pthread_mutex_lock
int                 SyncLockMutex(THREAD_MUTEX *mutexRef)
{
    return pthread_mutex_lock(mutexRef);
}

// pthread_mutex_unlock
int                 SyncUnlockMutex(THREAD_MUTEX *mutexRef)
{
    return pthread_mutex_unlock(mutexRef);
}

// pthread_cond_init
int                 SyncCreateCond(THREAD_COND *condRef, void* condAttr)
{
    return pthread_cond_init(condRef, condAttr);
}

// pthread_cond_destroy
int                 SyncDestroyCond(THREAD_COND *condRef)
{
    return pthread_cond_destroy(condRef);
}

// pthread_cond_wait
int                 SyncWaitCond(THREAD_COND *condRef, THREAD_MUTEX *mutexRef)
{
    return pthread_cond_wait(condRef, mutexRef);
}

// pthread_cond_signal
int                 SyncSignalCond(THREAD_COND *condRef)
{
    return pthread_cond_signal(condRef);
}

// pthread_cond_broadcast
int                 SyncBroadcastCond(THREAD_COND *condRef)
{
    return pthread_cond_broadcast(condRef);
}

#endif 