#include "json_utility.h"

#include <nan.h>

// C
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// custom source
#include "utilities.h"

char* JsonUtility::Stringify(Handle<Value> valueHandle)
{
    NanScope();

    // get reference to JSON object
    Handle<Object> contextObject = NanGetCurrentContext()->Global();
    Handle<Object> jsonObject = contextObject->Get(NanNew<String>("JSON"))->ToObject();
    Handle<Function> stringifyFunc = jsonObject->Get(NanNew<String>("stringify")).As<Function>();

    // execute stringify
    Handle<Value> stringifyResult = stringifyFunc->Call(jsonObject, 1, &valueHandle);

    // only process if the result is valid
    if(stringifyResult->IsUndefined() == false)
    {
        NanUtf8String utf8String(stringifyResult);
        char* returnString = (char *)malloc(utf8String.length() + 1);
        memset(returnString, 0, utf8String.length() + 1);
        memcpy(returnString, *utf8String, utf8String.length() + 1);
        return returnString;
    }

    char* returnString = (char *)malloc(1);
    memset(returnString, 0, 1);
    return returnString;
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
