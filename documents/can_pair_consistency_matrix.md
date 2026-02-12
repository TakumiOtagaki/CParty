# Can-Pair Cross-Path Consistency Matrix

This matrix defines representative accept/reject cases for can-pair hard constraints across:
- `get_structure_energy(seq, db_full)` API path
- `pseudo_loop_can_pair::can_form_allowed_pair(seq, left, right)`
- `part_func_can_pair::can_form_allowed_pair(seq, left, right)`

Acceptance semantics:
- API: finite energy => accept, `NaN` => reject
- pseudo_loop/part_func helpers: `true` => accept, `false` => reject

| Case ID | Family | seq | db_full | Pair (1-based) | Expected |
| --- | --- | --- | --- | --- | --- |
| `pk_free_accept` | PK-free | `GCAUGC` | `(....)` | `(1,6)` | Accept |
| `pk_free_reject_can_pair` | PK-free | `AAAAAA` | `(....)` | `(1,6)` | Reject |
| `h_type_accept` | H-type | `GGGCAAAAGGCGAAAAGCCCAAAACGCC` | `((((....[[[[....))))....]]]]` | `(9,28)` | Accept |
| `h_type_reject_can_pair` | H-type | `AAAAAAAAAAAAAAAAAAAAAAAAAAAA` | `((((....[[[[....))))....]]]]` | `(9,28)` | Reject |
| `k_type_accept` | K-type | `GGCGGAAAAACCGGCAAAAACCGCCAAAAACGGGUAAUAAGCUGGAAAAAGCCCG` | `(((((.....[[[[[.....))))).....(((((.....]]]]].....)))))` | `(13,43)` | Accept |
| `k_type_reject_can_pair` | K-type | `AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA` | `(((((.....[[[[[.....))))).....(((((.....]]]]].....)))))` | `(13,43)` | Reject |
| `invalid_symbol_reject` | Invalid | `XCAUGC` | `(....)` | `(1,6)` | Reject |

The automated gate is implemented in `tests/can_pair_cross_path_consistency_test.cc` and runs as CTest `can_pair_cross_path_consistency`.
