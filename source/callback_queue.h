#ifndef _CALLBACK_QUEUE_H_
#define _CALLBACK_QUEUE_H_

// C++
#include <queue>

// custom source
#include "thread.h"
#include "synchronize.h"

class CallbackQueue
{
    public:

        // singleton instance of class
        static CallbackQueue&   GetInstance();

        // destructor
        virtual                 ~CallbackQueue();

        // load work item into queue
        void                    AddWorkItem(THREAD_WORK_ITEM* workItem);

        // get work item from queue and remove it
        THREAD_WORK_ITEM*       GetWorkItem();

    protected:

        // ensure default constructor can't get called
        CallbackQueue();

        // declare private copy constructor methods to ensure they can't be called
        CallbackQueue(CallbackQueue const&);
        void operator=(CallbackQueue const&);

    private:

        std::queue<THREAD_WORK_ITEM*>    *callbackQueue;
        THREAD_MUTEX                queueMutex;
};

#endif /* _CALLBACK_QUEUE_H_ */
