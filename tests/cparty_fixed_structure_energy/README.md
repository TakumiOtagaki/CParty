# Fixed-Structure Energy Fixtures

This directory defines normalized fixtures for fixed-structure energy development.

## Schema
Fixture source of truth is `fixtures.tsv` with tab-separated columns:

1. `id`: unique case id
2. `family`: `pk_free`, `h_type`, `k_type`, or `invalid`
3. `expected`: `valid` or `invalid`
4. `seq`: RNA sequence (A/C/G/U/T only)
5. `db_full`: full dot-bracket structure
6. `invalid_reason`: `-` for valid fixtures, required reason tag for invalid fixtures

## Validation rules
- Valid fixtures must:
  - have matching `seq` and `db_full` length
  - contain only supported symbols
  - be balanced/parseable for each bracket family
  - use only `.` and `()` for `pk_free`
  - use exactly one PK family (`[]`) for `h_type`/`k_type`
- Invalid fixtures must:
  - set `family=invalid`
  - set `expected=invalid`
  - include an explicit reason tag
  - fail at least one schema/parseability rule

## Invalid reason tags
- `length_mismatch`
- `invalid_sequence_symbol`
- `unbalanced_structure`
- `mixed_pk_families`
- `unsupported_pk_family`

## Derived file
`test.multi.fa` is a FASTA-style representation of the same fixture set for later end-to-end checks.
