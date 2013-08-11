#ifndef _TASK_QUEUE_H_
#define _TASK_QUEUE_H_

/*---------------------------------------------------------------------------*/
/* FILE INCLUSION */
/*---------------------------------------------------------------------------*/

#include "synchronize.h"

/*---------------------------------------------------------------------------*/
/* MACRO DEFINITIONS */
/*---------------------------------------------------------------------------*/

/* N/A */

/*---------------------------------------------------------------------------*/
/* ENUMERATIONS */
/*---------------------------------------------------------------------------*/

// success/fail of adding a task item to the queue
typedef enum TASK_QUEUE_STATUS_ENUM
{
    // item successfully added to queue
    TASK_QUEUE_STATUS_ADD_SUCCESS = 0,

    // item was not added to queue because it was at capacity
    TASK_QUEUE_STATUS_ADD_FULL_FAIL,

    // item was not added to queue because memory could not be allocated
    TASK_QUEUE_STATUS_ADD_MALLOC_FAIL

} TASK_QUEUE_STATUS;

/*---------------------------------------------------------------------------*/
/* TYPE DECLARATIONS */
/*---------------------------------------------------------------------------*/

// this structure is passed as the first argument to the work and callback functions
typedef struct TASK_QUEUE_WORK_DATA_STRUCT
{
    // queue id where work was performed
    unsigned int    queueId;

    // thread id where work was performed
    unsigned int    threadId;

    // task id of the work item
    unsigned int    taskId;

} TASK_QUEUE_WORK_DATA;

// use this structure to pass a work item to the queue
typedef struct TASK_QUEUE_ITEM_STRUCT
{
    // reference to work item function
    void*           (*taskItemFunction)(TASK_QUEUE_WORK_DATA *taskData, void *threadContext, void *workData);

    // reference to work item callback function
    void            (*taskItemCallback)(TASK_QUEUE_WORK_DATA *taskData, void *threadContext, void *callbackData);

    // reference to work item context (this will be passed to the work item function)
    void            *taskItemData;

    // size (bytes) of the work item context
    unsigned int    dataSize;

    // id of task
    unsigned int    taskId;

} TASK_QUEUE_ITEM;

// forward declaration to hide implementation
typedef struct TASK_QUEUE_STRUCT TASK_QUEUE;

// do not modify, use or interact with this structure or its members directly
typedef struct TASK_QUEUE_DATA_STRUCT 
{
    // this is the only member that should ever be accessed as read-only
    unsigned int                queueId;

    THREAD_COND                 *queueCond;
    THREAD_MUTEX                *queueMutex;
    TASK_QUEUE                  *taskQueue;

} TASK_QUEUE_DATA;

/*---------------------------------------------------------------------------*/
// FUNCTION PROTOTYPES
// These methods can be called from application code.
/*---------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

// should only be called during application startup
TASK_QUEUE_DATA*    CreateTaskQueue(unsigned int queueId);

// should only be called once per task queue and after destroying the associated thread pool
void                DestroyTaskQueue(TASK_QUEUE_DATA *taskQueueData);

// thread safe
TASK_QUEUE_STATUS   AddTaskToQueue(TASK_QUEUE_DATA *taskQueueData, TASK_QUEUE_ITEM *taskQueueItem);

// thread safe
void                FlushTaskQueue(TASK_QUEUE_DATA *taskQueueData);

#ifdef __cplusplus
}
#endif

/*---------------------------------------------------------------------------*/
/* OBJECT DECLARATIONS */
/*---------------------------------------------------------------------------*/

#ifndef _TASKE_QUEUE_C_

/* N/A */

#endif /* _TASKE_QUEUE_C_ */

/*****************************************************************************/

#endif /* _TASK_QUEUE_H_ */