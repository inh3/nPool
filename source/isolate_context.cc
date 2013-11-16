#include "isolate_context.h"

// C
#include "string.h"

// Custom
#include "nrequire.h"

static Handle<Value> ConsoleLog(const Arguments& args)
{
    HandleScope scope;

    // validate input
    if((args.Length() != 1) || !args[0]->IsString())
    {
        return scope.Close(ThrowException(Exception::TypeError(String::New("console.log - Expects 1 argument: log message (string)"))));
    }

    // get log message
    String::AsciiValue logMessage((args[0])->ToString());
    fprintf(stdout, *logMessage);
    fprintf(stdout, "\n");

    return scope.Close(Undefined());
}

void IsolateContext::CreateGlobalContext(Handle<Object> globalContext)
{
    HandleScope handleScope;

    // require(...)

    // get handle to nRequire function
    Local<FunctionTemplate> functionTemplate = FunctionTemplate::New(Require::RequireFunction);
    Local<Function> requireFunction = functionTemplate->GetFunction();
    requireFunction->SetName(String::NewSymbol("require"));

    // attach function to context
    globalContext->Set(String::NewSymbol("require"), requireFunction);

    // console.log(...)

    // setup console object
    Handle<Object> consoleObject = Object::New();

    // get handle to log function
    Local<FunctionTemplate> logTemplate = FunctionTemplate::New(ConsoleLog);
    Local<Function> logFunction = logTemplate->GetFunction();
    logFunction->SetName(String::NewSymbol("log"));

    // attach log function to console object
    consoleObject->Set(String::NewSymbol("log"), logFunction);

    // attach object to context
    globalContext->Set(String::NewSymbol("console"), consoleObject);
}

void IsolateContext::UpdateGlobalContextDirName(Handle<Object> globalContext, const FILE_INFO* fileInfo)
{
    HandleScope handleScope;
    
    // copy global properties
    globalContext->Set(String::NewSymbol("__dirname"), String::New(fileInfo->fullPath, strlen(fileInfo->fullPath) + 1));
}

void IsolateContext::CloneGlobalContextObject(Handle<Object> sourceObject, Handle<Object> cloneObject)
{
    HandleScope handleScope;
    
    // copy global properties
    cloneObject->Set(String::NewSymbol("require"), sourceObject->Get(String::NewSymbol("require")));
    cloneObject->Set(String::NewSymbol("console"), sourceObject->Get(String::NewSymbol("console")));
}

void IsolateContext::CreateModuleContext(Handle<Object> contextObject, const FILE_INFO* fileInfo)
{
    HandleScope handleScope;

    // create the module/exports within context
    Handle<Object> moduleObject = Object::New();
    moduleObject->Set(String::NewSymbol("exports"), Object::New());
    contextObject->Set(String::NewSymbol("module"), moduleObject);
    contextObject->Set(String::NewSymbol("exports"), moduleObject->Get(String::NewSymbol("exports"))->ToObject());

    // copy file properties
    if(fileInfo != NULL)
    {
        contextObject->Set(String::NewSymbol("__dirname"), String::New(fileInfo->fullPath, strlen(fileInfo->fullPath) + 1));
    }
}