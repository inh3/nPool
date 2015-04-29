#include "json_utility.h"

// C
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// custom source
#include "utilities.h"

NanUtf8String* JsonUtility::Stringify(Handle<Value> valueHandle)
{
    NanScope();

    // get reference to JSON object
    Handle<Object> contextObject = NanGetCurrentContext()->Global();
    Handle<Object> jsonObject = contextObject->Get(NanNew<String>("JSON"))->ToObject();
    Handle<Function> stringifyFunc = jsonObject->Get(NanNew<String>("stringify")).As<Function>();

    // execute stringify
    Handle<Value> stringifyResult = stringifyFunc->Call(jsonObject, 1, &valueHandle);
    return new NanUtf8String(stringifyResult);
}

Handle<Value> JsonUtility::Parse(char* objectString)
{
    NanEscapableScope();

    // short circuit if bad object
    if(objectString == NULL)
    {
        return NanEscapeScope(NanUndefined());
    }

    // get reference to JSON object
    Handle<Object> contextObject = NanGetCurrentContext()->Global();
    Handle<Object> jsonObject = contextObject->Get(NanNew<String>("JSON"))->ToObject();
    Handle<Function> parseFunc = jsonObject->Get(NanNew<String>("parse")).As<Function>();

    // execute parse
    Handle<Value> jsonString = NanNew<String>(objectString);
    Local<Value> valueHandle = parseFunc->Call(jsonObject, 1, &jsonString);

    return NanEscapeScope(valueHandle);
}
