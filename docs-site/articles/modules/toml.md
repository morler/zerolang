## Status

Staged after payload choices and owned documents.

Planned surface:

| Area | Purpose |
| --- | --- |
| Parser | Convert TOML text into an owned document. |
| Values | Inspect strings, integers, floats, booleans, arrays, tables, and dates. |
| Config access | Read nested values with clear missing-key diagnostics. |
| Diagnostics | Report source spans, table paths, and duplicate keys. |

## Preview

```zero
use std.toml

pub fun load_version(alloc: Alloc, bytes: Span<const u8>) -> String raises {
    let document = check std.toml.parse(alloc, bytes)
    defer document.drop()

    return check document.string("package.version")
}
```

## Design Notes

TOML should stay config-shaped, not become a general database or dynamic object framework. Diagnostics need source spans because configuration files are edited by people and agents.
