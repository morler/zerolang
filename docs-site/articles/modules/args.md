## Status

Runnable today:

| API | Return | Notes |
| --- | --- | --- |
| `std.args.len()` | `usize` | Returns the process argument count. |
| `std.args.get(index)` | `Maybe<String>` | Returns the argument at `index` when present. |

Specified but not complete:

- Iterator-style argument APIs.
- Typed decoding helpers.
- Target diagnostics for platforms without process arguments.

## Example

```zero
pub fun main(world: World) -> Void raises {
    let count = std.args.len()
    let first = std.args.get(1)
    if count > 1 && first.has {
        check world.out.write(first.value)
        check world.out.write("\n")
    }
}
```

## Design Notes

The module is hosted-only. Freestanding, edge, and embedded targets should reject it unless they explicitly provide an argument capability.
On native Windows-style targets, `std.args` is documented as hosted process-argument access, but cross-target executable behavior depends on the selected C backend and is not a Unicode argv normalization layer. Programs that need portable argument semantics should treat returned `String` values as byte-oriented process input and keep target-specific decoding outside the target-neutral core.
