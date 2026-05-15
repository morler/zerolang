## Install Zero

Zero can be built from this repository. The local compiler is exposed through `bin/zero`.

```sh
npm install
make -C native/zero-c
bin/zero --version
```

Use `zero doctor` to check the local environment:

```sh
bin/zero doctor
bin/zero doctor --json
```

For supported native executable builds, Zero uses direct emitters and does not need a C compiler. `zero doctor` still reports PATH health, workspace write access, bundled target support, and target SDK/sysroot status because those facts matter for target readiness and interop workflows. `zero doctor --json` includes `targetToolchains`, a per-target readiness matrix for relevant tools.

The experimental `--emit wasm --target wasm32-wasi` path writes a minimal WebAssembly module directly and does not require an external C toolchain, but it only supports the direct-wasm MVP subset.

```sh
bin/zero build --emit exe --target linux-musl-x64 examples/hello.0 --out .zero/out/hello
```

The repository validation commands are:

```sh
npm run conformance
npm run native:test
npm run docs:test
ZERO_BENCH_RUNS=1 npm run bench
```
