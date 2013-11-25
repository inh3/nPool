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
#include "isolate_context.h"

// file loader and hash (npool.cc)
static FileManager *fileManager = &(FileManager::GetInstance());

// callback queue
static CallbackQueue *callbackQueue = &(CallbackQueue::GetInstance());

void* Thread::ThreadInit()
{
    // allocate memory for thread context
    THREAD_CONTEXT* threadContext = (THREAD_CONTEXT*)malloc(sizeof(THREAD_CONTEXT));
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

void Thread::ThreadPostInit(void* threadContext)
{
    // thread context
    THREAD_CONTEXT* thisContext = (THREAD_CONTEXT*)threadContext;

    // get reference to thread isolate
    Isolate* isolate = thisContext->threadIsolate;
    {
        // lock the isolate
        Locker myLocker(isolate);

        // enter the isolate
        isolate->Enter();

        // create a stack-allocated handle-scope
        HandleScope handle_scope;

        // create a persistent context for the javascript in this thread
        Persistent<Context> persistentContext(Context::New());

        // store reference to persistent context
        thisContext->threadJSContext = persistentContext;

        // enter thread specific context
        Context::Scope context_scope(thisContext->threadJSContext);

        // create global context
        Handle<Object> globalContext = thisContext->threadJSContext->Global();
        IsolateContext::CreateGlobalContext(globalContext);

        // create module context
        IsolateContext::CreateModuleContext(globalContext, NULL);
    }

    // leave the isolate
    isolate->Exit();
}

void Thread::ThreadDestroy(void* threadContext)
{
    // thread context
    THREAD_CONTEXT* thisContext = (THREAD_CONTEXT*)threadContext;

    // get reference to thread isolate
    Isolate* isolate = thisContext->threadIsolate;
    {
        // lock the isolate
        Locker myLocker(isolate);

        // enter the isolate
        isolate->Enter();

        // dispose of js context
        thisContext->threadJSContext.Dispose();
    }

    // exit the isolate
    isolate->Exit();

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
    // work item to be returned
    THREAD_WORK_ITEM *workItem = NULL;

    // exception catcher
    TryCatch tryCatch;

    // get all the properties from the object
    Local<Number> workId = v8Object->Get(String::NewSymbol("workId"))->ToUint32();
    Local<Number> fileKey = v8Object->Get(String::NewSymbol("fileKey"))->ToUint32();
    Local<String> workFunction = v8Object->Get(String::NewSymbol("workFunction"))->ToString();
    Local<Object> workParam = v8Object->Get(String::NewSymbol("workParam"))->ToObject();
    Local<Object> callbackContext = v8Object->Get(String::NewSymbol("callbackContext"))->ToObject();
    Handle<Function> callbackFunction = Handle<Function>::Cast(v8Object->Get(String::NewSymbol("callbackFunction")));

    // determine if the object is valid
    bool isInvalidWorkObject = (workId.IsEmpty() || fileKey.IsEmpty() ||
                                workFunction.IsEmpty() || workParam.IsEmpty() ||
                                callbackContext.IsEmpty() || callbackFunction.IsEmpty());

    // ensure there weren't any exceptions and properties were valid
    if((isInvalidWorkObject == false) || !(tryCatch.HasCaught()))
    {
        // return value
        workItem = (THREAD_WORK_ITEM*)malloc(sizeof(THREAD_WORK_ITEM));
        memset(workItem, 0, sizeof(THREAD_WORK_ITEM));

        // workId
        workItem->workId = workId->Value();

        // fileKey
        workItem->fileKey = fileKey->Value();

        // workFunction
        workItem->workFunction = Utilities::CreateCharBuffer(workFunction);

        // generate JSON c str of param object
        char* stringify = JSON::Stringify(workParam);
        workItem->workParam = stringify;

        // callback context
        workItem->callbackContext = Persistent<Object>::New(callbackContext);

        // callback function
        workItem->callbackFunction = Persistent<Function>::New(callbackFunction);

        // register external memory
        int bytesAlloc = strlen(workItem->workParam) + strlen(workItem->workFunction);
        V8::AdjustAmountOfExternalAllocatedMemory(bytesAlloc);
    }

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

    // get reference to thread isolate
    Isolate* isolate = thisContext->threadIsolate;
    {
        // Lock the isolate
        Locker myLocker(isolate);

        // Enter the isolate
        isolate->Enter();

        // Create a stack-allocated handle scope.
        HandleScope handle_scope;

        // enter thread specific context
        Context::Scope context_scope(thisContext->threadJSContext);

        // exception catcher
        TryCatch tryCatch;

        // get worker object
        Handle<Object> workerObject = Thread::GetWorkerObject(thisContext, workItem);

        // no errors getting the worker object
        if(workItem->isError == false)
        {
            // get work param
            Handle<Value> workParam = JSON::Parse(workItem->workParam);

            // get worker function name
            Handle<Value> workerFunction = workerObject->Get(String::New(workItem->workFunction));

            // execute function and get work result
            Handle<Value> workResult = Handle<Function>::Cast(workerFunction)->Call(workerObject, 1, &workParam);
            
            // work failed to perform successfully
            if(workResult.IsEmpty() || tryCatch.HasCaught())
            {
                workItem->jsException = Utilities::HandleException(&tryCatch, true);
                workItem->isError = true;
            }
            // work performed successfully
            else
            {
                // strinigfy callback object
                char* stringify = JSON::Stringify(workResult->ToObject());
                workItem->callbackObject = stringify;
                workItem->isError = false;

                // register external memory
                V8::AdjustAmountOfExternalAllocatedMemory(strlen(workItem->callbackObject));
            }
        }
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
        Handle<Value> callbackObject = Null();
        Handle<Value> exceptionObject = Null();

        // parse exception if one is present
        if(workItem->isError == true)
        {
            exceptionObject = JSON::Parse(workItem->jsException);
        }
        else
        {
            // parse stringified result
            callbackObject = JSON::Parse(workItem->callbackObject);
        }

        //create arguments array
        const unsigned argc = 3;
        Handle<Value> argv[argc] = { callbackObject, Number::New(workItem->workId), exceptionObject };

        // make callback on node thread
        workItem->callbackFunction->Call(workItem->callbackContext, argc, argv);

        // clean up memory and dispose of persistent references
        Thread::DisposeWorkItem(workItem, true);
    }
}

Handle<Object> Thread::GetWorkerObject(THREAD_CONTEXT* thisContext, THREAD_WORK_ITEM* workItem)
{
    HandleScope scope;

    // return variable and exception
    Handle<Object> workerObject;
    TryCatch tryCatch;

    // check module cache
    const FILE_INFO* workFileInfo = 0;
    if(thisContext->moduleMap->find(workItem->fileKey) == thisContext->moduleMap->end())
    {
        // get work file string
        workFileInfo = fileManager->GetFileInfo(workItem->fileKey);
    }

    // compile the object script if necessary
    if(workFileInfo != 0)
    {
        // update the context for file properties of work file
        Handle<Object> globalContext = thisContext->threadJSContext->Global();
        IsolateContext::UpdateContextFileProperties(globalContext, workFileInfo);

        // compile the source code
        ScriptOrigin scriptOrigin(String::New(workFileInfo->fileName));
        Handle<Script> script = Script::Compile(String::New(workFileInfo->fileBuffer), &scriptOrigin);

        // check for exception on compile
        if(script.IsEmpty() || tryCatch.HasCaught())
        {
            workItem->jsException = Utilities::HandleException(&tryCatch, true);
            workItem->isError = true;
        }
        // no exception
        else
        {
            // run the script to get the result
            Handle<Value> scriptResult = script->Run();

            // throw exception if script failed to run properly
            if(scriptResult.IsEmpty() || tryCatch.HasCaught())
            {
                workItem->jsException = Utilities::HandleException(&tryCatch, true);
                workItem->isError = true;
            }
            else
            {
                // get the handle to constructor function of worker object type
                Handle<Function> constructorFunction = Handle<Function>::Cast(scriptResult);
                workerObject = constructorFunction->NewInstance();

                // cache the persistent object type for later use
                thisContext->moduleMap->insert(make_pair(workItem->fileKey, Persistent<Object>::New(workerObject)));
            }
        }
    }
    // get the worker object from the cache
    else
    {
        // get the constructor function from cache
        workerObject = thisContext->moduleMap->find(workItem->fileKey)->second;
    }

    return scope.Close(workerObject);
}

void Thread::DisposeWorkItem(THREAD_WORK_ITEM* workItem, bool freeWorkItem)
{
    // cleanup the work item data
    workItem->callbackContext.Dispose();
    workItem->callbackContext.Clear();

    workItem->callbackFunction.Dispose();
    workItem->callbackFunction.Clear();

    // de-register memory
    int bytesToFree = strlen(workItem->workParam) + strlen(workItem->workFunction);
    if(workItem->callbackObject != NULL)
    {
        bytesToFree += strlen(workItem->callbackObject);
    }
    V8::AdjustAmountOfExternalAllocatedMemory(-bytesToFree);

    // un-alloc the memory
    free(workItem->workFunction);
    free(workItem->workParam);
    if(workItem->callbackObject != NULL)
    {
        free(workItem->callbackObject);
    }
    if(workItem->isError == true)
    {
        free(workItem->jsException);
    }
    if(freeWorkItem == true)
    {
        free(workItem);
    }

    // wait for gc to do its business (250/1000 "work")
    /*while(!V8::IdleNotification(250)) {
        //fprintf(stdout, "[%u] Thread::uvAsyncCallback - Notifying Idle...\n", SyncGetThreadId());
    }*/
}