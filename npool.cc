/*---------------------------------------------------------------------------*/
/* FILE INCLUSION */
/*---------------------------------------------------------------------------*/

// C
#include <stdlib.h>
#include <stdio.h>
//#include <unistd.h>

// C++
#include <iostream>
#include <string>
#include <sstream>

// memset(...)
#include <string.h>

// node and v8
#include <node.h>
#include <v8.h>
#include <nan.h>

// thread pool
#include "synchronize.h"
#include "task_queue.h"
#include "thread_pool.h"

// custom source
#include "thread.h"
#include "file_manager.h"

/*---------------------------------------------------------------------------*/
/* NAMESPACES */
/*---------------------------------------------------------------------------*/

// namespace
using namespace v8;
using namespace std;

/*---------------------------------------------------------------------------*/
/* MACRO DEFINITIONS */
/*---------------------------------------------------------------------------*/

#define TASK_QUEUE_ID       1

/*---------------------------------------------------------------------------*/
/* TYPE DECLARATIONS */
/*---------------------------------------------------------------------------*/

/* N/A */

/*---------------------------------------------------------------------------*/
/* FUNCTION PROTOTYPES */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* OBJECT DECLARATIONS */
/*---------------------------------------------------------------------------*/

// thread pool
static THREAD_POOL_DATA     *threadPool     = 0;
static TASK_QUEUE_DATA      *taskQueue      = 0;

// file loader and hash
static FileManager          *fileManager    = 0;

/*---------------------------------------------------------------------------*/
/* STATIC FUNCTION DEFINITIONS */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* FUNCTION DEFINITIONS */
/*---------------------------------------------------------------------------*/

NAN_METHOD(CreateThreadPool)
{
    //fprintf(stdout, "[%u] nPool - CreateThreadPool\n", SyncGetThreadId());

    Nan::HandleScope();

    // validate input
    if((info.Length() != 1) || !info[0]->IsNumber())
    {
        return Nan::ThrowError("createThreadPool() - Expects 1 arguments: 1) number of threads (uint32)");
    }

    // ensure thread pool has not already been created
    if((taskQueue != 0) || (threadPool != 0))
    {
        return Nan::ThrowError("createThreadPool() - Thread pool already created");
    }

    // number of threads
    Local<Uint32> v8NumThreads = (info[0])->ToUint32();
    uint32_t numThreads = v8NumThreads->Value();

    //fprintf(stdout, "[%u] nPool - Num Threads: %u\n", SyncGetThreadId(), numThreads);

    // create task queue and thread pool
    taskQueue = CreateTaskQueue(TASK_QUEUE_ID);
    threadPool = CreateThreadPool(numThreads, taskQueue, Thread::ThreadInit, Thread::ThreadPostInit, Thread::ThreadDestroy);

    info.GetReturnValue().SetUndefined();
}

NAN_METHOD(DestoryThreadPool)
{
    //fprintf(stdout, "[%u] nPool - DestoryThreadPool\n", SyncGetThreadId());

    Nan::HandleScope();

    // ensure thread pool has already been created
    if((threadPool == 0) || (taskQueue == 0))
    {
        return Nan::ThrowError("destroyThreadPool() - No thread pool exists to destroy");
    }

    // destroy thread pool and task queue
    DestroyThreadPool(threadPool);
    DestroyTaskQueue(taskQueue);

    // reset the references because they are no longer valid
    threadPool = 0;
    taskQueue = 0;

   info.GetReturnValue().SetUndefined();
}

NAN_METHOD(LoadFile)
{
    //fprintf(stdout, "[%u] nPool - LoadFile\n", SyncGetThreadId());

    Nan::HandleScope();

    // validate input
    if((info.Length() != 2) || !info[0]->IsNumber() || !info[1]->IsString())
    {
        return Nan::ThrowError("loadFile() - Expects 2 arguments: 1) file key (uint32) 2) file path (string)");
    }

    // file key
    uint32_t fileKey = (info[0])->ToUint32()->Value();
    Nan::Utf8String filePath(info[1]);

    // ensure file was loaded successfully
    LOAD_FILE_STATUS fileStatus = fileManager->LoadFile(fileKey, *filePath);
    if(fileStatus != LOAD_FILE_SUCCESS)
    {
        return Nan::ThrowError("loadFile() - Failed to load file. Check if file exists.");
    }

    info.GetReturnValue().SetUndefined();
}

NAN_METHOD(RemoveFile)
{
    //fprintf(stdout, "[%u] nPool - RemoveFile\n", SyncGetThreadId());

    Nan::HandleScope();

    // validate input
    if((info.Length() != 1) || !info[0]->IsNumber())
    {
        return Nan::ThrowError("loadFile() - Expects 1 argument: 1) file key (uint32)");
        info.GetReturnValue().SetUndefined();
    }

    // file key
    Local<Uint32> v8FileKey = (info[0])->ToUint32();
    uint32_t fileKey = v8FileKey->Value();

    fileManager->RemoveFile(fileKey);

    info.GetReturnValue().SetUndefined();
}

NAN_METHOD(QueueWork)
{
    //fprintf(stdout, "[%u] nPool - Work\n", SyncGetThreadId());

    Nan::HandleScope();

    // validate input
    if((info.Length() != 1) || !info[0]->IsObject())
    {
        return Nan::ThrowError("work() - Expects 1 argument: 1) work item (object)");
    }

    // get object from argument
    Handle<Value> v8Object = info[0];
    THREAD_WORK_ITEM* workItem = Thread::BuildWorkItem(v8Object->ToObject());

    if(workItem == NULL)
    {
        return Nan::ThrowError("queueWork() - Work item is malformed");
    }
    else
    {
        // queue the work
        Thread::QueueWorkItem(taskQueue, workItem);
    }

    info.GetReturnValue().SetUndefined();
}

/*---------------------------------------------------------------------------*/
/* NODE INITIALIZATION */
/*---------------------------------------------------------------------------*/

#if NODE_VERSION_AT_LEAST(0, 9, 0)
void Init(Local<Object> exports)
#else
void Init(Handle<Object> exports)
#endif
{

    // static object initialization

    fileManager = &(FileManager::GetInstance());

    // module initialization

    Nan::Export(exports, "createThreadPool",     CreateThreadPool);
    Nan::Export(exports, "destroyThreadPool",    DestoryThreadPool);
    Nan::Export(exports, "loadFile",             LoadFile);
    Nan::Export(exports, "removeFile",           RemoveFile);
    Nan::Export(exports, "queueWork",            QueueWork);
}

NODE_MODULE(npool, Init)
