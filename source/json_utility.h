#ifndef _JSON_UTILITY_H_
#define _JSON_UTILITY_H_

// node
#include <node.h>
#include <v8.h>
using namespace v8;

#include <nan.h>

class JsonUtility
{
    public:

        static NanUtf8String*   Stringify(Handle<Value> valueHandle);
        static Handle<Value>    Parse(char* objectString);
};

#endif /* _JSON_UTILITY_H_ */
