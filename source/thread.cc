#include "thread.h"

// C
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// v8
#include <v8.h>
using namespace v8;

// custom source
#include "file_manager.h"
#include "json.h"
#include "utilities.h"
#include "callback_queue.h"

// file loader and hash (npool.cc)
static FileManager *fileManager = &(FileManager::GetInstance());

// callback queue
static CallbackQueue *callbackQueue = &(CallbackQueue::GetInstance());

typedef struct THREAD_CONTEXT_STRUCT
{
  uv_async_t*       uvAsync;
  Isolate*          threadIsolate;
  ThreadModuleMap*  moduleMap;
} THREAD_CONTEXT;

void* Thread::ThreadInit()
{
    // allocate memory for thread context
    THREAD_CONTEXT* threadContext= (THREAD_CONTEXT*)malloc(sizeof(THREAD_CONTEXT));
    memset(threadContext, 0, sizeof(THREAD_CONTEXT));

    // create and initialize async watcher
    threadContext->uvAsync = (uv_async_t*)malloc(sizeof(uv_async_t));
    memset(threadContext->uvAsync, 0, sizeof(uv_async_t));
    uv_async_init(uv_default_loop(), threadContext->uvAsync, Thread::uvAsyncCallback);
    threadContext->uvAsync->close_cb = Thread::uvCloseCallback;

    // create thread isolate
    threadContext->threadIsolate = Isolate::New();

    // create module map
    threadContext->moduleMap = new ThreadModuleMap();

    return (void*)threadContext;
}

void Thread::ThreadDestroy(void* threadContext)
{
    // thread context
    THREAD_CONTEXT* thisContext = (THREAD_CONTEXT*)threadContext;

    // Dispose of the isolate
    ((Isolate *)(thisContext->threadIsolate))->Dispose();

    // release the module map
    delete thisContext->moduleMap;

    //uv_unref((uv_handle_t*)thisContext->uvAsync);
    uv_close((uv_handle_t*)thisContext->uvAsync, ((uv_async_t*)thisContext->uvAsync)->close_cb);

    // release the thread context memory
    free(threadContext);
}

THREAD_WORK_ITEM* Thread::BuildWorkItem(Handle<Object> v8Object)
{
    //fprintf(stdout, "[%u] Thread::BuildWorkItem\n", SyncGetThreadId());
    //Utilities::ParseObject(v8Object);

    // return value
    THREAD_WORK_ITEM *workItem = (THREAD_WORK_ITEM*)malloc(sizeof(THREAD_WORK_ITEM));
    memset(workItem, 0, sizeof(THREAD_WORK_ITEM));

    // temporary handles
    Local<String> propertyName;
    Local<Value> propertyValue;
    Local<String> propertyString;
    Local<Object> propertyObject;

    // workId
    propertyName = String::New("workId");
    propertyValue = v8Object->Get(propertyName);
    workItem->workId = propertyValue->ToUint32()->Value();
    //fprintf(stdout, "[%u] Thread::BuildWorkItem - WorkId: %u\n", SyncGetThreadId(), propertyValue->ToUint32()->Value());

    // fileKey
    propertyName = String::New("fileKey");
    propertyValue = v8Object->Get(propertyName);
    workItem->fileKey = propertyValue->ToUint32()->Value();

    // workFunction
    propertyName = String::New("workFunction");
    propertyString = v8Object->Get(propertyName)->ToString();
    workItem->workFunction = Utilities::CreateCharBuffer(propertyString);

    // generate JSON c str of param object
    propertyName = String::New("workParam");
    propertyObject = v8Object->Get(propertyName)->ToObject();
    char* stringify = JSON::Stringify(propertyObject);
    workItem->workParam = stringify;

    // callback context
    propertyName = String::New("callbackContext");
    propertyObject = v8Object->Get(propertyName)->ToObject();
    workItem->callbackContext = Persistent<Object>::New(propertyObject);

    // callback function
    propertyName = String::New("callbackFunction");
    workItem->callbackFunction = Persistent<Function>::New(Handle<Function>::Cast(v8Object->Get(propertyName)));

    return workItem;
}

void Thread::QueueWorkItem(TASK_QUEUE_DATA *taskQueue, THREAD_WORK_ITEM *workItem)
{
    // reference to task queue item to be added
    TASK_QUEUE_ITEM     *taskQueueItem = 0;

    // create task queue item object
    taskQueueItem = (TASK_QUEUE_ITEM*)malloc(sizeof(TASK_QUEUE_ITEM));
    memset(taskQueueItem, 0, sizeof(TASK_QUEUE_ITEM));
    
    // set the data size
    taskQueueItem->dataSize = sizeof(THREAD_WORK_ITEM);

    // store reference to work item
    taskQueueItem->taskItemData = (void*)workItem;

    // set the task item work function
    taskQueueItem->taskItemFunction = Thread::WorkItemFunction;

    // set the task item callback function
    taskQueueItem->taskItemCallback = Thread::WorkItemCallback;

    // set the task item id
    taskQueueItem->taskId = workItem->workId;
    
    // add the task to the queue
    AddTaskToQueue(taskQueue, taskQueueItem);
}

void* Thread::WorkItemFunction(TASK_QUEUE_WORK_DATA *taskData, void *threadContext, void *threadWorkItem)
{
    //fprintf(stdout, "[%u] Thread::WorkItemFunction\n", SyncGetThreadId());

    // thread context
    THREAD_CONTEXT* thisContext = (THREAD_CONTEXT*)threadContext;

    // thread work item
    THREAD_WORK_ITEM* workItem = (THREAD_WORK_ITEM*)threadWorkItem;

    // check module cache
    const string* workFile = 0;
    if(thisContext->moduleMap->find(workItem->fileKey) == thisContext->moduleMap->end())
    {
        // get work file string
        workFile = fileManager->GetFileString(workItem->fileKey);
    }

    // get reference to thread isolate
    Isolate* isolate = thisContext->threadIsolate;
    {
        // Lock the isolate
        Locker myLocker(isolate);

        // Enter the isolate
        isolate->Enter();

        // Create a stack-allocated handle scope.
        HandleScope handle_scope;

        // Create a new context.
        Handle<Context> context = Context::New();

        // Enter the created context
        Context::Scope context_scope(context);

        // get the module string if necessary
        Handle<Object> workerObject;
        if(workFile != 0)
        {
            // compile the source code
            Handle<Script> script = Script::Compile(String::New(workFile->c_str()));
    
            // run the script to get the result
            Handle<Value> scriptResult = script->Run();

            // construct the worker object from constructor
            Handle<Function> constructorFunction = Handle<Function>::Cast(scriptResult);
            workerObject = constructorFunction->NewInstance();

            // cache the persistent object for later use
            thisContext->moduleMap->insert(make_pair(workItem->fileKey, Persistent<Object>::New(workerObject)));
        }
        else
        {
            // get the module from cache
            workerObject = thisContext->moduleMap->find(workItem->fileKey)->second;
        }

        // get work param
        Handle<Value> workParam = JSON::Parse(workItem->workParam);

        // get worker function name
        Handle<Value> workerFunction = workerObject->Get(String::New(workItem->workFunction));

        // execute function and get work result
        Handle<Value> workResult = Handle<Function>::Cast(workerFunction)->Call(workerObject, 1, &workParam);

        // strinigfy callback object
        char* stringify = JSON::Stringify(workResult->ToObject());
        workItem->callbackObject = stringify;
    }

    // leave the isolate
    isolate->Exit();

    // return the work item
    return workItem;
}

void Thread::WorkItemCallback(TASK_QUEUE_WORK_DATA *taskData, void *threadContext, void *threadWorkItem)
{
  //fprintf(stdout, "[%u] Thread::WorkItemCallback\n", SyncGetThreadId());

  // thread context
  THREAD_CONTEXT* thisContext = (THREAD_CONTEXT*)threadContext;

  // add work item to callback queue
  THREAD_WORK_ITEM* workItem = (THREAD_WORK_ITEM*)malloc(sizeof(THREAD_WORK_ITEM));
  memcpy(workItem, threadWorkItem, sizeof(THREAD_WORK_ITEM));
  callbackQueue->AddWorkItem(workItem);

  // async callback
  uv_async_t *uvAsync = (uv_async_t*)thisContext->uvAsync;
  uv_async_send(uvAsync);
}

void Thread::uvCloseCallback(uv_handle_t* handle)
{
  //fprintf(stdout, "[%u] Thread::uvCloseCallback - Async: %p\n", SyncGetThreadId(), handle);

  // cleanup the handle
  //free(handle->data);
  free(handle);
}

void Thread::uvAsyncCallback(uv_async_t* handle, int status)
{
    //fprintf(stdout, "[%u] Thread::uvAsyncCallback - Async: %p\n", SyncGetThreadId(), handle);

    HandleScope scope;

    // process all work items awaiting callback
    THREAD_WORK_ITEM* workItem = 0;
    while((workItem = callbackQueue->GetWorkItem()) != 0)
    {
        // get references to work item callback data
        Persistent<Object> callbackContext = workItem->callbackContext;
        Persistent<Function> callbackFunction = workItem->callbackFunction;
        Handle<Value> callbackObject = JSON::Parse(workItem->callbackObject);

        //create arguments array
        const unsigned argc = 2;
        Handle<Value> argv[argc] = { callbackObject, Number::New(workItem->workId) };

        //fprintf(stdout, "[%u] Thread::uvAsyncCallback - Call", SyncGetThreadId());

        // make callback on node thread
        callbackFunction->Call(callbackContext, argc, argv);

        //fprintf(stdout, "[%u] Thread::uvAsyncCallback - Clearing data...\n", SyncGetThreadId());

        // cleanup the work item data
        callbackContext.Dispose();
        callbackContext.Clear();

        callbackFunction.Dispose();
        callbackFunction.Clear();

        free(workItem->workFunction);
        free(workItem->workParam);
        free(workItem->callbackObject);
        free(workItem);
    }
}