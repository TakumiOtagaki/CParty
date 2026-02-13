# Fixed Energy CLI Alignment Report

## Scope
- Gate test: `api_cli_density2_energy_alignment`
- Baseline: `test/baselines/fixed_energy_cli/density2_mfe.tsv`
- Runtime mode: `-d2` only

## Metrics (emitted by test)
- `alignment_compared`
- `alignment_mismatched`
- `skipped`
- `finite_rate`

## Anti-Cheat Conditions
- `alignment_compared=0` is immediate fail.
- Metrics are printed on every run for auditability.

## Notes
- Story 006 installs strict metric reporting and anti-cheat checks.
- Final strict target (`alignment_mismatched=0`, `alignment_compared>=100`, `finite_rate=100%`) is enforced in later convergence stories.
