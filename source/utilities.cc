#include "utilities.h"

// C
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

// FILE and fxxx()
#ifndef _WIN32
#include <unistd.h>
#else
#include <io.h>
#endif

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
        fileBuffer = (char*)malloc(*fileSize + 1);
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

FILE_INFO* Utilities::GetFileInfo(const char* relativePath)
{
    // return value
    FILE_INFO* fileInfo = (FILE_INFO*)malloc(sizeof(FILE_INFO));
    memset(fileInfo, 0, sizeof(FILE_INFO));

    // get the full path of the file
    #ifdef _WIN32
        // http://msdn.microsoft.com/en-us/library/506720ff.aspx
        fileInfo->fullPath = (const char*)malloc(_MAX_PATH);
        memset((void*)fileInfo->fullPath, 0, _MAX_PATH);
        _fullpath((char*)fileInfo->fullPath, relativePath, _MAX_PATH);
    #else
        fileInfo->fullPath = (char*)malloc(PATH_MAX);
        memset((void*)fileInfo->fullPath, 0, PATH_MAX);
        realpath(relativePath, (char *)fileInfo->fullPath);
    #endif

    // path is valid
    if(fileInfo->fullPath != 0)
    {
        // get the file name only
        // http://stackoverflow.com/a/5902743
        const char *charPtr = fileInfo->fullPath + strlen(fileInfo->fullPath);
        for (; charPtr > fileInfo->fullPath; charPtr--)
        {
            if ((*charPtr == '\\') || (*charPtr == '/'))
            {
                fileInfo->fileName = ++charPtr;
                break;
            }
        }

        // allocate and store the folder only path
        unsigned int folderPathLength = strlen(fileInfo->fullPath) - strlen(fileInfo->fileName);
        fileInfo->folderPath = (const char*)malloc(folderPathLength + 1);
        memset((void*)fileInfo->folderPath, 0, folderPathLength + 1);
        memcpy((void*)fileInfo->folderPath, fileInfo->fullPath, folderPathLength);

        // allocate file buffer
        fileInfo->fileBuffer = Utilities::ReadFile(fileInfo->fullPath, &(fileInfo->fileBufferLength));
    }

    //fprintf(stdout, "[ Utilities - File ] Full Path: %s\n", fileInfo->fullPath);
    //fprintf(stdout, "[ Utilities - File ] Folder Path: %s\n", fileInfo->folderPath);
    //fprintf(stdout, "[ Utilities - File ] File Name: %s\n", fileInfo->fileName);

    return fileInfo;
}

void Utilities::FreeFileInfo(const FILE_INFO* fileInfo)
{
    free((void*)fileInfo->fullPath);
    free((void*)fileInfo->folderPath);
    free((void*)fileInfo->fileBuffer);

    free((FILE_INFO*)fileInfo);
}