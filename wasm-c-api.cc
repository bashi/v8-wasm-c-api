#include <stdio.h>

#include "include/v8.h"
#include "include/libplatform/libplatform.h"
#include "third_party/wasm-api/wasm.hh"

namespace wasm {

// Required as wasm.hh declares these.
#ifdef DEBUG
template <class T>
void vec<T>::make_data() {}

template <class T>
void vec<T>::free_data() {}
#endif

}  // namespace wasm

int main(int argc, char* argv[]) {
  // Required for Isolate initialization with snapshot.
  // Initialization is too slow without snapshot on Debug build.
  v8::V8::InitializeExternalStartupData(argv[0]);

  // Initializing wasm engine.
  auto engine = wasm::Engine::make();
  auto store_ = wasm::Store::make(engine.get());
  auto store = store_.get();

  //     (func (export "add") (param i32 i32) (result i32)
  //       get_local 0
  //       get_local 1
  //       i32.add)
  std::vector<byte_t> wasm_binary{
      0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x01, 0x07, 0x01,
      0x60, 0x02, 0x7f, 0x7f, 0x01, 0x7f, 0x03, 0x02, 0x01, 0x00, 0x07,
      0x07, 0x01, 0x03, 0x61, 0x64, 0x64, 0x00, 0x00, 0x0a, 0x09, 0x01,
      0x07, 0x00, 0x20, 0x00, 0x20, 0x01, 0x6a, 0x0b};

  auto binary = wasm::vec<byte_t>::make(wasm_binary.size(), &wasm_binary[0]);

  // Compile.
  auto module = wasm::Module::make(store, binary);
  if (!module) {
    printf("Error compiling module\n");
    return 1;
  }

  // Instantiating.
  auto instance = wasm::Instance::make(store, module.get(), nullptr);
  if (!instance) {
    printf("Error instantiating module\n");
    return 1;
  }

  // Extacting the function.
  auto exports = instance->exports();
  if (exports.size() != 1 || exports[0]->kind() != wasm::EXTERN_FUNC ||
      !exports[0]->func()) {
    printf("Expected one export of a function\n");
    return 1;
  }

  auto run_func = exports[0]->func();

  // Call.
  wasm::Val args[] = { wasm::Val::i32(14), wasm::Val::i32(9) };
  wasm::Val results[1];
  if (run_func->call(args, results)) {
    printf("Failed to execute function\n");
    return 1;
  }
  printf("14 + 9 = %d\n", results[0].i32());

  return 0;
}
