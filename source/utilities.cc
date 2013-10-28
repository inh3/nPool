#include "utilities.h"

// C
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// custom
#include "synchronize.h"

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

const char* Utilities::ReadFile(const char* fileName, int* fileSize)
{
    
    // reference to c-string version of file
    char *fileBuffer = 0;

    // attempt to open the file
    FILE* fd = fopen(fileName, "rb");

    // clear file size
    *fileSize = 0;

    // file was valid
    if(fd != 0)
    {
        // get size of file
        fseek(fd, 0, SEEK_END);
        *fileSize = ftell(fd);
        rewind(fd);

        // allocate file buffer for file contents
        fileBuffer = new char[*fileSize + 1];
        fileBuffer[*fileSize] = 0;

        // copy file contents
        for (int charCount = 0; charCount < *fileSize;)
        {
            int charRead = static_cast<int>(fread(&fileBuffer[charCount], 1, *fileSize - charCount, fd));
            charCount += charRead;
        }

        // close the file
        fclose(fd);
    }

    return fileBuffer;
}

void Utilities::CloneObject(Handle<Object> sourceObject, Handle<Object> cloneObject)
{
    HandleScope handleScope;
    
    // copy global properties
    Handle<Value> cloneValue = sourceObject->Get(String::NewSymbol("require"));
    cloneObject->Set(String::NewSymbol("require"), cloneValue);
}

// https://code.google.com/p/v8/source/browse/trunk/samples/shell.cc
void Utilities::HandleException(TryCatch* tryCatch, bool throwException)
{
    // create scope for exception
    HandleScope handleScope;

    // get the exception string
    String::Utf8Value exceptionString(tryCatch->Exception());
    const char* exceptionCharStr = Utilities::ToCString(exceptionString);

    // get the exception message
    Handle<Message> exceptionMessage = tryCatch->Message();

    // the exception message was not valid
    if (exceptionMessage.IsEmpty())
    {
        // print the exception
        fprintf(stderr, "[ %u ] EXCEPTION - [ Message: %s ]\n", SyncGetThreadId(), exceptionCharStr);
    } 
    // there was a valid message attached to the exception
    else
    {
        // get file-name from the message
        v8::String::Utf8Value fileNameString(exceptionMessage->GetScriptResourceName());
        const char* fileNameCharStr = Utilities::ToCString(fileNameString);

        // get line-number of the exception
        int lineNum = exceptionMessage->GetLineNumber();

        // get the line of code
        v8::String::Utf8Value sourceLineString(exceptionMessage->GetSourceLine());
        const char* sourceLineCharStr = Utilities::ToCString(sourceLineString);

        // print the exception
        fprintf(stderr, "[ %u ] EXCEPTION - [ Message: %s ]\n", SyncGetThreadId(), exceptionCharStr);
        fprintf(stderr, "[ File: %s ] [ Line Num: %i ]\n", fileNameCharStr, lineNum);
        fprintf(stderr, "%s\n", sourceLineCharStr);

        // print indicator at point of exception on source line
        int start = exceptionMessage->GetStartColumn();
        for (int i = 0; i < start; i++) {
            fprintf(stderr, " ");
        }
        int end = exceptionMessage->GetEndColumn();
        for (int i = start; i < end; i++) {
            fprintf(stderr, "^");
        }
        fprintf(stderr, "\n");

        // print the stack trace
        v8::String::Utf8Value stackTraceString(tryCatch->StackTrace());
        if (stackTraceString.length() > 0)
        {
            fprintf(stderr, "[ Stack Trace ]\n");
            const char* stackTraceCharStr = Utilities::ToCString(stackTraceString);
            fprintf(stderr, "%s\n", stackTraceCharStr);
        }
    }

    // throw exceptio if requested
    if(throwException == true)
    {
        tryCatch->ReThrow();
    }
}

void Utilities::PrintObjectProperties(Handle<Object> objectHandle)
{
    Local<Array> propertyKeys = (*objectHandle)->GetPropertyNames();
    for (uint32_t keyIndex = 0; keyIndex < propertyKeys->Length(); keyIndex++)
    {
        Handle<v8::String> keyString = propertyKeys->Get(keyIndex)->ToString();
        String::AsciiValue propertyName(keyString);
        fprintf(stdout, "[ Property %u ] %s\n", keyIndex, *propertyName);
    }
}