## C Interop Guide

Zero supports a small, explicit C ABI export surface. Exported C functions must use primitive scalar parameters or explicit references accepted by the native checker.

```zero
export c fun add(a: i32, b: i32) -> i32 {
    return a + b
}
```

Check the ABI surface:

```sh
bin/zero check conformance/native/pass/c-abi-export.0
bin/zero abi dump --json conformance/native/pass/c-abi-export.0
```

The ABI dump reports the exported C symbol list and a small generated header text block. For `conformance/native/pass/c-abi-export.0`, `generatedHeader.available` is `true` and the header contains the `zero_add` declaration.

Invalid export surfaces fail before C emission:

```sh
bin/zero check --json conformance/native/fail/bad-c-export.0
```

Header imports are available as typed metadata:

```sh
bin/zero graph --json --target wasm32-web conformance/check/pass/c-header-import.0
```

The graph JSON exposes `cImports[].typedModel` with imported functions, constants, structs, enums, and typedefs. It also includes a cache object keyed by header hash, target, ABI, flags hash, and sysroot fingerprint so agents can tell when bindings are target-specific.

External C calls still require target library audit facts. `zero graph --json` reports each `cLibraries[].linkPlan` with include paths, library paths, sysroot status, target ABI, and host discovery status. Cross builds must use package-relative vendored headers/libraries or an explicit target sysroot; they cannot silently reuse host include or library paths.

Unsafe foreign-target discovery fails with `CIMP003` before code generation:

```sh
bin/zero build --json --target linux-musl-x64 conformance/c/host-leak-package --out .zero/out/host-leak-package
```
