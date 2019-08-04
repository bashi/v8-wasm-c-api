// Based on https://stackoverflow.com/questions/53925972/call-webassembly-from-embedded-v8-without-js

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include/libplatform/libplatform.h"
#include "include/v8.h"

int main(int argc, char* argv[]) {
  v8::V8::InitializeICUDefaultLocation(argv[0]);
  v8::V8::InitializeExternalStartupData(argv[0]);
  std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
  v8::V8::InitializePlatform(platform.get());
  v8::V8::Initialize();

  v8::Isolate::CreateParams create_params;
  create_params.array_buffer_allocator =
      v8::ArrayBuffer::Allocator::NewDefaultAllocator();
  v8::Isolate* isolate = v8::Isolate::New(create_params);

  {
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);

    v8::Local<v8::Context> context = v8::Context::New(isolate);
    v8::Context::Scope context_scope(context);

    //     (func (export "add") (param i32 i32) (result i32)
    //       get_local 0
    //       get_local 1
    //       i32.add)
    std::vector<const uint8_t> wasm_binary{
        0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x01, 0x07, 0x01,
        0x60, 0x02, 0x7f, 0x7f, 0x01, 0x7f, 0x03, 0x02, 0x01, 0x00, 0x07,
        0x07, 0x01, 0x03, 0x61, 0x64, 0x64, 0x00, 0x00, 0x0a, 0x09, 0x01,
        0x07, 0x00, 0x20, 0x00, 0x20, 0x01, 0x6a, 0x0b};

    v8::MemorySpan<const uint8_t> serialized_module;
    v8::MemorySpan<const uint8_t> wire_bytes(&wasm_binary[0],
                                             wasm_binary.size());
    v8::Local<v8::WasmModuleObject> module =
        v8::WasmModuleObject::DeserializeOrCompile(isolate, serialized_module,
                                                   wire_bytes)
            .ToLocalChecked();

    v8::Local<v8::Value> instance_args[] = {module};
    v8::Local<v8::Object> module_instance_exports =
        context->Global()
            ->Get(context, v8::String::NewFromUtf8(isolate, "WebAssembly")
                               .ToLocalChecked())
            .ToLocalChecked()
            .As<v8::Object>()
            ->Get(context,
                  v8::String::NewFromUtf8(isolate, "Instance").ToLocalChecked())
            .ToLocalChecked()
            .As<v8::Object>()
            ->CallAsConstructor(context, 1, instance_args)
            .ToLocalChecked()
            .As<v8::Object>()
            ->Get(context,
                  v8::String::NewFromUtf8(isolate, "exports").ToLocalChecked())
            .ToLocalChecked()
            .As<v8::Object>();

    v8::Local<v8::Value> add_args[] = {v8::Int32::New(isolate, 77),
                                       v8::Int32::New(isolate, 88)};
    v8::Local<v8::Int32> adder_res =
        module_instance_exports
            ->Get(context,
                  v8::String::NewFromUtf8(isolate, "add").ToLocalChecked())
            .ToLocalChecked()
            .As<v8::Function>()
            ->Call(context, context->Global(), 2, add_args)
            .ToLocalChecked()
            .As<v8::Int32>();

    printf("77 + 88 = %d\n", adder_res->Value());
  }

  isolate->Dispose();
  v8::V8::Dispose();
  v8::V8::ShutdownPlatform();
  return 0;
}
