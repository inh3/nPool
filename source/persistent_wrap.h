#ifndef _PERSISTENT_WRAP_H_
#define _PERSISTENT_WRAP_H_

#include <node_object_wrap.h>
//using namespace node;
#include <v8.h>
using namespace v8;

#include <nan.h>

class PersistentWrap : public node::ObjectWrap
{
    public:

        // provide public access to Node's ObjectWrap
        // Ref(), Unref() and Wrap()
        virtual void Ref() { node::ObjectWrap::Ref(); }
        virtual void Unref() { node::ObjectWrap::Unref(); }
        virtual void Wrap(Handle<Object> handle) { node::ObjectWrap::Wrap(handle); }

    //private:

        //virtual ~PersistentWrap() { printf("PersistentWrap::~PersistentWrap\n"); }
};

#endif /* _PERSISTENT_WRAP_H_ */
