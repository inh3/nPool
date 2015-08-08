#ifndef _UTILITIES_H_
#define _UTILITIES_H_

// C++
#include <string>

// node
#include <node.h>
#include <v8.h>
using namespace v8;

#include <nan.h>

typedef struct FILE_INFO_STRUCT
{
    const char*             fileName;

    const char*             folderPath;

    const char*             fullPath;

    const char*             fileBuffer;

    int                     fileBufferLength;

} FILE_INFO;

class Utilities
{
    public:

        // create a standard ascii char*
        static char*            CreateCharBuffer(Local<String> v8String);

        // read file contents to char buffer
        static const char*      ReadFile(const char* fileName, int* fileSize);

        // exception handler
        static Nan::Utf8String*   HandleException(TryCatch* tryCatch, bool createExceptionObject = false);

        // copy properties from one object to another
        static void             CopyObject(Local<Object> toObject, Local<Object> fromObject);

        // print object properties
        static void             PrintObjectProperties(Local<Object> objectHandle);

        // get file name and directory from path
        static FILE_INFO*       GetFileInfo(const char* relativePath, const char* currentDirectory = NULL);

        // free file info that was created from GetFileInfo
        static void             FreeFileInfo(const FILE_INFO* fileInfo);
};

#endif /* _UTILITIES_H_ */
