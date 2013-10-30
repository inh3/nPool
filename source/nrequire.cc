#include "nrequire.h"

// C++
#include <string>

// C
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifndef _WIN32
#include <unistd.h>
#else
#include <io.h>
#endif

// Custom
#include "utilities.h"
#include "isolate_context.h"

Handle<Value> Require::RequireFunction(const Arguments& args)
{
	HandleScope scope;

	// validate input
    if((args.Length() != 1) || !args[0]->IsString())
    {
        return scope.Close(ThrowException(Exception::Error(String::New("Require::RequireFunction - Expects 1 arguments: 1) file name (string)"))));
    }

	// get filename string
	Local<String> v8FileName = (args[0])->ToString();
	String::AsciiValue fileName(v8FileName);

	// allocate file buffer
    int fileSize = 0;
    const char *fileBuffer = Utilities::ReadFile(*fileName, &fileSize);

    // file was invalid
    if(fileBuffer == 0)
    {
        std::string exceptionPrefix("Require::RequireFunction - File Name is invalid: ");
        std::string exceptionFileName(*fileName);
        std::string exceptionString = exceptionPrefix + exceptionFileName;
        return scope.Close(ThrowException(Exception::Error(String::New(exceptionString.c_str()))));
    }
    // file was read successfully
    else
    {
        // register external memory
        V8::AdjustAmountOfExternalAllocatedMemory(fileSize);

        // get reference to calling context
        Handle<Context> globalContext = Context::GetCalling();

        // create new module context
        Handle<Context> moduleContext = Context::New();

        // set the security token to access calling context properties within new context
        moduleContext->SetSecurityToken(globalContext->GetSecurityToken());

        // enter new context context scope
        Context::Scope moduleScope(moduleContext);

        // clone the calling context properties into this context
        IsolateContext::CloneGlobalContextObject(globalContext->Global(), moduleContext->Global());

        // get reference to current context's object
        Handle<Object> contextObject = moduleContext->Global();

        // create the module context
        IsolateContext::CreateModuleContext(contextObject);

        // process the source and execute it
        Handle<Value> scriptResult;
        {
            TryCatch scriptTryCatch;

            // compile the script
            Handle<String> sourceString = String::New(fileBuffer);
            Handle<Script> script = Script::Compile(sourceString);
            
            // throw exception if script failed to compile
            if(script.IsEmpty() || scriptTryCatch.HasCaught())
            {
                Utilities::HandleException(&scriptTryCatch, true);
                return scope.Close(Undefined());
            }
            
            //printf("[%u] Require::RequireFunction - Script Running: %s\n", SyncGetThreadId(), *fileName);
            scriptResult = script->Run();
            //printf("[%u] Require::RequireFunction - Script Completed: %s\n", SyncGetThreadId(), *fileName);

            // throw exception if script failed to execute
            if(scriptResult.IsEmpty() || scriptTryCatch.HasCaught())
            {
                Utilities::HandleException(&scriptTryCatch, true);
                return scope.Close(Undefined());
            }
        }

        // free the file buffer and de-register memory
        free((char *)fileBuffer);
        V8::AdjustAmountOfExternalAllocatedMemory(-fileSize);

        // print object properties
        //Utilities::PrintObjectProperties(contextObject);

        // return module export(s)
        Handle<Object> moduleObject = contextObject->Get(String::New("module"))->ToObject();
        return scope.Close(moduleObject->Get(String::New("exports")));
    }    
}