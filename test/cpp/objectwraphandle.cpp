/*********************************************************************
 * NAN - Native Abstractions for Node.js
 *
 * Copyright (c) 2015 NAN contributors
 *
 * MIT License <https://github.com/nodejs/nan/blob/master/LICENSE.md>
 ********************************************************************/

#include <nan.h>

using namespace Nan;  // NOLINT(build/namespaces)

class MyObject : public ObjectWrapTemplate<MyObject> {
 public:
  static NAN_MODULE_INIT(Init) {
    function_template tpl = NewConstructorFunctionTemplate("MyObject", 1);

    SetPrototypeMethod(tpl, "getHandle", GetHandle);
    SetPrototypeMethod(tpl, "getValue", GetValue);

    SetConstructorFunctionTemplate(tpl);
    SetNewFunction(target, tpl, "MyObject");
  }

  MyObject(NAN_METHOD_ARGS_TYPE info) {
    value_ = (info.Length() < 1 || info[0]->IsUndefined()) ? 0 : info[0]->NumberValue();
  }

  ~MyObject() {}

private:
  // (same as static NAN_METHOD(GetHandle) {...})
  static void GetHandle(NAN_METHOD_ARGS_TYPE info) {
    MyObject* obj = ObjectFromMethodArgsInfo(info);
    info.GetReturnValue().Set(obj->handle());
  }

  // (same as static NAN_METHOD(GetValue) {...})
  static void GetValue(NAN_METHOD_ARGS_TYPE info) {
    MyObject* obj = ObjectFromMethodArgsInfo(info);
    info.GetReturnValue().Set(obj->value_);
  }

  double value_;
};

NODE_MODULE(objectwraphandle, MyObject::Init)
