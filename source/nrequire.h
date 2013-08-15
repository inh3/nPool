#ifndef _NREQUIRE_H_
#define _NREQUIRE_H_

// node
#include <node.h>
#include <v8.h>
using namespace v8;

// threadpool
#include "synchronize.h"

class Require
{
    public:
        
        static Handle<Value>    RequireFunction(const Arguments& args);
};

#endif /* _NREQUIRE_H_ */