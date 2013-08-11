#ifndef _UTILITIES_H_
#define _UTILITIES_H_

// node
#include <node.h>
#include <v8.h>
using namespace v8;

class Utilities
{
    public:
        
        // char* utilities
        static char*        CreateCharBuffer(Handle<String> v8String);

        // Extracts a C string from a V8 Utf8Value.
        static const char*  ToCString(const String::Utf8Value& value);

        // object traversal
        static void         ParseObject(Handle<Object> v8Object);
        static void         ParseArray(Handle<Array> v8Array);
};

#endif /* _UTILITIES_H_ */