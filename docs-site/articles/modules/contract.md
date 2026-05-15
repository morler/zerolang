## Status

Specified, not runnable yet.

Planned surface:

| Area | Purpose |
| --- | --- |
| Issues | Store validation failures with stable codes and value paths. |
| Paths | Represent bounded paths such as `user.email` or `items[3].name`. |
| Reports | Accumulate and render validation results. |
| Checks | Provide common predicates for required fields, ranges, lengths, and formats. |

## Preview

```zero
use std.contract

pub fun validate_user(report: mutref<Report>, user: User) -> Void {
    std.contract.required(report, "email", user.email)
    std.contract.minLen(report, "name", user.name, 1)
}
```

## Design Notes

Validation APIs should be useful for CLIs, config files, web requests, and agent repair loops without pulling in JSON, TOML, HTTP, or a formatting runtime by default.
