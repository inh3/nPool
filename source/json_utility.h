#ifndef _JSON_UTILITY_H_
#define _JSON_UTILITY_H_

// node
#include <node.h>
#include <v8.h>
using namespace v8;

class JsonUtility
{
    public:

        static char*            Stringify(Handle<Value> valueHandle);
        static Handle<Value>    Parse(char* objectString);
};

#endif /* _JSON_UTILITY_H_ */
