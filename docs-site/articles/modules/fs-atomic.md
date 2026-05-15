## Status

Specified, not runnable yet.

Planned surface:

| Area | Purpose |
| --- | --- |
| Replace | Write a temporary file and atomically rename it over the destination. |
| Durability | Let callers choose whether file and directory syncs are required. |
| Cleanup | Remove temporary files when replacement fails. |
| Diagnostics | Report target and filesystem limitations. |

## Preview

```zero
use std.fs.atomic

pub fun save_config(fs: Fs, path: Path, bytes: Span<const u8>) -> Void raises {
    check std.fs.atomic.replaceFile(fs, path, bytes, durability: .fileAndDirectory)
}
```

## Design Notes

This module is an atomic rename helper, not a transaction protocol. It should keep durability choices visible because filesystems and targets differ.
