# Agent Learnings

## Regression Patterns
- CTest matrix tests should set an explicit `WORKING_DIRECTORY` when CLI output depends on runtime paths.
- For deterministic CLI regression checks, run each case twice and compare exact stdout before baseline comparison.
- Keep shell test parsers portable across GNU/BSD tools; avoid non-portable awk extensions in default macOS environments.
- Fixed-structure fixtures are easier to validate and reuse when normalized in TSV with explicit `expected` and `invalid_reason` columns.
- For behavior-preserving DP refactors, split large recurrence functions by branch family and keep border computations centralized in the caller.
- For partition-function refactors, keep `hfold_pf` as a thin orchestrator over explicit stage helpers (DP fill, exterior accumulation, output finalization).
