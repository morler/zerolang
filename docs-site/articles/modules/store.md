## Status

Deferred until file capabilities, buffer handling, mutation, and target semantics mature.

Planned surface:

| Area | Purpose |
| --- | --- |
| WAL | Encode and validate append-only records. |
| Pages | Define fixed-size page headers and checksums. |
| Segments | Manage segment metadata without prescribing a database engine. |
| Snapshots | Describe compact snapshot formats and validation rules. |

## Preview

```zero
use std.store

pub fun append_record(writer: mutref<Writer>, record: Span<const u8>) -> Void raises {
    let header = std.store.wal.header(record)
    check writer.write(header.bytes())
    check writer.write(record)
}
```

## Design Notes

The module should expose boring storage building blocks, not a database. APIs must keep allocation, file effects, and durability boundaries explicit.
