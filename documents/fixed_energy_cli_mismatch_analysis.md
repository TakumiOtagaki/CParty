# Fixed Energy CLI Mismatch Analysis

Date: 2026-02-12  
Inputs:
- `documents/fixed_energy_cli_alignment_report.md`
- `tests/baselines/fixed_energy_cli/density2_mfe.tsv`

## Mismatch Summary
- Comparable rows (`status=ok`, `exit_code=0`): 22
- Mismatched rows: 16
- Skipped rows (`exit_code!=0`): 2 (`pkfree_hairpin` with `-k` / `-k -d0`, both `139`)

## Status Update (After Story 23 Partial Work)
- `EnergyEvalOptions` / `EnergyEvalContext` types and API overload were introduced (`get_structure_energy(seq, db_full, options)`).
- Alignment test now maps CLI flags (`-p`, `-k`, `-d0`) into API options before comparison.
- Remaining mismatch drivers are therefore no longer "API has no options input", but production-path divergence (CLI core path vs API path), model-parameter divergence, and scorer/representation differences.
- Current execution state: Story 23/24 are treated as re-validation pending until staged non-strict gates are re-run cleanly under the updated PRD policy.

## Mismatch Categories
- Production-path option/context mismatch: shared options exist, but CLI main execution path and API fixed-energy path are not yet unified through one production scorer/context pipeline.
- Model-term mismatch: CLI default path loads Dirks/Pierce parameters while fixed-structure API path loads Turner 2004 parameters.
- Implementation-path mismatch: fixed-structure API currently calls `W_final::hfold` (search under constraints) instead of a direct fixed-structure scorer.
- PK constraint representation mismatch: API parser converts `[]` to `.` in `tree_structure` used to build `sparse_tree`, which can relax PK-specific tree constraints in the scoring path.

## Per-Row Attribution (Current Mismatches)
Note: The per-row notes below were first captured before production-path unification. Option-threading observations should be revalidated after Story 24 implements shared scorer routing.

| fixture_id | flag_id | delta (api - cli) | category | Cause hypothesis | Code pointers |
| --- | --- | ---: | --- | --- | --- |
| pkfree_hairpin | r1_p0_k0_d2 | -1.55 | model-term + implementation-path | Same structure still differs; likely parameter-set mismatch and non-direct scoring path. | `src/CParty.cc:165`, `src/fixed_structure_energy_internal.cc:57`, `src/fixed_structure_energy_internal.cc:181` |
| pkfree_hairpin | r1_p0_k0_d0 | -1.55 | option/context + model-term + implementation-path | API hardcodes dangle=2, so `-d0` context is dropped in addition to base path mismatches above. | `src/CParty.cc:142`, `src/fixed_structure_energy_internal.cc:179`, `src/fixed_structure_energy_internal.cc:181` |
| pkfree_hairpin | r1_p1_k0_d2 | -1.55 | option/context + model-term + implementation-path | `-p` context is not represented in API evaluation options; API always uses `pk_free=false`. | `src/CParty.cc:138`, `src/fixed_structure_energy_internal.cc:177`, `src/CPartyAPI.cc:330` |
| pkfree_hairpin | r1_p1_k0_d0 | -1.55 | option/context + model-term + implementation-path | Same as previous row, plus `-d0` dropped by API path. | `src/CParty.cc:138`, `src/CParty.cc:142`, `src/fixed_structure_energy_internal.cc:177` |
| h_type_1 | r1_p0_k0_d2 | -6.07 | model-term + implementation-path + PK representation | Large PK gap suggests parameter mismatch plus API path differences for PK structures (`[]` rewritten to `.` in tree). | `src/fixed_structure_energy_internal.cc:57`, `src/fixed_structure_energy_internal.cc:109`, `src/fixed_structure_energy_internal.cc:120`, `src/fixed_structure_energy_internal.cc:181` |
| h_type_1 | r1_p0_k0_d0 | -6.07 | option/context + model-term + implementation-path + PK representation | Same as above; also `-d0` ignored by API. | `src/CParty.cc:142`, `src/fixed_structure_energy_internal.cc:179` |
| h_type_1 | r1_p1_k0_d2 | -8.05 | option/context + model-term + implementation-path | CLI `-p` row changes structure/energy context, but API still evaluates with fixed non-`-p` options. | `src/CParty.cc:138`, `src/CParty.cc:199`, `src/CPartyAPI.cc:349` |
| h_type_1 | r1_p1_k0_d0 | -8.92 | option/context + model-term + implementation-path | Largest H-type gap includes missing `-p` and `-d0` context plus shared scorer differences. | `src/CParty.cc:138`, `src/CParty.cc:142`, `src/fixed_structure_energy_internal.cc:177` |
| h_type_1 | r1_p0_k1_d2 | -6.07 | option/context + model-term + implementation-path + PK representation | `-k` flag context is not represented in API options; PK representation/scoring path still differs. | `src/CParty.cc:139`, `src/fixed_structure_energy_internal.cc:177`, `src/fixed_structure_energy_internal.cc:109` |
| h_type_1 | r1_p0_k1_d0 | -6.07 | option/context + model-term + implementation-path + PK representation | Same as previous with `-d0` dropped by API. | `src/CParty.cc:139`, `src/CParty.cc:142`, `src/fixed_structure_energy_internal.cc:179` |
| k_type_1 | r1_p0_k0_d2 | -10.83 | model-term + implementation-path + PK representation | Very large K-type gap indicates parameter and path mismatch amplified on PK topologies. | `src/fixed_structure_energy_internal.cc:57`, `src/fixed_structure_energy_internal.cc:109`, `src/fixed_structure_energy_internal.cc:120`, `src/fixed_structure_energy_internal.cc:181` |
| k_type_1 | r1_p0_k0_d0 | -10.83 | option/context + model-term + implementation-path + PK representation | Same as previous with missing `-d0` option threading. | `src/CParty.cc:142`, `src/fixed_structure_energy_internal.cc:179` |
| k_type_1 | r1_p1_k0_d2 | -13.51 | option/context + model-term + implementation-path | `-p` changes CLI structure context but API still uses one fixed evaluator mode. | `src/CParty.cc:138`, `src/CParty.cc:199`, `src/fixed_structure_energy_internal.cc:177` |
| k_type_1 | r1_p1_k0_d0 | -14.47 | option/context + model-term + implementation-path | Largest overall gap includes missing `-p`/`-d0` options plus shared scorer mismatch. | `src/CParty.cc:138`, `src/CParty.cc:142`, `src/fixed_structure_energy_internal.cc:177` |
| k_type_1 | r1_p0_k1_d2 | -10.83 | option/context + model-term + implementation-path + PK representation | `-k` context is not threaded into API evaluation and PK path is not aligned. | `src/CParty.cc:139`, `src/fixed_structure_energy_internal.cc:177`, `src/fixed_structure_energy_internal.cc:109` |
| k_type_1 | r1_p0_k1_d0 | -10.83 | option/context + model-term + implementation-path + PK representation | Same as previous with `-d0` option mismatch. | `src/CParty.cc:139`, `src/CParty.cc:142`, `src/fixed_structure_energy_internal.cc:179` |

## Prioritized Fix Order (for Stories 23+)
1. Complete Story 23 in production paths: wire CLI main fixed-energy path to the same `EnergyEvalOptions`/`EnergyEvalContext` contract (current state is partial).
2. Unify parameter loading policy for alignment tests (same `.par` source in both paths) to remove baseline model-set skew.
3. Replace API fixed-energy `W_final::hfold` reuse with one direct fixed-structure scoring routine used by both CLI alignment flow and API.
4. Preserve PK bracket constraints in the evaluator tree/context instead of rewriting `[]` to `.` for PK rows.

## Acceptance for Next Stage
- This analysis is sufficient to proceed with Story 23 completion and Story 24 implementation because dominant mismatch classes are concrete, code-pointed, and prioritized.
