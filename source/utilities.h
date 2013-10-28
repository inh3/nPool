#ifndef _UTILITIES_H_
#define _UTILITIES_H_

// node
#include <node.h>
#include <v8.h>
using namespace v8;

class Utilities
{
    public:
        
        // create a standard ascii char*
        static char*        CreateCharBuffer(Handle<String> v8String);

        // create a utf8 char*
        static const char*  ToCString(const String::Utf8Value& value);

        // read file contents to char buffer
        static const char*  ReadFile(const char* fileName, int* fileSize);

        // clone specific properties of an object
        static void         CloneObject(Handle<Object> sourceObject, Handle<Object> cloneObject);

        // exception handler
        static void         HandleException(TryCatch* tryCatch, bool throwException);

        // print object properties
        static void         PrintObjectProperties(Handle<Object> objectHandle);
};

#endif /* _UTILITIES_H_ */