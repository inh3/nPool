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
static THREAD_POOL_DATA     *threadPool = 0;
static TASK_QUEUE_DATA      *taskQueue = 0;

// file loader and hash
static FileManager          *fileManager = 0;

/*---------------------------------------------------------------------------*/
/* STATIC FUNCTION DEFINITIONS */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* FUNCTION DEFINITIONS */
/*---------------------------------------------------------------------------*/

Handle<Value> CreateThreadPool(const Arguments& args)
{
    fprintf(stdout, "[%u] nPool - CreateThreadPool\n", SyncGetThreadId());

    HandleScope scope;

    // validate input
    if((args.Length() != 1) || !args[0]->IsNumber())
    {
        ThrowException(Exception::TypeError(String::New("createThreadPool() - Expects 1 arguments: 1) number of threads (uint32)")));
        return scope.Close(Undefined());
    }

    if(taskQueue != 0)
    {
        ThrowException(Exception::TypeError(String::New("createThreadPool() - Thread pool already created")));
        return scope.Close(Undefined());
    }

    // number of threads
    Local<Uint32> v8NumThreads = (args[0])->ToUint32();
    uint32_t numThreads = v8NumThreads->Value();

    fprintf(stdout, "[%u] nPool - Num Threads: %u\n", SyncGetThreadId(), numThreads);

    // create task queue and thread pool
    taskQueue = CreateTaskQueue(TASK_QUEUE_ID);
    threadPool = CreateThreadPool(numThreads, taskQueue, Thread::ThreadInit, Thread::ThreadPostInit, Thread::ThreadDestroy);

    return scope.Close(Undefined());
}

Handle<Value> DestoryThreadPool(const Arguments& args)
{
    fprintf(stdout, "[%u] nPool - DestoryThreadPool\n", SyncGetThreadId());

    HandleScope scope;

    // destroy thread pool and task queue
    DestroyThreadPool(threadPool);
    DestroyTaskQueue(taskQueue);

    // reset the references because they are no longer valid
    threadPool = 0;
    taskQueue = 0;

    return scope.Close(Undefined());
}

Handle<Value> LoadFile(const Arguments& args)
{
    fprintf(stdout, "[%u] nPool - LoadFile\n", SyncGetThreadId());

    HandleScope scope;

    // validate input
    if((args.Length() != 2) || !args[0]->IsNumber() || !args[1]->IsString())
    {
        ThrowException(Exception::TypeError(String::New("loadFile() - Expects 2 arguments: 1) file key (uint32) 2) file path (string).")));
        return scope.Close(Undefined());
    }

    // file key
    Local<Uint32> v8FileKey = (args[0])->ToUint32();
    uint32_t fileKey = v8FileKey->Value();

    Local<String> v8FilePath = Local<String>::Cast(args[1]);
    String::AsciiValue filePath(v8FilePath);

    fileManager->LoadFile(fileKey, *filePath);

    return scope.Close(Undefined());
}

Handle<Value> RemoveFile(const Arguments& args)
{
    fprintf(stdout, "[%u] nPool - RemoveFile\n", SyncGetThreadId());

    HandleScope scope;

    // validate input
    if((args.Length() != 1) || !args[0]->IsNumber())
    {
        ThrowException(Exception::TypeError(String::New("loadFile() - Expects 1 argument: 1) file key (uint32)")));
        return scope.Close(Undefined());
    }

    // file key
    Local<Uint32> v8FileKey = (args[0])->ToUint32();
    uint32_t fileKey = v8FileKey->Value();

    fileManager->RemoveFile(fileKey);

    return scope.Close(Undefined());
}

Handle<Value> QueueWork(const Arguments& args)
{
    //fprintf(stdout, "[%u] nPool - Work\n", SyncGetThreadId());

    HandleScope scope;

    // validate input
    if((args.Length() != 1) || !args[0]->IsObject())
    {
        ThrowException(Exception::TypeError(String::New("work() - Expects 1 argument: 1) work item (object)")));
        return scope.Close(Undefined());
    }

    // get object from argument
    Handle<Value> v8Object = args[0];
    THREAD_WORK_ITEM* workItem = Thread::BuildWorkItem(v8Object->ToObject());

    // queue the work
    Thread::QueueWorkItem(taskQueue, workItem);

    return scope.Close(Undefined());
}

/*---------------------------------------------------------------------------*/
/* NODE INITIALIZATION */
/*---------------------------------------------------------------------------*/

void init(Handle<Object> exports)
{
    // static object initialization

    fileManager = &(FileManager::GetInstance());

    // module initialization

    exports->Set(String::NewSymbol("createThreadPool"),
      FunctionTemplate::New(CreateThreadPool)->GetFunction());

    exports->Set(String::NewSymbol("destroyThreadPool"),
      FunctionTemplate::New(DestoryThreadPool)->GetFunction());

    exports->Set(String::NewSymbol("loadFile"),
      FunctionTemplate::New(LoadFile)->GetFunction());

    exports->Set(String::NewSymbol("removeFile"),
      FunctionTemplate::New(RemoveFile)->GetFunction());

    exports->Set(String::NewSymbol("queueWork"),
      FunctionTemplate::New(QueueWork)->GetFunction());
}

NODE_MODULE(npool, init)