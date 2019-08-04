# Running Wasm on V8 via C API

Tested V8 version: 7.8.58

## Prerequisite

[Building V8 with GN](https://v8.dev/docs/build-gn)

## Build & Execution

1. Make sure you are in the V8 source directory
2. Copy `wasm.cc` and `wasm-c-api.cc` in `samples/` directory.
3. Apply `v8-build-gn.diff`
4. Run:
```sh
$ autoninja -C out/foo v8_wasm
$ autoninja -C out/foo v8_wasm_c_api
```
5. Executete binaries:
```sh
$ out/foo/v8_wasm
$ out/foo/v8_wasm_c_api
```
