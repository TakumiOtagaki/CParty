# Fixed Energy CLI Alignment Report

Baseline source: `tests/baselines/fixed_energy_cli/density2_mfe.tsv`

## Summary

- compared: 22
- skipped: 2
- mismatched: 16
- abs_tol: 1e-06
- rel_tol: 1e-06

## Compared Rows (`status=ok` and `exit_code=0`)

| fixture_id | flag_id | flags | cli_mfe_energy | api_structure_energy | within_tolerance |
| --- | --- | --- | --- | --- | --- |
| pkfree_stem | r1_p0_k0_d2 |  | 0 | 0 | yes |
| pkfree_stem | r1_p0_k0_d0 | -d0 | 0 | 0 | yes |
| pkfree_stem | r1_p1_k0_d2 | -p | 0 | 0 | yes |
| pkfree_stem | r1_p1_k0_d0 | -p -d0 | 0 | 0 | yes |
| pkfree_stem | r1_p0_k1_d2 | -k | 0 | 0 | yes |
| pkfree_stem | r1_p0_k1_d0 | -k -d0 | 0 | 0 | yes |
| pkfree_hairpin | r1_p0_k0_d2 |  | -3.45 | -5 | no |
| pkfree_hairpin | r1_p0_k0_d0 | -d0 | -3.45 | -5 | no |
| pkfree_hairpin | r1_p1_k0_d2 | -p | -3.45 | -5 | no |
| pkfree_hairpin | r1_p1_k0_d0 | -p -d0 | -3.45 | -5 | no |
| h_type_1 | r1_p0_k0_d2 |  | -6.69 | -12.76 | no |
| h_type_1 | r1_p0_k0_d0 | -d0 | -6.69 | -12.76 | no |
| h_type_1 | r1_p1_k0_d2 | -p | -4.71 | -6.5 | no |
| h_type_1 | r1_p1_k0_d0 | -p -d0 | -3.84 | -4.8 | no |
| h_type_1 | r1_p0_k1_d2 | -k | -6.69 | -12.76 | no |
| h_type_1 | r1_p0_k1_d0 | -k -d0 | -6.69 | -12.76 | no |
| k_type_1 | r1_p0_k0_d2 |  | -12.05 | -22.88 | no |
| k_type_1 | r1_p0_k0_d0 | -d0 | -12.05 | -22.88 | no |
| k_type_1 | r1_p1_k0_d2 | -p | -9.37 | -13.9 | no |
| k_type_1 | r1_p1_k0_d0 | -p -d0 | -8.41 | -11.7 | no |
| k_type_1 | r1_p0_k1_d2 | -k | -12.05 | -22.88 | no |
| k_type_1 | r1_p0_k1_d0 | -k -d0 | -12.05 | -22.88 | no |

## Skipped Rows (nonzero CLI exit)

| fixture_id | flag_id | flags | exit_code | reason |
| --- | --- | --- | --- | --- |
| pkfree_hairpin | r1_p0_k1_d2 | -k | 139 | cli_exit_nonzero_status_error |
| pkfree_hairpin | r1_p0_k1_d0 | -k -d0 | 139 | cli_exit_nonzero_status_error |
