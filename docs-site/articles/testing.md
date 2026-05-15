## Testing And Reliability

`zero test` runs test blocks from a single `.0` file or every source file in a package. The test runner is part of the main CLI workflow and reports structured JSON for supported test workflows.

Common commands:

```sh
zero test conformance/native/pass/test-blocks.0
zero test --json --filter addition conformance/native/pass/test-blocks.0
zero test --json conformance/packages/test-app
npm run reliability:smoke
npm run native:sanitize
```

Package tests discover test blocks across the package entry file and local modules. Filtered tests use substring matching on the test name, which keeps CI shards simple and deterministic.

Expected-fail tests are named with `xfail:`, `expected fail:`, or `[xfail]`. A failing expected-fail test reports as `expected-fail` and increments `expectedFailures`; a passing expected-fail test reports as `unexpected-pass`, increments `unexpectedPasses`, and makes the command fail.

## JSON Contract

`zero test --json` is intended for CI, editors, and repair agents. The stable fields include:

```text
ok
sourceFile
target
testBackend
selectedTests
discoveredTests
passedTests
failedTests
expectedFailures
unexpectedPasses
durationMs
stdout
stderr
testDiscovery
fixtures
targetFacts
results
```

`testDiscovery` records the discovery mode, filter, package root, manifest path, source file count, module count, discovered count, and selected count. `fixtures.sourceFiles` lists the source files that were loaded, `fixtures.goldenOutput` records the deterministic text output, and `fixtures.snapshotKey` identifies the snapshot contract for review.

Each result contains the test name, status, expected-fail flag, duration, source location, and failure span when a failure is available.

## Golden And Snapshot Workflow

Golden rows compare command output against exact expected output. Snapshot rows compare structured metadata or formatted source against the checked-in contract. Review snapshot changes like API changes: confirm the behavior change first, then update the fixture or expected JSON in the same change that implements it.

The reliability smoke writes `.zero/reliability-smoke/report.json` with golden, snapshot, fuzz, and crasher rows. It covers package tests, filtered tests, expected-fail behavior, formatter snapshots, stdlib parser helpers, generated fuzz fixtures, and minimized crash repros.

## Fuzz And Crasher Repros

Fuzz harnesses should be small, deterministic, and source-backed:

- Parser fuzzing should run `zero tokens --json` and `zero parse --json` on generated `.0` files.
- Checker fuzzing should run `zero check --json` and assert diagnostics for invalid programs instead of accepting crashes.
- Formatter fuzzing should compare `zero fmt` output against stable source snapshots.
- Stdlib parser fuzzing should use module examples such as `examples/std-data-formats.0` and inspect `usedStdlibHelpers`.

When a compiler crash is found, reduce it to a single fixture under conformance or a documented smoke repro, then keep the minimized input in the reliability report until a focused regression test owns it.

`npm run native:sanitize` remains the native hardening gate. Keep sanitizer failures, fuzz failures, and crasher repros visible in CI rather than hiding them behind best-effort local scripts.
