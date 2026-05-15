## Status

Deferred until atomic semantics and target capability rules mature.

Planned surface:

| Area | Purpose |
| --- | --- |
| Cancellation | Pass explicit cancellation state through capability-aware APIs. |
| Latches | Coordinate one-time readiness or shutdown. |
| Backpressure | Track bounded work without requiring an executor. |
| Queues | Provide bounded queues where atomics and target support allow them. |
| In-flight keys | Deduplicate concurrent work by key. |

## Preview

```zero
use std.sync

pub fun fetch_once(ctx: Cancel, keys: mutref<InFlightKeys>, key: String) -> Void raises {
    let token = check std.sync.enter(keys, key)
    defer token.leave()
    check ctx.raiseIfCancelled()
}
```

## Design Notes

`std.sync` should not smuggle in an event loop or scheduler. Concurrency stays explicit through capabilities and target-supported primitives.
