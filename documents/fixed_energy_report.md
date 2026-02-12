# Fixed Energy End-to-End Report

Source fixture: `tests/cparty_fixed_structure_energy/test.multi.fa`

## Observed Energies

| id | family | expected | observed_energy_kcal_mol |
| --- | --- | --- | --- |
| pkfree_stem | pk_free | valid | 4.5 |
| pkfree_hairpin | pk_free | valid | -5 |
| h_type_1 | h_type | valid | -12.76 |
| k_type_1 | k_type | valid | -22.88 |
| invalid_length_mismatch | invalid | invalid | NaN |
| invalid_sequence_symbol | invalid | invalid | NaN |
| invalid_unbalanced_structure | invalid | invalid | NaN |
| invalid_mixed_pk_families | invalid | invalid | NaN |
| invalid_unsupported_pk_family | invalid | invalid | NaN |

## Unsupported/Invalid Cases

| id | reason |
| --- | --- |
| invalid_length_mismatch | length_mismatch |
| invalid_sequence_symbol | invalid_sequence_symbol |
| invalid_unbalanced_structure | unbalanced_structure |
| invalid_mixed_pk_families | mixed_pk_families |
| invalid_unsupported_pk_family | unsupported_pk_family |
