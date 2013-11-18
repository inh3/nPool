#ifndef _UTILITIES_H_
#define _UTILITIES_H_

// C++
#include <string>

// node
#include <node.h>
#include <v8.h>
using namespace v8;

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
        static char*        CreateCharBuffer(Handle<String> v8String);

        // create a utf8 char*
        static const char*  ToCString(const String::Utf8Value& value);

        // read file contents to char buffer
        static const char*  ReadFile(const char* fileName, int* fileSize);

        // exception handler
        static char*        HandleException(TryCatch* tryCatch, bool createExceptionObject = false);

        // print object properties
        static void         PrintObjectProperties(Handle<Object> objectHandle);

        // get file name and directory from path
        static FILE_INFO*   GetFileInfo(const char* relativePath, const char* currentDirectory = NULL);

        // free file info that was created from GetFileInfo
        static void         FreeFileInfo(const FILE_INFO* fileInfo);
};

#endif /* _UTILITIES_H_ */