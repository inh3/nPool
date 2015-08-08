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

        static Nan::Utf8String*   Stringify(Local<Value> valueHandle);
        static Local<Value>    Parse(char* objectString);
};

#endif /* _JSON_UTILITY_H_ */
