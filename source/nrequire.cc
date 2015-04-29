#include "nrequire.h"

// C++
#include <string>

// C
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Custom
#include "isolate_context.h"

NAN_METHOD(Require::RequireFunction)
{
    NanScope();

    // validate input
    if((args.Length() != 1) || !args[0]->IsString())
    {
        return NanThrowError("Require::RequireFunction - Expects 1 arguments: 1) file name (string)");
    }

    // get filename string
    NanUtf8String fileName(args[0]);

    // get handle to directory of current executing script
    #if NODE_VERSION_AT_LEAST(0, 12, 0)
    Handle<Object> currentContextObject = Isolate::GetCurrent()->GetCallingContext()->Global();
    #else
    Handle<Object> currentContextObject = NanGetCurrentContext()->GetCalling()->Global();
    #endif
    Handle<String> dirNameHandle = currentContextObject->Get(NanNew<String>("__dirname"))->ToString();
    NanUtf8String __dirname(dirNameHandle);

    // allocate file buffer
    const FILE_INFO* fileInfo = Utilities::GetFileInfo(*fileName, *__dirname);

    // file was invalid
    if(fileInfo->fileBuffer == 0)
    {
        std::string exceptionPrefix("Require::RequireFunction - File Name is invalid: ");
        std::string exceptionFileName(*fileName);
        std::string exceptionString = exceptionPrefix + exceptionFileName;
        return NanThrowError(exceptionString.c_str());
    }
    // file was read successfully
    else
    {
        // register external memory
        NanAdjustExternalMemory(fileInfo->fileBufferLength);

        // get reference to calling context
        Handle<Context> globalContext = NanGetCurrentContext();

        // create new module context
        Local<Context> moduleContext = NanNew<Context>();

        // set the security token to access calling context properties within new context
        moduleContext->SetSecurityToken(globalContext->GetSecurityToken());

        // clone the calling context properties into this context
        IsolateContext::CloneGlobalContextObject(globalContext->Global(), moduleContext->Global());

        // get reference to current context's object
        Handle<Object> contextObject = moduleContext->Global();

        // create the module context
        IsolateContext::CreateModuleContext(contextObject, fileInfo);

        // enter new context context scope
        Context::Scope context_scope(moduleContext);

        // process the source and execute it
        Handle<Value> scriptResult;
        {
            TryCatch scriptTryCatch;

            // compile the script
            ScriptOrigin scriptOrigin(NanNew<String>(fileInfo->fileName));
            #if NODE_VERSION_AT_LEAST(0, 11, 13)
            Handle<UnboundScript> moduleScript = NanNew<NanUnboundScript>(
                NanNew<String>(fileInfo->fileBuffer),
                scriptOrigin);
            #else
            Handle<Script> moduleScript = NanNew<Script>(
                NanNew<String>(fileInfo->fileBuffer),
                scriptOrigin);
            #endif

            // throw exception if script failed to compile
            if(moduleScript.IsEmpty() || scriptTryCatch.HasCaught())
            {
                Require::FreeFileInfo((FILE_INFO*)fileInfo);
                NanUtf8String* exceptionSerialized = Utilities::HandleException(&scriptTryCatch);
                std::string exceptionString(**exceptionSerialized);
                delete exceptionSerialized;
                return NanThrowError(exceptionString.c_str());
            }

            //printf("[%u] Require::RequireFunction - Script Running: %s\n", SyncGetThreadId(), *fileName);
            scriptResult = NanRunScript(moduleScript);
            //printf("[%u] Require::RequireFunction - Script Completed: %s\n", SyncGetThreadId(), *fileName);

            // throw exception if script failed to execute
            if(scriptResult.IsEmpty() || scriptTryCatch.HasCaught())
            {
                Require::FreeFileInfo((FILE_INFO*)fileInfo);
                NanUtf8String* exceptionSerialized = Utilities::HandleException(&scriptTryCatch);
                std::string exceptionString(**exceptionSerialized);
                delete exceptionSerialized;
                return NanThrowError(exceptionString.c_str());
            }
        }

        // print object properties
        //Utilities::PrintObjectProperties(contextObject);
        Require::FreeFileInfo((FILE_INFO*)fileInfo);

        // return module export(s)
        Handle<Object> moduleObject = contextObject->Get(NanNew<String>("module"))->ToObject();
        NanReturnValue(moduleObject->Get(NanNew<String>("exports")));
    }
}

void Require::FreeFileInfo(FILE_INFO* fileInfo) {
    // free the file buffer and de-register memory
    NanAdjustExternalMemory(-(fileInfo->fileBufferLength));
    Utilities::FreeFileInfo(fileInfo);
}
