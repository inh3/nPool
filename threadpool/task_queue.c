#define _TASK_QUEUE_C_

/*---------------------------------------------------------------------------*/
/* FILE INCLUSION */
/*---------------------------------------------------------------------------*/

#include "task_queue.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*---------------------------------------------------------------------------*/
/* MACRO DEFINITIONS */
/*---------------------------------------------------------------------------*/

/* N/A */

/*---------------------------------------------------------------------------*/
/* TYPE DECLARATIONS */
/*---------------------------------------------------------------------------*/

// node within a task queue
typedef struct TASK_QUEUE_NODE_STRUCT
{
    // reference to task queue item
    TASK_QUEUE_ITEM                 *taskQueueItem;

    // node priority (TODO: use in future for prioritization)
    unsigned int                    nodePriority;

    // reference to next node in queue
    struct TASK_QUEUE_NODE_STRUCT   *nextNode;

} TASK_QUEUE_NODE;

// definition of a task queue
struct TASK_QUEUE_STRUCT
{
    // reference to first item in queue
    TASK_QUEUE_NODE     *queueHead;

    // reference to last item in queue
    TASK_QUEUE_NODE     *queueTail;

    // number of nodes within the queue
    int                 queueLength;

    // synchronization mechanisms
    THREAD_COND         queueCond;
    THREAD_MUTEX        queueMutex;

}; /* TASK_QUEUE */

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

static TASK_QUEUE_NODE* CreateQueueNode(TASK_QUEUE_ITEM *taskQueueItem)
{
    TASK_QUEUE_NODE *queueNode = 0;

    // create new node
    queueNode = (TASK_QUEUE_NODE*)malloc(sizeof(TASK_QUEUE_NODE));
    if(queueNode == 0)
    {
        return 0;
    }
    memset(queueNode, 0, sizeof(TASK_QUEUE_NODE));

    // update node with task
    queueNode->taskQueueItem = taskQueueItem;

    return queueNode;
}

static void FreeQueueNode(TASK_QUEUE_NODE *queueNode)
{
    // release the task item data memory
    free(queueNode->taskQueueItem->taskItemData);
    queueNode->taskQueueItem->taskItemData = 0;

    // release the task item memory
    free(queueNode->taskQueueItem);
    queueNode->taskQueueItem = 0;

    // release the node
    free(queueNode);
    queueNode = 0;
}

static TASK_QUEUE_STATUS AddTaskToQueueInternal(TASK_QUEUE_DATA *taskQueueData, TASK_QUEUE_ITEM *taskQueueItem)
{
    // return value (assume success)
    TASK_QUEUE_STATUS   addStatus = TASK_QUEUE_STATUS_ADD_SUCCESS;

    // node to be added to queue
    TASK_QUEUE_NODE     *queueNode = 0;

    // create queue node
    queueNode = CreateQueueNode(taskQueueItem);

    // node creation failed
    if(queueNode == 0)
    {
        // mark as malloc fail
        addStatus = TASK_QUEUE_STATUS_ADD_MALLOC_FAIL;
    }
    // add node to queue
    else
    {
        // queue is empty
        if(taskQueueData->taskQueue->queueLength == 0)
        {
            taskQueueData->taskQueue->queueHead = queueNode;
            taskQueueData->taskQueue->queueTail = queueNode;
        }
        // add node to end of queue
        else
        {
            taskQueueData->taskQueue->queueTail->nextNode = queueNode;
            taskQueueData->taskQueue->queueTail = queueNode;
        }

        // update queue length
        taskQueueData->taskQueue->queueLength++;
    }

    return addStatus;
}

static void DestroyTaskQueueInternal(TASK_QUEUE_DATA *taskQueueData)
{
    // node to be deleted
    TASK_QUEUE_NODE *queueNode = 0;

    // delete each node sequentially
    while(taskQueueData->taskQueue->queueLength > 0)
    {
        // get reference of node to be removed
        queueNode = taskQueueData->taskQueue->queueHead;

        // update head node
        taskQueueData->taskQueue->queueHead = queueNode->nextNode;
        
        // release queue node memory
        FreeQueueNode(queueNode);

        // decrement length of the queue
        taskQueueData->taskQueue->queueLength--;
    }
}

/*---------------------------------------------------------------------------*/
/* FUNCTION DEFINITIONS */
/*---------------------------------------------------------------------------*/

TASK_QUEUE_DATA* CreateTaskQueue(unsigned int queueId)
{
    // define task queue data
    TASK_QUEUE_DATA *taskQueueData = (TASK_QUEUE_DATA*)malloc(sizeof(TASK_QUEUE_DATA));

    // define task queue
    TASK_QUEUE *taskQueue = (TASK_QUEUE*)malloc(sizeof(TASK_QUEUE));

    // initialize the task queue data
    memset(taskQueueData, 0, sizeof(TASK_QUEUE_DATA));

    // initialize task queue
    memset(taskQueue, 0, sizeof(TASK_QUEUE));

    // initialize the queue mutex and cond
    SyncCreateMutex(&(taskQueue->queueMutex), NULL);
    SyncCreateCond(&(taskQueue->queueCond), NULL);

    // set task queue data references
    taskQueueData->taskQueue = taskQueue;
    taskQueueData->queueCond = &(taskQueue->queueCond);
    taskQueueData->queueMutex = &(taskQueue->queueMutex);

    // store the queue id
    taskQueueData->queueId = queueId;

    return taskQueueData;
}

void DestroyTaskQueue(TASK_QUEUE_DATA *taskQueueData)
{
    // flush the queue
    FlushTaskQueue(taskQueueData);

    // destroy the queue mutex and conditional
    SyncDestroyCond(taskQueueData->queueCond);
    SyncDestroyMutex(taskQueueData->queueMutex);

    // release task queue memory
    free(taskQueueData->taskQueue);
    free(taskQueueData);
}

TASK_QUEUE_STATUS AddTaskToQueue(TASK_QUEUE_DATA *taskQueueData, TASK_QUEUE_ITEM *taskQueueItem)
{
    // return value
    TASK_QUEUE_STATUS   addStatus = TASK_QUEUE_STATUS_ADD_SUCCESS;

    // lock access to the queue
    SyncLockMutex(taskQueueData->queueMutex);

    // add task item to queue
    addStatus = AddTaskToQueueInternal(taskQueueData, taskQueueItem);

    // signal update to queue
    SyncSignalCond(taskQueueData->queueCond);

    // unlock access to the queue
    SyncUnlockMutex(taskQueueData->queueMutex);

    return addStatus;
}

void FlushTaskQueue(TASK_QUEUE_DATA *taskQueueData)
{
    // lock access to the queue
    SyncLockMutex(taskQueueData->queueMutex);

    // destroy task queue
    DestroyTaskQueueInternal(taskQueueData);

    // unlock access to the queue
    SyncUnlockMutex(taskQueueData->queueMutex);
}

TASK_QUEUE_ITEM* GetTaskQueueItem(TASK_QUEUE_DATA *taskQueueData)
{
    // node to be processed
    TASK_QUEUE_NODE *queueNode = 0;

    // task queue item to be returned
    TASK_QUEUE_ITEM *taskQueueItem = 0;

    if(taskQueueData->taskQueue->queueLength > 0)
    {
        // get node at front of queue
        queueNode = taskQueueData->taskQueue->queueHead;

        // copy task queue item to return
        taskQueueItem = (TASK_QUEUE_ITEM*)malloc(sizeof(TASK_QUEUE_ITEM));
        memcpy(taskQueueItem, queueNode->taskQueueItem, sizeof(TASK_QUEUE_ITEM));
        taskQueueItem->taskItemData = malloc(queueNode->taskQueueItem->dataSize);
        memcpy(taskQueueItem->taskItemData, queueNode->taskQueueItem->taskItemData, queueNode->taskQueueItem->dataSize);

        // update head node
        taskQueueData->taskQueue->queueHead = queueNode->nextNode;

        // release queue node memory
        FreeQueueNode(queueNode);

        // decrement length of the queue
        taskQueueData->taskQueue->queueLength--;
    }

    return taskQueueItem;
}

int GetQueueLength(TASK_QUEUE_DATA *taskQueueData)
{
    return taskQueueData->taskQueue->queueLength;
}