#define _THREAD_POOL_C_

/*---------------------------------------------------------------------------*/
/* FILE INCLUSION */
/*---------------------------------------------------------------------------*/

#include "thread_pool.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "synchronize.h"

/*---------------------------------------------------------------------------*/
/* MACRO DEFINITIONS */
/*---------------------------------------------------------------------------*/

/* N/A */

/*---------------------------------------------------------------------------*/
/* TYPE DECLARATIONS */
/*---------------------------------------------------------------------------*/

// context per thread
typedef struct THREAD_DATA_STRUCT
{
    // task queue and thread information
    TASK_QUEUE_WORK_DATA    *taskQueueWorkData;

    // reference to thread's pool terminate signal
    unsigned int            *terminateThread;

    // reference to the task queue of the thread pool
    TASK_QUEUE_DATA         *taskQueueData;

    // reference to the thread context
    void                    *context;

    // reference to initialize method
    void*                   (*initialize)();

    // reference to post-init method
    void                    (*postInit)(void *context);

    // reference to destroy method
    void                    (*destroy)(void* context);

} THREAD_DATA;

// context per thread pool
struct THREAD_POOL_DATA_STRUCT
{
    // stores number of threads within pool
    unsigned int        numThreads;

    // array of pthread references
    THREAD              *threadIds;

    // thread pool's terminate signal
    unsigned int        terminateThread;

    // reference to the task queue of the pool
    TASK_QUEUE_DATA     *taskQueueData;

}; /* THREAD_POOL_DATA */

/*---------------------------------------------------------------------------*/
/* FUNCTION PROTOTYPES */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
// TASK QUEUE FUNCTION PROTOTYPES
// The thread pool uses these methods to interact with the task queue.
/*---------------------------------------------------------------------------*/

extern TASK_QUEUE_ITEM*    GetTaskQueueItem(TASK_QUEUE_DATA *taskQueueData);
extern int                 GetQueueLength(TASK_QUEUE_DATA *taskQueueData);

/*---------------------------------------------------------------------------*/
/* OBJECT DECLARATIONS */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* STATIC FUNCTION DEFINITIONS */
/*---------------------------------------------------------------------------*/

// individual thread function
static THREAD_FUNC threadFunction(void *threadArg)
{
    // task item function return data reference
    void* taskData = 0;

    // local reference to task queue item to be worked
    TASK_QUEUE_ITEM *taskQueueItem = 0;

    // store thread context data locally and update thread id
    THREAD_DATA *threadData = (THREAD_DATA*)threadArg;
    threadData->taskQueueWorkData->threadId = SyncGetThreadId();

    // execute post initialize if set
    if(threadData->postInit != NULL)
    {
        //fprintf(stdout, "[%u] threadFunction - postInit\n", SyncGetThreadId());
        threadData->postInit(threadData->context);
    }

    // continue until termination signaled
    while(!*(threadData->terminateThread))
    {
        // lock the queue before checking if there is work to be done
        SyncLockMutex(threadData->taskQueueData->queueMutex);

        // continue while not terminating AND no work (needed for spurious wake-ups)
        while(!*(threadData->terminateThread) && (GetQueueLength(threadData->taskQueueData) == 0))
        {
            //printf("Thread [%u] is waiting....\n", (unsigned int)threadData->taskQueueWorkData->threadId);
            SyncWaitCond(threadData->taskQueueData->queueCond, threadData->taskQueueData->queueMutex);
        }

        // thread is not suppose to terminate
        if(!*(threadData->terminateThread))
        {
            // get the work item from the queue
            taskQueueItem = GetTaskQueueItem(threadData->taskQueueData);
        }

        // unlock the queue
        SyncUnlockMutex(threadData->taskQueueData->queueMutex);

        // check one more time if work should actually be done AND if the task queue item is valid
        if((taskQueueItem != 0) && !*(threadData->terminateThread))
        {
            // store the task id
            threadData->taskQueueWorkData->taskId = taskQueueItem->taskId;

            // execute the task and get the return value
            taskData = taskQueueItem->taskItemFunction(threadData->taskQueueWorkData, threadData->context, taskQueueItem->taskItemData);

            // execute callback if present
            if(taskQueueItem->taskItemCallback != 0)
            {
                taskQueueItem->taskItemCallback(threadData->taskQueueWorkData, threadData->context, taskData);
            }

            // release the task item data memory
            free(taskQueueItem->taskItemData);
            taskQueueItem->taskItemData = 0;

            // release the task item memory
            free(taskQueueItem);
            taskQueueItem = 0;
        }
    }

    // call destroy if set
    if(threadData->destroy != NULL)
    {
        threadData->destroy(threadData->context);
    }

    //printf("Thread [%u] is done!\n", (unsigned int)threadData->taskQueueWorkData->threadId);

    // free up thread data
    free(threadData->taskQueueWorkData);
    free(threadData);

#ifdef _WIN32
    // http://msdn.microsoft.com/en-us/library/kdzttdcb(v=vs.110).aspx
    _endthreadex(0);
#endif

    return THREAD_FUNC_RETURN;
}

/*---------------------------------------------------------------------------*/
/* FUNCTION DEFINITIONS */
/*---------------------------------------------------------------------------*/

THREAD_POOL_DATA* CreateThreadPool(
    unsigned int numThreads,
    TASK_QUEUE_DATA *taskQueueData,
    void* (*threadInit)(void),
    void (*threadPostInit)(void* threadContext),
    void (*threadDestory)(void* threadContext))
{
    // loop variable
    unsigned int i = 0;

    // thread context reference
    THREAD_DATA *threadData = 0;

    // create thread pool
    THREAD_POOL_DATA *threadPool = (THREAD_POOL_DATA*)malloc(sizeof(THREAD_POOL_DATA));
    memset(threadPool, 0, sizeof(THREAD_POOL_DATA));

    // store number of threads
    threadPool->numThreads = numThreads;

    // store task queue reference
    threadPool->taskQueueData = taskQueueData;

    // allocate memory for thread ids
    threadPool->threadIds = (THREAD*)malloc(numThreads * sizeof(THREAD));

    // create each thread
    for(i = 0; i < numThreads; i++)
    {
        // create thread context
        threadData = (THREAD_DATA*)malloc(sizeof(THREAD_DATA));
        memset(threadData, 0, sizeof(THREAD_DATA));

        // create task queue work data
        threadData->taskQueueWorkData = (TASK_QUEUE_WORK_DATA*)malloc(sizeof(TASK_QUEUE_WORK_DATA));
        memset(threadData->taskQueueWorkData, 0, sizeof(TASK_QUEUE_WORK_DATA));
        threadData->taskQueueWorkData->queueId = threadPool->taskQueueData->queueId;

        // store reference to termination signal and task queue
        threadData->terminateThread = &(threadPool->terminateThread);
        threadData->taskQueueData   = threadPool->taskQueueData;

        // set init and destroy functions if valid
        if(threadInit != NULL)
        {
            threadData->initialize = threadInit;
            threadData->context = threadData->initialize();
        }
        if(threadPostInit != NULL)
        {
            threadData->postInit = threadPostInit;
        }
        if(threadDestory != NULL)
        {
            threadData->destroy = threadDestory;
        }

        // create the thread
        SyncCreateThread(&(threadPool->threadIds[i]), NULL, threadFunction, threadData);
        //printf("Created Thread: %u (Queue: %u)\n", (unsigned int)threadPool->threadIds[i], threadData->taskQueueWorkData->queueId);
    }

    return threadPool;
}

void DestroyThreadPool(THREAD_POOL_DATA *threadPool)
{
    // loop variable
    unsigned int i = 0;

    // broadcast thread termination
    SyncLockMutex(threadPool->taskQueueData->queueMutex);
    threadPool->terminateThread = 1;
    SyncBroadcastCond(threadPool->taskQueueData->queueCond);
    SyncUnlockMutex(threadPool->taskQueueData->queueMutex);

    // wait for each thread to finish
    for(i = 0; i < threadPool->numThreads; i++)
    {
        SyncJoinThread(threadPool->threadIds[i], NULL);
    }

    // free up thread pool memory
    free(threadPool->threadIds);
    free(threadPool);
}