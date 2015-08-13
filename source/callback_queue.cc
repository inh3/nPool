#include "callback_queue.h"

// C
//#include <stdio.h>
#include <stdlib.h>

// public instance "constructor"
CallbackQueue& CallbackQueue::GetInstance()
{
    // lazy instantiation of class instance
    static CallbackQueue classInstance;

    // return by reference
    return classInstance;
}

// protected constructor
CallbackQueue::CallbackQueue()
{
    // create queue
    callbackQueue = new std::queue<THREAD_WORK_ITEM*>();

    // create file map mutex
    SyncCreateMutex(&(this->queueMutex), 0);
}

// destructor
CallbackQueue::~CallbackQueue()
{
    SyncLockMutex(&(this->queueMutex));
    delete this->callbackQueue;
    SyncUnlockMutex(&(this->queueMutex));

    SyncDestroyMutex(&(this->queueMutex));
}

void CallbackQueue::AddWorkItem(THREAD_WORK_ITEM* workItem)
{
    SyncLockMutex(&(this->queueMutex));

    this->callbackQueue->push(workItem);

    SyncUnlockMutex(&(this->queueMutex));
}

THREAD_WORK_ITEM* CallbackQueue::GetWorkItem()
{
    //fprintf(stdout, "CallbackQueue::GetWorkItem\n");

    THREAD_WORK_ITEM* returnValue = 0;

    SyncLockMutex(&(this->queueMutex));

    if(!(this->callbackQueue->empty()))
    {
        returnValue = this->callbackQueue->front();

        this->callbackQueue->pop();
    }

    SyncUnlockMutex(&(this->queueMutex));

    return returnValue;
}
