/*********************************************************************
 * NAN - Native Abstractions for Node.js
 *
 * Copyright (c) 2015 NAN contributors
 *
 * MIT License <https://github.com/nodejs/nan/blob/master/LICENSE.md>
 ********************************************************************/

#include <nan.h>

using namespace Nan;  // NOLINT(build/namespaces)

// ref: https://nodejs.org/api/addons.html#addons_factory_of_wrapped_objects
class MyObject : public ObjectWrapTemplate<MyObject> {
 public:
  static NAN_MODULE_INIT(Init) {
    function_template tpl = NewConstructorFunctionTemplate("MyObject", 1);

    SetPrototypeMethod(tpl, "getValue", GetValue);
    SetPrototypeMethod(tpl, "increment", Increment);

    SetConstructorFunctionTemplate(tpl);
  }

  static NAN_METHOD(NewInstance) {
    NewInstanceMethod(info);
  }

 //private:
  MyObject(NAN_METHOD_ARGS_TYPE info) {
    value_ = (info.Length() < 1 || info[0]->IsUndefined()) ? 0 : info[0]->NumberValue();
  }
  ~MyObject() {}

private:
  // (same as static NAN_METHOD(GetValue) {...})
  static void GetValue(NAN_METHOD_ARGS_TYPE info) {
    MyObject* obj = ObjectFromMethodArgsInfo(info);
    info.GetReturnValue().Set(obj->value_);
  }

  static void Increment(NAN_METHOD_ARGS_TYPE info) {
    MyObject* obj = ObjectFromMethodArgsInfo(info);
    ++obj->value_;
  }

  double value_;
};

NAN_MODULE_INIT(Init) {
  MyObject::Init(target);
  Set(target
    , New<v8::String>("newFactoryObjectInstance").ToLocalChecked()
    , New<v8::FunctionTemplate>(MyObject::NewInstance)->GetFunction()
  );
}

NODE_MODULE(objectwraphandle, Init)
