## Standard Library Reference

Zero's standard library is pay-as-used and capability-aware. Importing memory helpers does not pull in hosted filesystem helpers, and hosted APIs report their target requirements in `zero graph` and `zero size`.

Runnable modules:

- `std.mem`: spans, byte equality, copy/fill, fixed-buffer allocators, byte buffers, and arena-style reset helpers.
- `std.io`: buffered reader/writer metadata and byte copy helpers over caller-owned storage.
- `std.args`: hosted process argument count and indexed lookup.
- `std.env`: hosted environment variable lookup.
- `std.fs`: hosted file lifecycle helpers, owned file handles, byte reads/writes, remove, rename, and close.
- `std.path`: fixed-buffer path joining.
- `std.parse`: allocation-free ASCII scanners and unsigned integer parsers.
- `std.codec`: byte-oriented integer encoding, varint length, and CRC-32 helpers.
- `std.json`: validation, streaming token counts, explicit-allocator parsing, and caller-buffer string writing.
- `std.time`: duration math plus target-gated monotonic and wall-clock helpers.
- `std.rand`: explicit deterministic random sources and target entropy helpers.
- `std.proc`: host process status helpers behind the process capability.

Each module page documents target support, allocation behavior, error behavior, ownership notes, and runnable examples. Use `zero graph --json <input>` to inspect required capabilities, `zero size --json <input>` to inspect helper metadata, and `zero mem --json <input>` to inspect `memoryBudgets`, `allocatorFacts`, `allocationInstrumentation`, and `collectionFacts`. The `stdlibHelpers` and `usedStdlibHelpers` JSON entries include `module`, `effects`, `allocationBehavior`, `targetSupport`, `errorBehavior`, `ownershipNotes`, `example`, and `apiStability` for each public helper.

## Metadata Contract

Public standard library symbols document the fields agents need to call them safely:

```text
symbol: std.fs.readAllOrRaise
effects: fs
allocation behavior: caller allocator
target support: host
error behavior: raises { NotFound, TooLarge, Io }
ownership notes: returns owned<ByteBuf>
example: examples/readall-cli/
```

Module pages may group related symbols when their metadata is identical, but they should keep these labels visible: effects, allocation behavior, target support, error behavior, ownership notes, and example.
