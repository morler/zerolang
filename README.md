# Zero

Zero is the programming language for agents: a systems language for small native tools, explicit effects, predictable memory, and structured compiler output.

Zero is experimental and still changing. The compiler, standard library, docs, and examples are useful for trying the language and giving feedback, but the language is not stable yet.

## Quick Start

Build the local compiler from a checkout:

```sh
npm install
make -C native/zero-c
bin/zero --version
```

Check a program:

```sh
bin/zero check examples/hello.0
```

Build and run a small executable:

```sh
bin/zero build --emit exe --target linux-musl-x64 examples/add.0 --out .zero/out/add
./.zero/out/add
```

Expected output:

```text
math works
```

## Learn Zero

- `docs-site/articles/getting-started.md`: build the compiler and run a first program.
- `docs-site/articles/learn-zero.md`: a practical tour of the language.
- `docs-site/articles/language-reference.md`: syntax and behavior reference.
- `examples/README.md`: examples grouped by concept.

Run the docs site locally:

```sh
npm run docs:dev
```

## Common Commands

```sh
bin/zero check examples/hello.0
bin/zero build --emit exe --target linux-musl-x64 examples/add.0 --out .zero/out/add
bin/zero graph --json examples/systems-package
bin/zero size --json examples/point.0
bin/zero routes --json examples/web/hello
bin/zero doctor --json
```

## Validation

```sh
npm run docs:test
npm run conformance
npm run native:test
npm run command-contracts
```

Benchmarks run locally by default:

```sh
npm run bench
```

## Repository Layout

- `native/zero-c/`: native compiler implementation.
- `compiler-zero/`: Zero-authored compiler sources.
- `examples/`: runnable Zero source examples.
- `conformance/`: language and CLI behavior fixtures.
- `docs-site/`: documentation site.
- `tests/`: TypeScript tests for CLI behavior.
- `extensions/vscode/`: editor syntax highlighting for `.0` files.
