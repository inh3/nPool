#include "utilities.h"

// C++
#include <string>

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
#include "json_utility.h"

char* Utilities::CreateCharBuffer(Local<String> v8String)
{
    Nan::Utf8String bufferValue(v8String);
    char* charBuffer = (char*)malloc(bufferValue.length() + 1);
    memset(charBuffer, 0, bufferValue.length() + 1);
    memcpy(charBuffer, *bufferValue, bufferValue.length());
    return charBuffer;
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
Nan::Utf8String* Utilities::HandleException(TryCatch* tryCatch, bool createExceptionObject)
{
    // return value
    Nan::Utf8String* exceptionSerialized = NULL;

    // create scope for exception
    Nan::HandleScope scope;

    // get the exception message
    Local<Message> exceptionMessage = tryCatch->Message();

    // the exception message was not valid
    if (exceptionMessage.IsEmpty())
    {
        // build exception object if required
        if(createExceptionObject == true)
        {
            Local<Object> exceptionObject = Nan::New<Object>();
            Nan::Set(exceptionObject, Nan::New<String>("message").ToLocalChecked(), tryCatch->Exception());
            exceptionSerialized = JsonUtility::Stringify(exceptionObject);
        }
    }
    // there was a valid message attached to the exception
    else
    {
        // build exception object if required
        if(createExceptionObject == true)
        {
            Local<Object> exceptionObject = Nan::New<Object>();
            Nan::Set(exceptionObject, Nan::New<String>("message").ToLocalChecked(), tryCatch->Message()->Get());
            Nan::Set(exceptionObject, Nan::New<String>("resourceName").ToLocalChecked(), exceptionMessage->GetScriptResourceName());
            Nan::Set(exceptionObject, Nan::New<String>("lineNum").ToLocalChecked(), Nan::New<Number>(exceptionMessage->GetLineNumber()));
            Nan::Set(exceptionObject, Nan::New<String>("sourceLine").ToLocalChecked(), exceptionMessage->GetSourceLine());
            // missing reference with 0.11.13
            #if !(NODE_VERSION_AT_LEAST(0, 11, 13))
            Nan::Set(exceptionObject, Nan::New<String>("scriptData").ToLocalChecked(), exceptionMessage->GetScriptData());
            #endif
            if(!tryCatch->StackTrace().IsEmpty())
            {
                Nan::Set(exceptionObject, Nan::New<String>("stackTrace").ToLocalChecked(), tryCatch->StackTrace());
            }
            else
            {
                Nan::Set(exceptionObject, Nan::New<String>("stackTrace").ToLocalChecked(), Nan::Null());
            }

            exceptionSerialized = JsonUtility::Stringify(exceptionObject);
        }
    }

    return exceptionSerialized;
}

void Utilities::CopyObject(Local<Object> toObject, Local<Object> fromObject)
{
    Local<Array> propertyKeys = Nan::GetPropertyNames(fromObject).ToLocalChecked();
    for (uint32_t keyIndex = 0; keyIndex < propertyKeys->Length(); keyIndex++)
    {
        Local<Value> propertyKey = propertyKeys->Get(keyIndex);
        Nan::Set(toObject, propertyKey, Nan::Get(fromObject, propertyKey).ToLocalChecked());
    }
}

FILE_INFO* Utilities::GetFileInfo(const char* relativePath, const char* currentDirectory)
{
    // return value
    FILE_INFO* fileInfo = (FILE_INFO*)malloc(sizeof(FILE_INFO));
    memset(fileInfo, 0, sizeof(FILE_INFO));

    /*fprintf(stdout, "[ Utilities - GetFileInfo ] Relative Path: %s Current Dir: %s\n",
        relativePath,
        currentDirectory);*/

    // check if relative path should be combined with current directory
    std::string filePath(relativePath);
    //fprintf(stdout, "[ Utilities - GetFileInfo ] File Path: %s\n", filePath.c_str());
    if((currentDirectory != NULL) && (strlen(relativePath) > 2))
    {
        // combine current directory to relative path if it starts with './' or '../'
        if(((relativePath[0] == '.') && ((relativePath[1] == '\\') || (relativePath[1] == '/'))) ||
            (relativePath[0] == '.' && relativePath[1] == '.' && ((relativePath[2] == '\\') || (relativePath[2] == '/'))))
        {
            filePath.insert(0, currentDirectory);
        }
    }

    // get the full path of the file
    #ifdef _WIN32
        // http://msdn.microsoft.com/en-us/library/506720ff.aspx
        fileInfo->fullPath = (const char*)malloc(_MAX_PATH + 1);
        memset((void*)fileInfo->fullPath, 0, _MAX_PATH + 1);
        _fullpath((char*)fileInfo->fullPath, filePath.c_str(), _MAX_PATH);

        // http://msdn.microsoft.com/en-us/library/a2xs1dts.aspx
        if((_access_s((char*)fileInfo->fullPath, 0)) != 0)
        {
            //fprintf(stdout, "[ Utilities - Error ] Invalid File: %s\n", filePath.c_str());
            free((void*)fileInfo->fullPath);
            fileInfo->fullPath = 0;
        }
    #else
        fileInfo->fullPath = (char*)malloc(PATH_MAX + 1);
        memset((void*)fileInfo->fullPath, 0, PATH_MAX + 1);
        if(realpath(filePath.c_str(), (char *)fileInfo->fullPath) == NULL)
        {
            //fprintf(stdout, "[ Utilities - Error ] Invalid File: %s\n", filePath.c_str());
            free((void*)fileInfo->fullPath);
            fileInfo->fullPath = 0;
        }
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
