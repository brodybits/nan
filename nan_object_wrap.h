/*********************************************************************
 * NAN - Native Abstractions for Node.js
 *
 * Copyright (c) 2015 NAN contributors
 *
 * MIT License <https://github.com/nodejs/nan/blob/master/LICENSE.md>
 ********************************************************************/

#ifndef NAN_OBJECT_WRAP_H_
#define NAN_OBJECT_WRAP_H_

class ObjectWrap {
 public:
  ObjectWrap() {
    refs_ = 0;
  }


  virtual ~ObjectWrap() {
    if (persistent().IsEmpty()) {
      return;
    }

    assert(persistent().IsNearDeath());
    persistent().ClearWeak();
    persistent().Reset();
  }


  template <class T>
  static inline T* Unwrap(v8::Local<v8::Object> object) {
    assert(!object.IsEmpty());
    assert(object->InternalFieldCount() > 0);
    // Cast to ObjectWrap before casting to T.  A direct cast from void
    // to T won't work right when T has more than one base class.
    void* ptr = GetInternalFieldPointer(object, 0);
    ObjectWrap* wrap = static_cast<ObjectWrap*>(ptr);
    return static_cast<T*>(wrap);
  }


  inline v8::Local<v8::Object> handle() {
    return New(persistent());
  }


  inline Persistent<v8::Object>& persistent() {
    return handle_;
  }


 protected:
  inline void Wrap(v8::Local<v8::Object> object) {
    assert(persistent().IsEmpty());
    assert(object->InternalFieldCount() > 0);
    SetInternalFieldPointer(object, 0, this);
    persistent().Reset(object);
    MakeWeak();
  }

#if defined(V8_MAJOR_VERSION) && (V8_MAJOR_VERSION > 4 ||                      \
  (V8_MAJOR_VERSION == 4 && defined(V8_MINOR_VERSION) && V8_MINOR_VERSION >= 3))

  inline void MakeWeak() {
    persistent().v8::PersistentBase<v8::Object>::SetWeak(
        this, WeakCallback, v8::WeakCallbackType::kParameter);
    persistent().MarkIndependent();
  }

#elif NODE_MODULE_VERSION > NODE_0_10_MODULE_VERSION

  inline void MakeWeak() {
    persistent().v8::PersistentBase<v8::Object>::SetWeak(this, WeakCallback);
    persistent().MarkIndependent();
  }

#else

  inline void MakeWeak() {
    persistent().persistent.MakeWeak(this, WeakCallback);
    persistent().MarkIndependent();
  }

#endif

  /* Ref() marks the object as being attached to an event loop.
   * Refed objects will not be garbage collected, even if
   * all references are lost.
   */
  virtual void Ref() {
    assert(!persistent().IsEmpty());
    persistent().ClearWeak();
    refs_++;
  }

  /* Unref() marks an object as detached from the event loop.  This is its
   * default state.  When an object with a "weak" reference changes from
   * attached to detached state it will be freed. Be careful not to access
   * the object after making this call as it might be gone!
   * (A "weak reference" means an object that only has a
   * persistant handle.)
   *
   * DO NOT CALL THIS FROM DESTRUCTOR
   */
  virtual void Unref() {
    assert(!persistent().IsEmpty());
    assert(!persistent().IsWeak());
    assert(refs_ > 0);
    if (--refs_ == 0)
      MakeWeak();
  }

  int refs_;  // ro

 private:
  NAN_DISALLOW_ASSIGN_COPY_MOVE(ObjectWrap)
#if defined(V8_MAJOR_VERSION) && (V8_MAJOR_VERSION > 4 ||                      \
  (V8_MAJOR_VERSION == 4 && defined(V8_MINOR_VERSION) && V8_MINOR_VERSION >= 3))

  static void
  WeakCallback(v8::WeakCallbackInfo<ObjectWrap> const& info) {
    ObjectWrap* wrap = info.GetParameter();
    assert(wrap->refs_ == 0);
    assert(wrap->handle_.IsNearDeath());
    wrap->handle_.Reset();
    delete wrap;
  }

#elif NODE_MODULE_VERSION > NODE_0_10_MODULE_VERSION

  static void
  WeakCallback(v8::WeakCallbackData<v8::Object, ObjectWrap> const& data) {
    ObjectWrap* wrap = data.GetParameter();
    assert(wrap->refs_ == 0);
    assert(wrap->handle_.IsNearDeath());
    wrap->handle_.Reset();
    delete wrap;
  }

#else

  static void WeakCallback(v8::Persistent<v8::Value> value, void *data) {
    ObjectWrap *wrap = static_cast<ObjectWrap*>(data);
    assert(wrap->refs_ == 0);
    assert(wrap->handle_.IsNearDeath());
    wrap->handle_.Reset();
    delete wrap;
  }

#endif
  Persistent<v8::Object> handle_;
};

template <typename T>
class ObjectWrapTemplate : public Nan::ObjectWrap {
 protected:
  typedef v8::Local<v8::FunctionTemplate> function_template;

  static inline function_template
  NewConstructorFunctionTemplate(const char * object_name, int internal_field_count) {
    function_template tpl = Nan::New<v8::FunctionTemplate>(New);
    tpl->SetClassName(Nan::New(object_name).ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);
    return tpl;
  }

  static inline void
  SetConstructorFunctionTemplate(function_template tpl) {
    constructor().Reset(tpl->GetFunction());
  }

  static inline void
  SetNewFunction(Nan::ADDON_REGISTER_FUNCTION_ARGS_TYPE target,
                 function_template tpl, const char * object_name) {
    Nan::Set(target, Nan::New(object_name).ToLocalChecked(), tpl->GetFunction());
  }

  ObjectWrapTemplate() {}
  ~ObjectWrapTemplate() {}

  // Used by NewConstructorFunctionTemplate():
  // (same as static NAN_METHOD(New) { ... })
  static void New(Nan::NAN_METHOD_ARGS_TYPE info) {
    if (info.IsConstructCall()) {
      T *obj = new T(info);
      obj->Wrap(info.This());
      info.GetReturnValue().Set(info.This());
    } else {
      NewInstanceMethod(info);
    }
  }

  // ref: https://nodejs.org/api/addons.html#addons_factory_of_wrapped_objects
  // (same as static NAN_METHOD(NewInstanceMethod) { ... })
  static inline void NewInstanceMethod(Nan::NAN_METHOD_ARGS_TYPE info) {
    v8::Local<v8::Function> cons = Nan::New(constructor());
    const int argc = info.Length();
    std::vector<v8::Local<v8::Value> > argv;
    for (int i=0; i<argc; ++i) argv.push_back(info[i]);
    info.GetReturnValue().Set(cons->NewInstance(argc, &argv[0]));
  }

  static inline T * ObjectFromMethodArgsInfo(Nan::NAN_METHOD_ARGS_TYPE info) {
    return ObjectWrap::Unwrap<T>(info.This());
  }

 private:
  static inline Nan::Persistent<v8::Function> & constructor() {
    static Nan::Persistent<v8::Function> constructor_;
    return constructor_;
  }
};

#endif  // NAN_OBJECT_WRAP_H_
