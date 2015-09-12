#ifndef _ARRAY_BUFFER_ALLOCATOR_H_
#define _ARRAY_BUFFER_ALLOCATOR_H_

// node version 4 requires array buffer allocator for isolates
// http://stackoverflow.com/a/30424476
#if NODE_MAJOR_VERSION == 4

#include <stdlib.h>
#include <string.h>

#include <v8.h>
using namespace v8;

// https://github.com/v8/v8-git-mirror/blob/master/samples/hello-world.cc
class ArrayBufferAllocator : public v8::ArrayBuffer::Allocator {
    public:
        virtual void* Allocate(size_t length) {
            void* data = AllocateUninitialized(length);
            return data == NULL ? data : memset(data, 0, length);
        }
        virtual void* AllocateUninitialized(size_t length) { return malloc(length); }
        virtual void Free(void* data, size_t) { free(data); }
};

#endif

#endif /* _ARRAY_BUFFER_ALLOCATOR_H_ */
