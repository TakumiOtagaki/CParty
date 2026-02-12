# Agent Learnings

## Regression Patterns
- CTest matrix tests should set an explicit `WORKING_DIRECTORY` when CLI output depends on runtime paths.
- For deterministic CLI regression checks, run each case twice and compare exact stdout before baseline comparison.
- Keep shell test parsers portable across GNU/BSD tools; avoid non-portable awk extensions in default macOS environments.
