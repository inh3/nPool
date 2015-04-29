#include "isolate_context.h"

// C
#include "string.h"

// Custom
#include "nrequire.h"
#include "json_utility.h"

static NAN_METHOD(ConsoleLog)
{
    NanScope();

    // validate input
    if(args.Length() > 1)
    {
        return NanThrowTypeError("console.log - Expects only 1 argument.");
    }

    // get log message
    if(args.Length() == 0)
    {
        printf("\n");
    }
    else
    {
        NanUtf8String* utf8String = JsonUtility::Stringify(args[0]);
        printf("%s\n", **utf8String);
        delete utf8String;
    }

    NanReturnUndefined();
}

void IsolateContext::CreateGlobalContext(Handle<Object> globalContext)
{
    NanScope();

    // global namespace object
    globalContext->Set(NanNew<String>("global"), NanNew<Object>());

    // require(...)

    // get handle to nRequire function
    Local<FunctionTemplate> functionTemplate = NanNew<FunctionTemplate>(Require::RequireFunction);
    Local<Function> requireFunction = functionTemplate->GetFunction();
    requireFunction->SetName(NanNew<String>("require"));

    // attach function to context
    globalContext->Set(NanNew<String>("require"), requireFunction);

    // console.log(...)

    // setup console object
    Handle<Object> consoleObject = NanNew<Object>();

    // get handle to log function
    Local<FunctionTemplate> logTemplate = NanNew<FunctionTemplate>(ConsoleLog);
    Local<Function> logFunction = logTemplate->GetFunction();
    logFunction->SetName(NanNew<String>("log"));

    // attach log function to console object
    consoleObject->Set(NanNew<String>("log"), logFunction);

    // attach object to context
    globalContext->Set(NanNew<String>("console"), consoleObject);

}

void IsolateContext::UpdateContextFileProperties(Handle<Object> contextObject, const FILE_INFO* fileInfo)
{
    NanScope();

    // set the file properites on the context
    contextObject->Set(NanNew<String>("__dirname"), NanNew<String>(fileInfo->folderPath));
    contextObject->Set(NanNew<String>("__filename"), NanNew<String>(fileInfo->fullPath));
}

void IsolateContext::CloneGlobalContextObject(Handle<Object> sourceObject, Handle<Object> cloneObject)
{
    NanScope();

    // copy global properties
    cloneObject->Set(NanNew<String>("global"), sourceObject->Get(NanNew<String>("global")));
    cloneObject->Set(NanNew<String>("require"), sourceObject->Get(NanNew<String>("require")));
    cloneObject->Set(NanNew<String>("console"), sourceObject->Get(NanNew<String>("console")));
}

void IsolateContext::CreateModuleContext(Handle<Object> contextObject, const FILE_INFO* fileInfo)
{
    NanScope();

    // create the module/exports within context
    Handle<Object> moduleObject = NanNew<Object>();
    moduleObject->Set(NanNew<String>("exports"), NanNew<Object>());
    contextObject->Set(NanNew<String>("module"), moduleObject);
    contextObject->Set(NanNew<String>("exports"), moduleObject->Get(NanNew<String>("exports"))->ToObject());

    // copy file properties
    if(fileInfo != NULL)
    {
        IsolateContext::UpdateContextFileProperties(contextObject, fileInfo);
    }
}
