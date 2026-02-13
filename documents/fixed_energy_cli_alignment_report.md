# Fixed Energy CLI Alignment Report

## Scope
- Gate test: `api_cli_density2_energy_alignment`
- Baseline: `test/baselines/fixed_energy_cli/density2_mfe.tsv`
- Runtime mode: `-d2` only
- Run date: `2026-02-13`

## Metrics (emitted by test)
- `alignment_compared`
- `alignment_mismatched`
- `skipped`
- `finite_rate`
- `max_abs_diff`
- `max_rel_diff`

## Anti-Cheat Conditions
- `alignment_compared=0` is immediate fail.
- Metrics are printed on every run for auditability.

## Strict Story-019 Results
- `refactor_compared=152`
- `refactor_strict_mismatched=0`
- `alignment_compared=152`
- `alignment_mismatched=0`
- `skipped=0`
- `finite_rate=100%`
- `max_abs_diff=0`
- `max_rel_diff=0`

## Diff Summary
- Absolute diff summary: all compared rows are exact matches (`max_abs_diff=0`).
- Relative diff summary: all compared rows are exact matches (`max_rel_diff=0`).
