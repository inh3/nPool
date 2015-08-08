#include "isolate_context.h"

// C
#include "string.h"

// Custom
#include "nrequire.h"
#include "json_utility.h"

static NAN_METHOD(ConsoleLog)
{
    Nan::HandleScope scope;

    // validate input
    if(info.Length() > 1)
    {
        return Nan::ThrowTypeError("console.log - Expects only 1 argument.");
    }

    // get log message
    if(info.Length() == 0)
    {
        printf("\n");
    }
    else
    {
        Nan::Utf8String* utf8String = JsonUtility::Stringify(info[0]);
        printf("%s\n", **utf8String);
        delete utf8String;
    }
}

void IsolateContext::CreateGlobalContext(Local<Object> globalContext)
{
    Nan::HandleScope scope;

    // global namespace object
    Nan::Set(globalContext, Nan::New<String>("global").ToLocalChecked(), Nan::New<Object>());

    // require(...)

    // get handle to nRequire function
    Local<FunctionTemplate> functionTemplate = Nan::New<FunctionTemplate>(Require::RequireFunction);
    Local<Function> requireFunction = functionTemplate->GetFunction();
    requireFunction->SetName(Nan::New<String>("require").ToLocalChecked());

    // attach function to context
    Nan::Set(globalContext, Nan::New<String>("require").ToLocalChecked(), requireFunction);

    // console.log(...)

    // setup console object
    Local<Object> consoleObject = Nan::New<Object>();

    // get handle to log function
    Local<FunctionTemplate> logTemplate = Nan::New<FunctionTemplate>(ConsoleLog);
    Local<Function> logFunction = logTemplate->GetFunction();
    logFunction->SetName(Nan::New<String>("log").ToLocalChecked());

    // attach log function to console object
    Nan::Set(consoleObject, Nan::New<String>("log").ToLocalChecked(), logFunction);

    // attach object to context
    Nan::Set(globalContext, Nan::New<String>("console").ToLocalChecked(), consoleObject);

}

void IsolateContext::UpdateContextFileProperties(Local<Object> contextObject, const FILE_INFO* fileInfo)
{
    Nan::HandleScope scope;

    // set the file properites on the context
    Nan::Set(
        contextObject,
        Nan::New<String>("__dirname").ToLocalChecked(),
        Nan::New<String>(fileInfo->folderPath).ToLocalChecked());
    Nan::Set(
        contextObject,
        Nan::New<String>("__filename").ToLocalChecked(),
        Nan::New<String>(fileInfo->fullPath).ToLocalChecked());
}

void IsolateContext::CloneGlobalContextObject(Local<Object> sourceObject, Local<Object> cloneObject)
{
    Nan::HandleScope scope;

    // copy global properties
    Nan::Set(
        cloneObject,
        Nan::New<String>("global").ToLocalChecked(),
        Nan::Get(
            sourceObject,
            Nan::New<String>("global").ToLocalChecked()).ToLocalChecked());
    Nan::Set(
        cloneObject,
        Nan::New<String>("require").ToLocalChecked(),
        Nan::Get(
            sourceObject,
            Nan::New<String>("require").ToLocalChecked()).ToLocalChecked());
    Nan::Set(
        cloneObject,
        Nan::New<String>("console").ToLocalChecked(),
        Nan::Get(
            sourceObject,
            Nan::New<String>("console").ToLocalChecked()).ToLocalChecked());
}

void IsolateContext::CreateModuleContext(Local<Object> contextObject, const FILE_INFO* fileInfo)
{
    Nan::HandleScope scope;

    // create the module/exports within context
    Local<Object> moduleObject = Nan::New<Object>();
    Nan::Set(moduleObject, Nan::New<String>("exports").ToLocalChecked(), Nan::New<Object>());
    Nan::Set(contextObject, Nan::New<String>("module").ToLocalChecked(), moduleObject);
    Nan::Set(
        contextObject,
        Nan::New<String>("exports").ToLocalChecked(),
        Nan::Get(moduleObject, Nan::New<String>("exports").ToLocalChecked()).ToLocalChecked());

    // copy file properties
    if(fileInfo != NULL)
    {
        IsolateContext::UpdateContextFileProperties(contextObject, fileInfo);
    }
}
