#ifndef _THREAD_H_
#define _THREAD_H_

// C++
#ifdef __APPLE__
#include <tr1/unordered_map>
using namespace std::tr1;
#else
#include <unordered_map>
using namespace std;
#endif

// node
#include <node.h>
#include <v8.h>
using namespace v8;

// threadpool
#include "synchronize.h"
#include "task_queue.h"
#include "thread_pool.h"

// thread module map
typedef unordered_map<uint32_t, Persistent<Object> > ThreadModuleMap;

typedef struct THREAD_CONTEXT_STRUCT
{
    // libuv
    uv_async_t*         uvAsync;

    // v8
    Isolate*            threadIsolate;
    Persistent<Context> threadJSContext;

    // thread module cache
    ThreadModuleMap*    moduleMap;

} THREAD_CONTEXT;

typedef struct THREAD_WORK_ITEM_STRUCT
{
    // work info and input object/function
    uint32_t                workId;
    uint32_t                fileKey;
    char*                   workFunction;
    char*                   workParam;

    // callback and output object/function
    Persistent<Object>      callbackContext;
    Persistent<Function>    callbackFunction;
    char*                   callbackObject;

    // indicates error
    bool                    isError;
    char*                   jsException;

} THREAD_WORK_ITEM;

class Thread
{
    public:
        
        static void*                ThreadInit();
        static void                 ThreadPostInit(void* threadContext);
        static void                 ThreadDestroy(void* threadContext);

        static THREAD_WORK_ITEM*    BuildWorkItem(Handle<Object> v8Object);
        static void                 QueueWorkItem(TASK_QUEUE_DATA *taskQueue, THREAD_WORK_ITEM *workItem);

    private:

        // work function and callback
        static void*            WorkItemFunction(TASK_QUEUE_WORK_DATA *taskData, void *threadContext, void *threadWorkItem);
        static void             WorkItemCallback(TASK_QUEUE_WORK_DATA *taskData, void *threadContext, void *threadWorkItem);

        // worker object
        static Handle<Object>   GetWorkerObject(THREAD_CONTEXT* thisContext, THREAD_WORK_ITEM* workItem);

        // uv callbacks
        static void             uvCloseCallback(uv_handle_t* handle);
        static void             uvAsyncCallback(uv_async_t* handle, int status);

        // memory disposal
        static void             DisposeWorkItem(THREAD_WORK_ITEM* workItem, bool freeWorkItem);
};

#endif /* _THREAD_H_ */