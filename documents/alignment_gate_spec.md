# Alignment Gate Specification

## Purpose
fixed-structure API と CLI density-2 baseline の一致を機械判定する。

## Inputs
- Baseline TSV: `test/baselines/fixed_energy_cli/density2_mfe.tsv`
- Test cases: mandatory seed (`test/multi.secstruct`) + generated valid cases

## Required Metrics
- `alignment_compared`: comparison actually performed
- `alignment_mismatched`: number of mismatching compared rows
- `skipped`: rows excluded before comparison
- `finite_rate`: finite-valued ratio on valid density-2 rows

## Strict Conditions
- `alignment_compared >= 100`
- `alignment_mismatched = 0`
- `finite_rate = 100%`
- `alignment_compared = 0` is immediate FAIL

## Mismatch Definition
A row is mismatched if either:
1. compared structure differs where structure comparison is required, or
2. energy difference exceeds tolerance:
   - `abs(cli_energy - api_energy) > abs_tol` AND
   - relative difference exceeds `rel_tol`

## Notes
- `alignment_mismatched` is different from `refactor_strict_mismatched`.
- Keep metric names unchanged in logs and reports.
