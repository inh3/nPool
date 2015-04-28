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

    NanScope();

    // validate input
    if((args.Length() != 1) || !args[0]->IsNumber())
    {
        return NanThrowError("createThreadPool() - Expects 1 arguments: 1) number of threads (uint32)");
    }

    // ensure thread pool has not already been created
    if((taskQueue != 0) || (threadPool != 0))
    {
        return NanThrowError("createThreadPool() - Thread pool already created");
    }

    // number of threads
    Local<Uint32> v8NumThreads = (args[0])->ToUint32();
    uint32_t numThreads = v8NumThreads->Value();

    //fprintf(stdout, "[%u] nPool - Num Threads: %u\n", SyncGetThreadId(), numThreads);

    // create task queue and thread pool
    taskQueue = CreateTaskQueue(TASK_QUEUE_ID);
    threadPool = CreateThreadPool(numThreads, taskQueue, Thread::ThreadInit, Thread::ThreadPostInit, Thread::ThreadDestroy);

    NanReturnUndefined();
}

NAN_METHOD(DestoryThreadPool)
{
    //fprintf(stdout, "[%u] nPool - DestoryThreadPool\n", SyncGetThreadId());

    NanScope();

    // ensure thread pool has already been created
    if((threadPool == 0) || (taskQueue == 0))
    {
        return NanThrowError("destroyThreadPool() - No thread pool exists to destroy");
    }

    // destroy thread pool and task queue
    DestroyThreadPool(threadPool);
    DestroyTaskQueue(taskQueue);

    // reset the references because they are no longer valid
    threadPool = 0;
    taskQueue = 0;

   NanReturnUndefined();
}

NAN_METHOD(LoadFile)
{
    //fprintf(stdout, "[%u] nPool - LoadFile\n", SyncGetThreadId());

    NanScope();

    // validate input
    if((args.Length() != 2) || !args[0]->IsNumber() || !args[1]->IsString())
    {
        return NanThrowError("loadFile() - Expects 2 arguments: 1) file key (uint32) 2) file path (string)");
    }

    // file key
    uint32_t fileKey = (args[0])->ToUint32()->Value();
    NanUtf8String filePath(args[1]);

    // ensure file was loaded successfully
    LOAD_FILE_STATUS fileStatus = fileManager->LoadFile(fileKey, *filePath);
    if(fileStatus != LOAD_FILE_SUCCESS)
    {
        return NanThrowError("loadFile() - Failed to load file. Check if file exists.");
    }

    NanReturnUndefined();
}

NAN_METHOD(RemoveFile)
{
    //fprintf(stdout, "[%u] nPool - RemoveFile\n", SyncGetThreadId());

    NanScope();

    // validate input
    if((args.Length() != 1) || !args[0]->IsNumber())
    {
        return NanThrowError("loadFile() - Expects 1 argument: 1) file key (uint32)");
        NanReturnUndefined();
    }

    // file key
    Local<Uint32> v8FileKey = (args[0])->ToUint32();
    uint32_t fileKey = v8FileKey->Value();

    fileManager->RemoveFile(fileKey);

    NanReturnUndefined();
}

NAN_METHOD(QueueWork)
{
    //fprintf(stdout, "[%u] nPool - Work\n", SyncGetThreadId());

    NanScope();

    // validate input
    if((args.Length() != 1) || !args[0]->IsObject())
    {
        return NanThrowError("work() - Expects 1 argument: 1) work item (object)");
    }

    // get object from argument
    Handle<Value> v8Object = args[0];
    THREAD_WORK_ITEM* workItem = Thread::BuildWorkItem(v8Object->ToObject());

    if(workItem == NULL)
    {
        return NanThrowError("queueWork() - Work item is malformed");
    }
    else
    {
        // queue the work
        Thread::QueueWorkItem(taskQueue, workItem);
    }

    NanReturnUndefined();
}

/*---------------------------------------------------------------------------*/
/* NODE INITIALIZATION */
/*---------------------------------------------------------------------------*/

void init(Handle<Object> exports)
{
    // static object initialization

    fileManager = &(FileManager::GetInstance());

    // module initialization

    exports->Set(NanNew<String>("createThreadPool"),
      NanNew<FunctionTemplate>(CreateThreadPool)->GetFunction());

    exports->Set(NanNew<String>("destroyThreadPool"),
      NanNew<FunctionTemplate>(DestoryThreadPool)->GetFunction());

    exports->Set(NanNew<String>("loadFile"),
      NanNew<FunctionTemplate>(LoadFile)->GetFunction());

    exports->Set(NanNew<String>("removeFile"),
      NanNew<FunctionTemplate>(RemoveFile)->GetFunction());

    exports->Set(NanNew<String>("queueWork"),
      NanNew<FunctionTemplate>(QueueWork)->GetFunction());
}

NODE_MODULE(npool, init)
