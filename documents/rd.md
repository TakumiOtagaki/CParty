# CParty Fixed-Structure Energy Requirements

## 1. Goal
Implement a fixed-structure energy path in CParty that returns:

`E(G union G', S)` in `kcal/mol`

This is required for:

`logP(G' | G, S) = -E(G union G', S) / RT - logZ_cond`

where `logZ_cond` comes from the existing conditional partition-function machinery.

## 2. Key Clarifications
- `CParty -r` is a constraint mode, not a fixed-structure energy evaluator.
- `final_energy` and `pf_energy` from CLI output are not the same as `E(fixed structure)`.
- This work is CParty-internal first. HFold/HotKnots are optional sanity checks, not ground truth for acceptance.

## 3. Scope
- Add internal APIs to evaluate the energy of one provided structure.
- Keep existing MFE/PF behavior unchanged.
- Refactor with behavior-preserving commits before adding new logic.

Out of scope for this stage:
- Full parameter matching with HFold/HotKnots.
- Multi-layer pseudoknot generalization beyond current CParty grammar.

## 4. API Requirements
### Input
- `seq`: RNA sequence (`ACGU`; accept `T` by normalizing to `U`).
- `db_full`: dot-bracket for full target (`G union G'`), including PK brackets.

### Output
- `double energy_kcal`: fixed-structure free energy in `kcal/mol`.
- Return `NaN` on invalid input:
  - length mismatch
  - invalid sequence symbols
  - invalid/unbalanced structure
  - structure not representable under current CParty constraints

## 5. Refactoring Requirements
- Start with regression test coverage.
- Refactor `src/pseudo_loop.cc` first in small behavior-preserving steps.
- Refactor `src/part_func.cc` next to isolate responsibilities:
  - partition-function path
  - fixed-structure evaluation path
- Avoid large multi-file rewrites in one commit.

## 6. Testing Strategy (TDD)
Three layers:

1. Regression layer
- Preserve current CLI behavior (`-p`, `-k`, `-d`, with/without `-r`).
- Verify non-regression in structure validity and numeric stability.

2. Fixed-energy API layer
- Deterministic output for same input.
- Finite output for valid inputs.
- `NaN` for invalid inputs.
- Include PK-free and density-2 PK examples.

3. Diagnostic layer (breakdown)
- Add optional internal `EnergyBreakdown` for debugging:
  - pk-free core terms
  - PK penalties
  - band-spanning scaled terms
  - total
- This is for debugging and test observability, not a stable public interface yet.

## 7. Acceptance Criteria
- Existing behavior remains stable under regression tests.
- New fixed-structure API returns deterministic finite values on valid cases.
- Invalid inputs fail predictably (`NaN`).
- Case set in `tests/cparty_fixed_structure_energy/test.multi.fa` is runnable end-to-end.

## 8. Notes for Future Comparison
- Cross-tool energy comparison with HFold/HotKnots may differ due to model and decomposition differences.
- Use cross-tool checks only as coarse sanity checks unless full parameter/decomposition alignment is implemented.
