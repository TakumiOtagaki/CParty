# Can-Pair Rollout Report

## Scope
This report summarizes final can-pair rollout verification across fixed-energy API, pseudo-loop, and partition-function paths.

## Changed Decision Points
- `src/can_pair_policy.hh` and `src/can_pair_policy.cc`: shared can-pair hard-constraint policy for allowed base pairs and tree-up span checks.
- `src/pseudo_loop_can_pair.hh` and `src/pseudo_loop_can_pair.cc`: pseudo-loop recurrence/backtracking pair gating routed through shared policy helpers.
- `src/part_func_can_pair.hh` and `src/part_func_can_pair.cc`: partition-function recurrence/backtracking pair gating routed through shared policy helpers.
- `tests/can_pair_cross_path_consistency_test.cc`: one shared acceptance/rejection matrix asserting API/helper outcome alignment.

## Regression and Integration Gate
The following tests were executed together in one gate:
- `regression_matrix`
- `can_pair_policy`
- `pseudo_loop_can_pair_integration`
- `pseudo_loop_can_pair_integration_helpers`
- `part_func_can_pair_integration`
- `part_func_can_pair_integration_helpers`
- `can_pair_cross_path_consistency`

## Performance Smoke Checks
| case | elapsed_seconds | parsed_mfe_energy |
| --- | ---: | ---: |
| pkfree_r | 0 | -1.14 |
| h_type_r_k | 0 | -6.69 |
| k_type_r_k_d0 | 0 | -12.05 |

## Remaining Limitations
- CLI constrained `-k` handling still has known deterministic failure modes for some fixture/flag combinations, tracked in `tests/baselines/fixed_energy_cli/density2_mfe.tsv`.
- Representability remains bounded by current CParty grammar; unsupported structures are intentionally rejected (`NaN`) by fixed-energy API validation.
- Smoke timing here is a sanity gate on representative inputs, not a full benchmark suite.
