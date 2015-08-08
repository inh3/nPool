#include "json_utility.h"

// C
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// custom source
#include "utilities.h"

Nan::Utf8String* JsonUtility::Stringify(Local<Value> valueHandle)
{
    Nan::HandleScope scope;

    // get reference to JSON object
    Local<Object> contextObject = Nan::GetCurrentContext()->Global();

    Local<String> propertyName = Nan::New<String>("JSON").ToLocalChecked();
    Local<Object> jsonObject = Nan::To<Object>(Nan::Get(contextObject, propertyName).ToLocalChecked()).ToLocalChecked();

    propertyName = Nan::New<String>("stringify").ToLocalChecked();
    Local<Function> stringifyFunc = Nan::Get(jsonObject, propertyName).ToLocalChecked().As<Function>();

    // execute stringify
    Local<Value> stringifyResult = stringifyFunc->Call(jsonObject, 1, &valueHandle);
    return new Nan::Utf8String(stringifyResult);
}

Local<Value> JsonUtility::Parse(char* objectString)
{
    Nan::EscapableHandleScope scope;

    // short circuit if bad object
    if(objectString == NULL)
    {
        return scope.Escape(Nan::Undefined());
    }

    // get reference to JSON object
    Local<Object> contextObject = Nan::GetCurrentContext()->Global();

    Local<String> propertyName = Nan::New<String>("JSON").ToLocalChecked();
    Local<Object> jsonObject = Nan::To<Object>(Nan::Get(contextObject, propertyName).ToLocalChecked()).ToLocalChecked();

    propertyName = Nan::New<String>("parse").ToLocalChecked();
    Local<Function> parseFunc = Nan::Get(jsonObject, propertyName).ToLocalChecked().As<Function>();

    // execute parse
    Local<Value> jsonString = Nan::New<String>(objectString).ToLocalChecked();
    Local<Value> valueHandle = parseFunc->Call(jsonObject, 1, &jsonString);

    return scope.Escape(valueHandle);
}
