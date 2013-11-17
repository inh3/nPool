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

    // register external memory
    int bytesAlloc = strlen(workItem->workParam) + strlen(workItem->workFunction);
    V8::AdjustAmountOfExternalAllocatedMemory(bytesAlloc);

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
                Utilities::HandleException(&tryCatch, false);
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
        // work executed successfully
        if(workItem->isError == false)
        {
            // parse stringified result
            Handle<Value> callbackObject = JSON::Parse(workItem->callbackObject);

            //create arguments array
            const unsigned argc = 2;
            Handle<Value> argv[argc] = { callbackObject, Number::New(workItem->workId) };

            // make callback on node thread
            workItem->callbackFunction->Call(workItem->callbackContext, argc, argv);

            // clean up memory and dispose of persistent references
            Thread::DisposeWorkItem(workItem, true);
        }
        // work failed to get executed
        else
        {
            Thread::DisposeWorkItem(workItem, true);
        }
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
        Handle<Script> script = Script::Compile(String::New(workFileInfo->fileBuffer));

        // check for exception on compile
        if(script.IsEmpty() || tryCatch.HasCaught())
        {
            Utilities::HandleException(&tryCatch, false);
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
                Utilities::HandleException(&tryCatch, false);
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
    if(freeWorkItem == true)
    {
        free(workItem);
    }

    // wait for gc to do its business (250/1000 "work")
    while(!V8::IdleNotification(250)) {
        //fprintf(stdout, "[%u] Thread::uvAsyncCallback - Notifying Idle...\n", SyncGetThreadId());
    }
}