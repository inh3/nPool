#include "thread.h"

// C
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// custom source
#include "file_manager.h"
#include "json_utility.h"
#include "utilities.h"
#include "callback_queue.h"
#include "isolate_context.h"

#include "array_buffer_allocator.h"

// file loader and hash (npool.cc)
static FileManager *fileManager = &(FileManager::GetInstance());

// callback queue
static CallbackQueue *callbackQueue = &(CallbackQueue::GetInstance());

// array buffer allocator
// node version 4 requires array buffer allocator for isolates
#if NODE_MAJOR_VERSION >= 4
    static ArrayBufferAllocator arrayBufferAllocator;
#endif

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
    // node version 4 requires array buffer allocator for isolates
    #if NODE_MAJOR_VERSION >= 4
        Isolate::CreateParams create_params;
        create_params.array_buffer_allocator = &arrayBufferAllocator;
        threadContext->threadIsolate = Isolate::New(create_params);
    #else
        threadContext->threadIsolate = Isolate::New();
    #endif

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
        Nan::HandleScope scope;

        // store reference to a new persistent context
        Local<Context> isolateContext = Nan::New<Context>();
        thisContext->threadJSContext = new Nan::Persistent<Context>(isolateContext);

        // enter thread specific context
        isolateContext->Enter();

        // create global context
        Local<Object> globalContext = Nan::GetCurrentContext()->Global();
        IsolateContext::CreateGlobalContext(globalContext);

        // create module context
        IsolateContext::CreateModuleContext(globalContext, NULL);

        // exit thread specific context
        isolateContext->Exit();
    }

    // leave the isolate
    isolate->Exit();
}

void Thread::ThreadDestroy(void* threadContext)
{
    //fprintf(stdout, "[%u] Thread::ThreadDestroy\n", SyncGetThreadId());

    // thread context
    THREAD_CONTEXT* thisContext = (THREAD_CONTEXT*)threadContext;

    // get reference to thread isolate
    Isolate* isolate = thisContext->threadIsolate;
    {
        // lock the isolate
        Locker myLocker(isolate);

        // enter the isolate
        isolate->Enter();

        // clean-up the worker modules
        for(ThreadModuleMap::iterator it = thisContext->moduleMap->begin(); it != thisContext->moduleMap->end(); ++it)
        {
            Nan::Persistent<Object>* pObject = it->second;
            pObject->Reset();
            delete pObject;
        }
        thisContext->moduleMap->clear();

        // dispose of js context
        thisContext->threadJSContext->Reset();
        delete thisContext->threadJSContext;
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

THREAD_WORK_ITEM* Thread::BuildWorkItem(Local<Object> v8Object)
{
    // work item to be returned
    THREAD_WORK_ITEM *workItem = NULL;

    // exception catcher
    TryCatch tryCatch;

    // get all the properties from the object
    Local<String> propertyName = Nan::New<String>("workId").ToLocalChecked();
    Nan::MaybeLocal<Number> workId = Nan::To<Number>(Nan::Get(v8Object, propertyName).ToLocalChecked());

    propertyName = Nan::New<String>("fileKey").ToLocalChecked();
    Nan::MaybeLocal<Number> fileKey = Nan::To<Number>(Nan::Get(v8Object, propertyName).ToLocalChecked());

    propertyName = Nan::New<String>("workFunction").ToLocalChecked();
    Nan::MaybeLocal<String> workFunction = Nan::To<String>(Nan::Get(v8Object, propertyName).ToLocalChecked());

    propertyName = Nan::New<String>("workParam").ToLocalChecked();
    Nan::MaybeLocal<Object> workParam = Nan::To<Object>(Nan::Get(v8Object, propertyName).ToLocalChecked());

    propertyName = Nan::New<String>("callbackContext").ToLocalChecked();
    Nan::MaybeLocal<Object> callbackContext = Nan::To<Object>(Nan::Get(v8Object, propertyName).ToLocalChecked());

    propertyName = Nan::New<String>("callbackFunction").ToLocalChecked();
    Nan::MaybeLocal<Value> callbackFunction = Nan::Get(v8Object, propertyName);

    // determine if the object is valid
    bool isInvalidWorkObject = (workId.IsEmpty() ||
                                fileKey.IsEmpty() ||
                                workFunction.IsEmpty() ||
                                workParam.IsEmpty() ||
                                callbackContext.IsEmpty() ||
                                callbackFunction.IsEmpty());

    // ensure there weren't any exceptions and properties were valid
    if((isInvalidWorkObject == false) || !(tryCatch.HasCaught()))
    {
        // return value
        workItem = (THREAD_WORK_ITEM*)malloc(sizeof(THREAD_WORK_ITEM));
        memset(workItem, 0, sizeof(THREAD_WORK_ITEM));

        // workId
        workItem->workId = workId.ToLocalChecked()->Value();

        // fileKey
        workItem->fileKey = fileKey.ToLocalChecked()->Value();

        // workFunction
        workItem->workFunction = Utilities::CreateCharBuffer(workFunction.ToLocalChecked());

        // generate JSON c str of param object
        workItem->workParam = JsonUtility::Stringify(workParam.ToLocalChecked());

        // callback context
        workItem->callbackContext = new Nan::Persistent<Object>(callbackContext.ToLocalChecked());

        // callback function
        workItem->callbackFunction = new Nan::Callback(callbackFunction.ToLocalChecked().As<Function>());

        // register external memory
        int bytesAlloc = (workItem->workParam->length() + 1) + strlen(workItem->workFunction);
        Nan::AdjustExternalMemory(bytesAlloc);
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
        Nan::HandleScope scope;

        // enter thread specific context
        Local<Context> isolateContext = Nan::New<Context>(*(thisContext->threadJSContext));
        isolateContext->Enter();

        // exception catcher
        TryCatch tryCatch;

        // get worker object
        Local<Object> workerObject = Thread::GetWorkerObject(thisContext, workItem);

        // no errors getting the worker object
        if(workItem->isError == false)
        {
            // get work param
            Local<Value> workParam = JsonUtility::Parse(**(workItem->workParam));

            // get worker function name
            Local<Value> workerFunction = Nan::Get(workerObject, Nan::New<String>(workItem->workFunction).ToLocalChecked()).ToLocalChecked();

            // execute function and get work result
            Local<Value> workResult = workerFunction.As<Function>()->Call(workerObject, 1, &workParam);

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
                workItem->callbackObject = JsonUtility::Stringify(workResult->ToObject());
                workItem->isError = false;

                // register external memory
                Nan::AdjustExternalMemory(workItem->callbackObject->length() + 1);
            }
        }

        // exit thread specific context
        isolateContext->Exit();
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

#if NODE_VERSION_AT_LEAST(0, 11, 13)
void Thread::uvAsyncCallback(uv_async_t* handle)
#else
void Thread::uvAsyncCallback(uv_async_t* handle, int status)
#endif
{
    //fprintf(stdout, "[%u] Thread::uvAsyncCallback - Async: %p\n", SyncGetThreadId(), handle);

    Nan::HandleScope scope;

    // process all work items awaiting callback
    THREAD_WORK_ITEM* workItem = 0;
    while((workItem = callbackQueue->GetWorkItem()) != 0)
    {
        Local<Value> callbackObject = Nan::Null();
        Local<Value> exceptionObject = Nan::Null();

        // parse exception if one is present
        if(workItem->isError == true)
        {
            exceptionObject = JsonUtility::Parse(**(workItem->jsException));
        }
        else
        {
            // parse stringified result
            callbackObject = JsonUtility::Parse(**(workItem->callbackObject));
        }

        //create arguments array
        const unsigned argc = 3;
        Local<Value> argv[argc] = {
            callbackObject,
            Nan::New<Number>(workItem->workId),
            exceptionObject
        };

        // make callback on node thread
        Nan::MakeCallback(
            Nan::New<Object>(*(workItem->callbackContext)),
            workItem->callbackFunction->GetFunction(),
            argc,
            argv);

        // clean up memory and dispose of persistent references
        Thread::DisposeWorkItem(workItem, true);
    }
}

Local<Object> Thread::GetWorkerObject(THREAD_CONTEXT* thisContext, THREAD_WORK_ITEM* workItem)
{
    Nan::EscapableHandleScope scope;

    // return variable and exception
    Local<Object> workerObject;
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
        Local<Object> globalContext = Nan::New<Context>(*(thisContext->threadJSContext))->Global();
        IsolateContext::UpdateContextFileProperties(globalContext, workFileInfo);

        // compile the script
        ScriptOrigin scriptOrigin(Nan::New<String>(workFileInfo->fileName).ToLocalChecked());
        Nan::MaybeLocal<Nan::BoundScript> fileScript = Nan::CompileScript(
            Nan::New<String>(workFileInfo->fileBuffer).ToLocalChecked(),
            scriptOrigin);

        // check for exception on compile
        if(fileScript.IsEmpty() || tryCatch.HasCaught())
        {
            workItem->jsException = Utilities::HandleException(&tryCatch, true);
            workItem->isError = true;
        }
        // no exception
        else
        {
            // run the script to get the result
            Nan::MaybeLocal<Value> scriptResult = Nan::RunScript(fileScript.ToLocalChecked());

            // throw exception if script failed to run properly
            if(scriptResult.IsEmpty() || tryCatch.HasCaught())
            {
                workItem->jsException = Utilities::HandleException(&tryCatch, true);
                workItem->isError = true;
            }
            else
            {
                 // create object template in order to use object wrap
                Local<ObjectTemplate> objectTemplate = Nan::New<ObjectTemplate>();
                objectTemplate->SetInternalFieldCount(1);
                workerObject = objectTemplate->NewInstance();

                // copy the script result to the worker object
                Utilities::CopyObject(
                    workerObject,
                    scriptResult.ToLocalChecked().As<Function>()->NewInstance());

                // cache the persistent object type for later use
                // wrap the object so it can be persisted
                Nan::Persistent<Object>* pObject = new Nan::Persistent<Object>(workerObject);
                thisContext->moduleMap->insert(std::make_pair(
                    workItem->fileKey,
                    pObject));
            }
        }
    }
    // get the worker object from the cache
    else
    {
        // get the cached object instance
        workerObject = Nan::New<Object>(*(thisContext->moduleMap->find(workItem->fileKey)->second));
    }

    return scope.Escape(workerObject);
}

void Thread::DisposeWorkItem(THREAD_WORK_ITEM* workItem, bool freeWorkItem)
{
    // cleanup the work item data
    workItem->callbackContext->Reset();
    delete workItem->callbackContext;
    delete workItem->callbackFunction;

    // de-register memory
    int bytesToFree = (workItem->workParam->length() + 1) + strlen(workItem->workFunction);
    if(workItem->callbackObject != NULL)
    {
        bytesToFree += (workItem->callbackObject->length() + 1);
    }
    Nan::AdjustExternalMemory(-bytesToFree);

    // un-alloc the memory
    free(workItem->workFunction);
    delete workItem->workParam;
    if(workItem->callbackObject != NULL)
    {
        delete workItem->callbackObject;
    }
    if(workItem->isError == true)
    {
        delete workItem->jsException;
    }
    if(freeWorkItem == true)
    {
        free(workItem);
    }
}
