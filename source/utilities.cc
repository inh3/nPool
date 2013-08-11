#include "utilities.h"

// C
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* Utilities::CreateCharBuffer(Handle<String> v8String)
{
    char* charBuffer = (char*)malloc(v8String->Length() + 1);
    memset(charBuffer, 0, v8String->Length() + 1);
    String::AsciiValue bufferValue(v8String);
    memcpy(charBuffer, *bufferValue, v8String->Length());

    return charBuffer;
}

// taken from examples given at: http://izs.me/v8-docs/classv8_1_1String_1_1Utf8Value.html
const char* Utilities::ToCString(const String::Utf8Value& value)
{
    return *value ? *value : "<string conversion failed>";
}

void Utilities::ParseObject(Handle<Object> v8Object)
{
    // get object properties
    Local<Array> v8ObjectProperties = v8Object->GetOwnPropertyNames();
    for(uint32_t i = 0; i < v8ObjectProperties->Length(); i++)
    {
        // get property name
        Local<String> propertyName = v8ObjectProperties->Get(i)->ToString();
        String::AsciiValue propertyString(propertyName);
        fprintf(stdout, "Utilities::ParseObject - Property: (%u) %s |", i, *propertyString);

        // get property values
        Local<Value> propertyValue = v8Object->Get(v8ObjectProperties->Get(i));
        if(propertyValue->IsString())
        {
            Local<String> stringProperty = propertyValue->ToString();
            String::AsciiValue value(stringProperty);
            fprintf(stdout, " String: %s\n", *value);
        }
        else if(propertyValue->IsNumber())
        {
            Local<Number> numberProperty = propertyValue->ToNumber();
            uint32_t value = numberProperty->Value();
            fprintf(stdout, " Number: %u\n", value);
        }
        else if(propertyValue->IsFunction())
        {
            fprintf(stdout, " Function\n");
        }
        else if(propertyValue->IsArray())
        {
            Utilities::ParseArray(Local<Array>::Cast(propertyValue));
        }
        else if(propertyValue->IsObject())
        {
            Utilities::ParseObject(propertyValue->ToObject());
        }
        else
        {
            fprintf(stdout, "\n");
        }
    }
}

void Utilities::ParseArray(Handle<Array> v8Array)
{
    fprintf(stdout, " Array:\n");
    for(uint32_t idx = 0; idx < v8Array->Length(); idx++)
    {
        fprintf(stdout, "\t%u) ", idx);

        Handle<Value> arrayValue = v8Array->Get(idx);
        if(arrayValue->IsString())
        {
            Local<String> stringProperty = arrayValue->ToString();
            String::AsciiValue value(stringProperty);
            fprintf(stdout, "%s\n", *value);
        }
        else if(arrayValue->IsNumber())
        {
            Local<Number> numberProperty = arrayValue->ToNumber();
            uint32_t value = numberProperty->Value();
            fprintf(stdout, "%u\n", value);
        }
        else if(arrayValue->IsObject())
        {
            //Local<Object> objectProperty = arrayValue->ToObject();
            //Utilities::ParseObject(objectProperty);
        }
    }
}