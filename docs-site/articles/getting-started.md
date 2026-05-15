## Getting Started

Zero is the programming language for agents. The fastest way to try it today is
to build the compiler from this repository and run the included examples.

The project is still early, so treat these commands as a hands-on preview rather
than a stable production install path.

## Build The Compiler

From the repository root:

```sh
npm install
make -C native/zero-c
bin/zero --version
```

`npm` installs docs and test tooling. The compiler itself is built by `make` and
written to `.zero/bin/zero`; `bin/zero` is the repository wrapper.

## Check Your First File

Create `hello.0`:

```zero
pub fun main(world: World) -> Void raises {
    check world.out.write("hello from zero\n")
}
```

Run the checker:

```sh
bin/zero check hello.0
```

The important parts are:

- `pub fun main(...)` declares the program entry point.
- `world: World` is the capability object passed to the program by the runtime.
- `world.out.write(...)` writes through that explicit capability.
- `check` handles a fallible operation.
- `raises` marks that `main` can return an error.

Zero makes effects visible. A program that writes output asks for `World`
instead of reading a hidden global process object.

## Build And Run An Executable

Create `add.0`:

```zero
fun answer() -> i32 {
    return 40 + 2
}

pub fun main(world: World) -> Void raises {
    let value = answer()
    if value == 42 {
        check world.out.write("math works\n")
    } else {
        check world.out.write("math broke\n")
    }
}
```

Build and run it:

```sh
bin/zero build --emit exe --target linux-musl-x64 add.0 --out .zero/out/add
./.zero/out/add
```

Expected output:

```text
math works
```

This example introduces a helper function, a local binding, and `if` / `else`.

## Create A Package

The project workflow starts with `zero new`:

```sh
bin/zero new cli hello
cd hello
../bin/zero check
../bin/zero test
../bin/zero build --target linux-musl-x64 --out .zero/out/hello
```

Single files are useful for learning, but real Zero projects use a `zero.json`
manifest and source files under `src/`.

## Learn The Core Syntax

Work through these examples in order:

```sh
bin/zero check examples/hello.0
bin/zero check examples/hello-let.0
bin/zero check examples/functions.0
bin/zero check examples/branch.0
bin/zero check examples/point.0
bin/zero check examples/result-choice.0
```

They cover:

- entry points and output
- `let` bindings
- functions and return values
- conditionals
- `shape` data declarations
- `enum`, `choice`, and `match`

## Inspect A Package

The CLI package example lives in `examples/systems-package`:

```text
examples/
  systems-package/
    src/
      main.0
      helpers.0
      types.0
    zero.json
```

Check it:

```sh
bin/zero check examples/systems-package
```

Inspect its module graph:

```sh
bin/zero graph --json examples/systems-package
```

The manifest tells Zero where the entry point lives:

```json
{
  "package": { "name": "systems-package", "version": "0.1.0" },
  "targets": { "cli": { "kind": "exe", "main": "src/main.0" } }
}
```

## Next Steps

- Read Learn Zero for a practical language tour.
- Use the examples index to pick the next example by concept.
- Use Building From Source when you want to validate a local checkout.
- Use the language reference once you have written a few small programs.
