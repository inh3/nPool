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
    Nan::HandleScope scope;

    // validate input
    if((info.Length() != 1) || !info[0]->IsString())
    {
        return Nan::ThrowError("Require::RequireFunction - Expects 1 arguments: 1) file name (string)");
    }

    // get filename string
    Nan::Utf8String fileName(info[0]);

    // get handle to directory of current executing script
    #if NODE_VERSION_AT_LEAST(0, 12, 0)
    Local<Object> currentContextObject = Isolate::GetCurrent()->GetCallingContext()->Global();
    #else
    Local<Object> currentContextObject = Nan::GetCurrentContext()->GetCalling()->Global();
    #endif
    Local<String> dirNameHandle = Nan::To<String>(
        Nan::Get(currentContextObject, Nan::New<String>("__dirname").ToLocalChecked()).ToLocalChecked()).ToLocalChecked();
    Nan::Utf8String __dirname(dirNameHandle);

    // allocate file buffer
    const FILE_INFO* fileInfo = Utilities::GetFileInfo(*fileName, *__dirname);

    // file was invalid
    if(fileInfo->fileBuffer == 0)
    {
        std::string exceptionPrefix("Require::RequireFunction - File Name is invalid: ");
        std::string exceptionFileName(*fileName);
        std::string exceptionString = exceptionPrefix + exceptionFileName;
        return Nan::ThrowError(exceptionString.c_str());
    }
    // file was read successfully
    else
    {
        // register external memory
        Nan::AdjustExternalMemory(fileInfo->fileBufferLength);

        // get reference to calling context
        Local<Context> globalContext = Nan::GetCurrentContext();

        // create new module context
        Local<Context> moduleContext = Nan::New<Context>();

        // set the security token to access calling context properties within new context
        moduleContext->SetSecurityToken(globalContext->GetSecurityToken());

        // clone the calling context properties into this context
        IsolateContext::CloneGlobalContextObject(globalContext->Global(), moduleContext->Global());

        // get reference to current context's object
        Local<Object> contextObject = moduleContext->Global();

        // create the module context
        IsolateContext::CreateModuleContext(contextObject, fileInfo);

        // enter new context context scope
        Context::Scope context_scope(moduleContext);

        // process the source and execute it
        Nan::MaybeLocal<Value> scriptResult;
        {
            TryCatch scriptTryCatch;

            // compile the script
            ScriptOrigin scriptOrigin(Nan::New<String>(fileInfo->fileName).ToLocalChecked());
            Nan::MaybeLocal<Nan::BoundScript> moduleScript = Nan::CompileScript(
                Nan::New<String>(fileInfo->fileBuffer).ToLocalChecked(),
                scriptOrigin);

            // throw exception if script failed to compile
            if(moduleScript.IsEmpty() || scriptTryCatch.HasCaught())
            {
                Require::FreeFileInfo((FILE_INFO*)fileInfo);
                scriptTryCatch.ReThrow();
                info.GetReturnValue().SetUndefined();
                return;
            }

            //printf("[%u] Require::RequireFunction - Script Running: %s\n", SyncGetThreadId(), *fileName);
            scriptResult = Nan::RunScript(moduleScript.ToLocalChecked());
            //printf("[%u] Require::RequireFunction - Script Completed: %s\n", SyncGetThreadId(), *fileName);

            // throw exception if script failed to execute
            if(scriptResult.IsEmpty() || scriptTryCatch.HasCaught())
            {
                Require::FreeFileInfo((FILE_INFO*)fileInfo);
                scriptTryCatch.ReThrow();
                info.GetReturnValue().SetUndefined();
                return;
            }
        }

        // print object properties
        //Utilities::PrintObjectProperties(contextObject);
        Require::FreeFileInfo((FILE_INFO*)fileInfo);

        // return module export(s)
        Local<Object> moduleObject = Nan::To<Object>(
            Nan::Get(contextObject, Nan::New<String>("module").ToLocalChecked()).ToLocalChecked()).ToLocalChecked();
        info.GetReturnValue().Set(
            Nan::Get(moduleObject, Nan::New<String>("exports").ToLocalChecked()).ToLocalChecked());
    }
}

void Require::FreeFileInfo(FILE_INFO* fileInfo) {
    // free the file buffer and de-register memory
    Nan::AdjustExternalMemory(-(fileInfo->fileBufferLength));
    Utilities::FreeFileInfo(fileInfo);
}
