## Status

Specified, not runnable yet.

Planned surface:

| Area | Purpose |
| --- | --- |
| UUID | Parse, format, and generate UUID values. |
| Sortable IDs | Generate time-sortable IDs when a clock and randomness are available. |
| NanoID-style buffers | Fill caller-provided buffers without hidden allocation. |
| Typed IDs | Wrap raw ID values in domain-specific types. |

## Preview

```zero
use std.id

shape UserId {
    value: std.id.Uuid,
}

pub fun new_user_id(rand: Rand) -> UserId raises {
    return UserId { value: check std.id.uuidV4(rand) }
}
```

## Design Notes

ID generation depends on explicit capabilities such as `Rand` and sometimes `Clock`. Formatting should support caller-provided buffers so small tools and web handlers avoid surprise heap use.
