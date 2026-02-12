# Agent Learnings

## Regression Patterns
- For part-function can-pair integration stages, route pair-formation checks through `part_func_can_pair::can_form_allowed_pair` and keep `tree.up` span checks delegated to shared policy helpers so recurrence and backtracking stay semantically aligned.
- For pseudo-loop can-pair integration stages, guard recurrence/backtracking pair formation with one shared helper (`can_form_allowed_pair`) and route internal `tree.up` emptiness checks through helper predicates so unsupported pairs are blocked consistently.
- For phase-2 part-function refactors, route every `tree.up` span predicate through a tiny `part_func_can_pair` helper so recurrence and backtracking gates share identical can-pair checks and can be unit-tested.
- For phase-2 DP refactors, extract `tree.up` can-pair span checks and CP-penalty math into a tiny shared helper so branch gating is unit-testable without full end-to-end folding runs.
- For CLI baseline datasets that include unstable flag combinations, persist `exit_code` and `status` metadata so deterministic failures (for example SIGSEGV paths) are regression-testable without masking parser checks on successful cases.
- For debug-only diagnostic stages, expose an internal breakdown struct with stable term groups and assert `total == pk_free_core + pk_penalties + band_scaled_terms` within tolerance.
- CTest matrix tests should set an explicit `WORKING_DIRECTORY` when CLI output depends on runtime paths.
- For deterministic CLI regression checks, run each case twice and compare exact stdout before baseline comparison.
- Keep shell test parsers portable across GNU/BSD tools; avoid non-portable awk extensions in default macOS environments.
- Fixed-structure fixtures are easier to validate and reuse when normalized in TSV with explicit `expected` and `invalid_reason` columns.
- For behavior-preserving DP refactors, split large recurrence functions by branch family and keep border computations centralized in the caller.
- For partition-function refactors, keep `hfold_pf` as a thin orchestrator over explicit stage helpers (DP fill, exterior accumulation, output finalization).
- Internal fixed-energy APIs are easiest to harden when validation (sequence/brackets/can-pair) is kept separate from the energy kernel and fixture-driven tests assert finite vs `NaN` contracts.
- For declaration-only API stages, use a compile-time signature test (`decltype` + `static_assert`) so header contracts are verified without forcing link-time behavior changes.
- For staged public API rollout, add a boundary normalizer/validator first and assert `std::isnan` contracts before evaluator wiring to avoid coupling multiple stories.
- For staged fixed-energy API rollout, gate `get_structure_energy` by feature level (PK-free first) and assert API/internal evaluator equality on enabled paths.
- For staged PK rollout, classify `[]` pseudoknot topology at the API boundary so H-type can be enabled while K-type remains an explicit `NaN` path.
- When enabling a previously staged-out topology (such as K-type), update earlier stage tests that asserted `NaN` so full regression remains forward-compatible.
- For compatibility gates, assert legacy API outputs before and after new API calls to detect cross-API state regressions.
- End-to-end fixed-energy checks are easiest to keep auditable when a fixture-driven test both enforces finite/`NaN` contracts and writes a markdown report of observed energies plus unsupported reasons.
