#include "json.h"

// C
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// custom source
#include "utilities.h"

char* JSON::Stringify(Handle<Object> v8Object)
{
    HandleScope scope;

    // get handle to global context object
    Handle<Context> context = Context::GetCurrent();
    Handle<Object> global = context->Global();

    // get handle to JSON.stringify()
    Local<Object> JSON = global->Get(String::New("JSON"))->ToObject();
    Local<Function> Stringify = Local<Function>::Cast(JSON->Get(String::New("stringify")));

    // execute stringify
    Handle<Value> v8ObjectHandle = (Handle<Value>)v8Object;
    Local<String> jsonObject = Local<String>::Cast(Stringify->Call(JSON, 1, &(v8ObjectHandle)));

    // get and copy json string (utf8)
    String::Utf8Value utf8ObjectString(jsonObject);
    const char* utf8String = Utilities::ToCString(utf8ObjectString);
    uint32_t utf8StringLength = strlen(utf8String);
    char* returnString = (char *)malloc(utf8StringLength + 1);
    memset(returnString, 0, utf8StringLength + 1);
    memcpy(returnString, *utf8ObjectString, utf8StringLength);

    scope.Close(Undefined());

    return returnString;
}

Handle<Object> JSON::Parse(char* jsonObject)
{
    HandleScope scope;

    // get handle to global context object
    Handle<Context> context = Context::GetCurrent();
    Handle<Object> global = context->Global();

    // get handle to JSON.parse()
    Local<Object> JSON = global->Get(String::New("JSON"))->ToObject();
    Local<Function> Parse = Local<Function>::Cast(JSON->Get(String::New("parse")));

    // execute parse
    Handle<Value> jsonString = String::New(jsonObject);
    Local<Value> v8Object = Parse->Call(JSON, 1, &jsonString);

    return scope.Close(Handle<Object>::Cast(v8Object));
}