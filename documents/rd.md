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
- `seq`: RNA sequence (`ACGU` only; `T` is treated as invalid input in this phase).
- `db_full`: dot-bracket for full target (`G union G'`), including PK brackets.

### Output
- `double energy_kcal`: fixed-structure free energy in `kcal/mol`.
- Return `NaN` on invalid input:
  - length mismatch
  - invalid sequence symbols
  - invalid/unbalanced structure
  - structure not representable under current CParty constraints

### Error/Exception Contract
- The API is non-throwing for validation/representability failures and returns `std::numeric_limits<double>::quiet_NaN()`.
- Tests and callers must validate invalid results using `std::isnan(...)`.
- Existing exception behavior in unrelated legacy APIs must remain unchanged.

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

## 9. Numeric Tolerance Policy
- Use shared tolerance settings for regression and deterministic checks:
  - `abs_tol = 1e-6`
  - `rel_tol = 1e-6`
- Equality checks for floating-point outputs must use:
  - `abs(a - b) <= max(abs_tol, rel_tol * max(abs(a), abs(b)))`
- Exact string equality is not required for floating-point logs/reports.

## 10. Dot-Bracket and Invalidity Rules
- `db_full` must match sequence length exactly.
- Accept unpaired symbol `.` and paired symbols under this rule:
  - canonical pair bracket `()`
  - exactly one PK bracket type from `[]`, `{}`, `<>` according to current CParty parser configuration
  - mixing multiple PK bracket families in one `db_full` is invalid
- If the structure uses unsupported bracket symbols or crossing patterns not representable by current CParty constraints, return `NaN`.
- Invalid input examples that must return `NaN`:
  - length mismatch between `seq` and `db_full`
  - any non-`ACGU` symbol in `seq` (including `T`)
  - unbalanced bracket counts
  - use of unsupported PK bracket family for the active parser configuration
  - mixing two or more PK bracket families in one structure
  - representability failure under current CParty grammar

## 10.1 Base-Pair Policy
- Allowed canonical/wobble pairs for hard constraints:
  - `A-U`, `U-A`
  - `C-G`, `G-C`
  - `G-U`, `U-G`
- Any other forced pair in `db_full` must be rejected as invalid (`NaN`).

## 11. Loop Safety Policy (for ralph-loop)
- Per-story retry policy:
  - default `retry_limit = 2`
  - Story 13 (final e2e validation) uses `retry_limit = 1`
- Global stop conditions:
  - stop if the same failure signature occurs 3 times consecutively
  - stop if total failed attempts reaches 12 in one run
  - stop immediately on infrastructure/configuration failures (test framework broken, fixture schema unreadable, build configuration broken)
- On stop, emit a short failure summary including failing story id, last command, and failure signature.

## 12. Story Execution and Test Gates
- Story selection order is deterministic: choose the smallest story `id` with `passes: false` whose `depends_on` stories are all `passes: true`.
- `passes` may be set to `true` only when all of the following hold:
  - all `done_when` items are satisfied
  - required `artifacts` exist or are updated
  - story `test_command` passes
  - full regression command `ctest --output-on-failure` passes
- After every refactoring and every feature-addition story, run:
  - the story-local test command
  - full regression command
- Every 3 completed stories in Phase-2, run a checkpoint:
  - focused code review
  - refactoring sanity check
  - full `ctest --output-on-failure`

## 12.1 CLI-MFE Baseline Policy
- CParty CLI reported density-2 MFE structure/energy may be used as a practical regression baseline source.
- For baseline generation, always persist:
  - exact CLI flags (`-p`, `-k`, `-d`, `-r`)
  - sequence and structure string used
  - parsed structure/energy fields from stdout
- Interpretation rule:
  - treat this baseline as an implementation-consistency oracle for CParty paths
  - do not treat it as an external physical ground truth oracle

## 13. Current can-pair Coverage Clarification
- Current can-pair hard-constraint enforcement is not only a ViennaRNA constraint setting.
- In the current implementation, can-pair is enforced by:
  - API/input validation and representability checks in `get_structure_energy`
  - internal fixed-structure evaluator validation before energy evaluation
  - evaluator-side pair validation prior to energy calculation
- This stage primarily guarantees can-pair behavior for the fixed-structure API path.
- It does not yet guarantee identical can-pair enforcement semantics across all CParty algorithmic paths (`pseudo_loop`, `part_func`) in every recurrence branch.

## 14. Phase-2 Goal (Full-Algorithm Consistency)
- Next phase goal: apply can-pair hard constraints consistently across:
  - fixed-structure API path
  - `pseudo_loop` recurrences
  - `part_func` recurrences
- Required strategy:
  - refactor long recurrence files into helper units first
  - centralize can-pair policy in one shared helper
  - add cross-path consistency tests for accept/reject decisions
  - keep full regression green after each incremental step
