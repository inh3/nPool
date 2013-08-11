#ifndef _JSON_H_
#define _JSON_H_

// node
#include <node.h>
#include <v8.h>
using namespace v8;

class JSON
{
    public:
        
        static char*            Stringify(Handle<Object> v8Object);
        static Handle<Object>   Parse(char* jsonObject);
};

#endif /* _JSON_H_ */