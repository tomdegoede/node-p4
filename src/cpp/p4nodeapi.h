#ifndef P4NODEAPI_H
#define P4NODEAPI_H

#include <node.h>
#include <uv.h>
#include <node_object_wrap.h>

#include "clientapi.h"
#include "basicuser.h"

class P4NodeApi : public node::ObjectWrap {
 public:
  static void Init(v8::Handle<v8::Object> exports);
  static void NewInstance(const v8::FunctionCallbackInfo<v8::Value>& args);

 private:
  explicit P4NodeApi();
  ~P4NodeApi();

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void Run(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void Close(const v8::FunctionCallbackInfo<v8::Value>& args);

  static void EIO_P4(uv_work_t *req);
  static void EIO_AfterP4(uv_work_t *req);

  static v8::Persistent<v8::Function> constructor;
  int m_count;
  BasicUser ui;
  ClientApi client;
};

#endif