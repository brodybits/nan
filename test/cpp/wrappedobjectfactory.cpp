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
    SetPrototypeMethod(tpl, "makeAnotherFactoryObject", MakeAnotherFactoryObject);

    SetConstructorFunctionTemplate(tpl);
  }

  static NAN_METHOD(NewInstance) {
    NewInstanceMethod(info);
  }

 private:
  friend class ObjectWrapTemplate;
  MyObject(NAN_METHOD_ARGS_TYPE info) {
    value_ = (info.Length() < 1 || info[0]->IsUndefined()) ? 0 : info[0]->NumberValue();
  }
  ~MyObject() {}

  // (same as static NAN_METHOD(GetValue) {...})
  static void GetValue(NAN_METHOD_ARGS_TYPE info) {
    MyObject* obj = ObjectFromMethodArgsInfo(info);
    info.GetReturnValue().Set(obj->value_);
  }

  static void Increment(NAN_METHOD_ARGS_TYPE info) {
    MyObject* obj = ObjectFromMethodArgsInfo(info);
    ++obj->value_;
  }

  static void MakeAnotherFactoryObject(NAN_METHOD_ARGS_TYPE info);

  double value_;
};

class AnotherFactoryObject : public ObjectWrapTemplate<AnotherFactoryObject> {
 public:
  static NAN_MODULE_INIT(Init) {
    function_template tpl = NewConstructorFunctionTemplate("AnotherFactoryObject", 1);
    SetPrototypeMethod(tpl, "getValue", GetValue);
    SetConstructorFunctionTemplate(tpl);
  }

  static v8::Local<v8::Value> NewInstance(int argc, v8::Local<v8::Value> argv[]) {
    return NewInstanceMethod(argc, argv);
  }

 private:
  friend class ObjectWrapTemplate;
  AnotherFactoryObject(NAN_METHOD_ARGS_TYPE info) {
    value_ = (info.Length() < 1 || info[0]->IsUndefined()) ? 0 : info[0]->NumberValue();
  }
  ~AnotherFactoryObject() {}

  // (same as static NAN_METHOD(GetValue) {...})
  static void GetValue(NAN_METHOD_ARGS_TYPE info) {
    AnotherFactoryObject* obj = ObjectFromMethodArgsInfo(info);
    info.GetReturnValue().Set(obj->value_);
  }

  double value_;
};

void MyObject::MakeAnotherFactoryObject(NAN_METHOD_ARGS_TYPE info) {
  const int argc = 1;
  v8::Local<v8::Value> argv[1] = { info[0] };
  info.GetReturnValue().Set(AnotherFactoryObject::NewInstance(argc, argv));
}

NAN_MODULE_INIT(Init) {
  MyObject::Init(target);
  AnotherFactoryObject::Init(target);
  Set(target
    , New<v8::String>("newFactoryObjectInstance").ToLocalChecked()
    , New<v8::FunctionTemplate>(MyObject::NewInstance)->GetFunction()
  );
}

NODE_MODULE(objectwraphandle, Init)
