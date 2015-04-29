#ifndef _NREQUIRE_H_
#define _NREQUIRE_H_

// node
#include <node.h>
#include <v8.h>
using namespace v8;

#include <nan.h>

// threadpool
#include "synchronize.h"

// custom
#include "utilities.h"

#define REQUIRE_FUNCTION_NAME "require"

class Require
{
    public:

        static NAN_METHOD(RequireFunction);

    private:

        static void     FreeFileInfo(FILE_INFO* fileInfo);
};

#endif /* _NREQUIRE_H_ */
