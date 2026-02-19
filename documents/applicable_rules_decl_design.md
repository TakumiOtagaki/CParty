# Applicable Rules Declaration Design (prep)

## Goal
Move `applicable_rules_*` from per-nonterminal hand-written loops into a data-driven path based on `RuleSpec` so that:
- Rule applicability is specified in one place (`rules_catalog.cc`).
- Split enumeration and predicate checks are uniform and testable.
- fixed-structure parsing and PF inside-core share the same rule filtering logic.

## Current State (2026-02-19)
- `RuleSpec` already carries:
  - `split_gen` (Custom / KRange / BandMinBpRange / BandMaxBpRange)
  - `split_range`
  - `split_filter` (CanPairLeft / CanPairRight / WmbpExteriorSplit / WmbpInnerArcSplit / BeSplitPairWithin)
  - `predicate` and `span_predicate`
- However `applicable_rules_*` still contains per-rule logic for:
  - split enumeration ranges (custom loops)
  - per-split conditions (inline if-guards)
  - span guards (e.g., BE validity)

## Target Model (two-phase filter)
1. **Span guard**
   - Evaluate `span_predicate_allows(spec, span_ctx, ctx)` once per rule.
   - This is the only place to reject invalid span-level inputs (e.g., BE pair topology).

2. **Split enumeration**
   - If `SplitKind::None`: single split `{}`.
   - If `SplitGenKind::KRange`: enumerate `k` from `split_range` (with `turn` if `subtract_turn`).
   - If `BandMinBpRange` / `BandMaxBpRange`: use `min_bp/max_bp` from the band oracle.
   - If `Custom`: delegate to a rule-specific generator (kept for the few remaining exceptions).

3. **Split filtering and predicate**
   - For each generated split, apply `split_filter` (if any).
   - Then apply `predicate_allows(spec, split_ctx, ctx)`.
   - If both pass, emit `(rule, split)`.

## Required Context Interfaces
- `sparse_tree` / `StructureView`: pk-free and fixed-structure paths.
- `PartFuncRuleHelpers` / `PartFuncRuleHelpersView`: band path (WMBP/WMBW/WMB).
- `PartFunc*Context`: for `turn`, `min_bp/max_bp`, and other band bounds.

## Known Missing Declarative Conditions (from audit)
These are currently inline and should become `split_filter` or `predicate` entries:
- `WM_START_V / WM_START_WMB`: `can_pair_left_span(i,k)` condition.
- `WIP_BASEPAIR_*`: `can_pair_left_span(i,k)` condition.
- `W_SPLIT_V / W_SPLIT_WMB`: `weakly_closed` guards on `(1,j)` and `(1,k-1)`.
- `V_HAIRPIN`: `canH` loop-length guard (can be a `PredicateKind::VHairpinMinLoop`).
- `VP_INTERNAL_LOOP`: `is_unpaired/is_empty_region` and `pair_type` checks inside split loop.
- `BE` rules: need clear split-level predicates separated from span predicates.

## Prep Work (staged)
1. **Spec coverage extension**
   - Add `SplitFilterKind` / `PredicateKind` entries for the missing guards.
   - Mark each target rule in `rules_catalog.cc` with the new declarative fields.

2. **Generic enumerators**
   - Extend `enumerate_splits_k_range` to optionally accept a split filter callback.
   - Provide a shared band-range enumerator (already in `band_vp.cc`) to be reused.

3. **Unified applicability helper**
   - Introduce a `collect_applicable_rules(specs, ctx, split_gen, split_filter, predicate)` helper.
   - Keep per-nonterminal `applicable_rules_*` but refactor them to call the helper.

4. **Incremental migration**
   - Start with pk-free (`W/WI/V/VM`) where predicates are already mostly declarative.
   - Then migrate band (`WIP/VPL/VPR/VP`) using `split_gen` + `split_filter`.
   - Finally migrate WMB/BE where split filters are most specialized.

## Guardrails
- No behavior change should occur without a strict compare run.
- When adding declarative predicates, prefer new enums over inline checks.
- Keep `Custom` split gen as an escape hatch until the last stage.

