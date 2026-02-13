# SCFG Fixed-Structure Parsing Pseudocode

## Goal
固定構造 `G∪G'` を入力として、`(state, [i,j])` ごとに適用可能な唯一の規則を機械的に選択し、規則列に沿ってスコアを集計する。

## Required Interfaces
- `rules_for(state)`: 状態に対応する規則候補を返す
- `applicable_rules(state, i, j, context)`: 構造制約に合う規則候補のみ返す
- `rule_score(rule, state, i, j, context)`: 局所スコアを返す
- `expand(rule, state, i, j, context)`: 子状態列を返す

## Canonical State Registry (Story 013)
- Total target states: `16`
- Canonical states:
  `W, WI, V, VM, WM, WMv, WMp, WIP, VP, VPL, VPR, WMB, WMBP, WMBW, BE, ZW`
- Source of truth in code:
  - `cparty::internal::fixed_energy_target_states()`
  - `cparty::internal::fixed_energy_rollout_plan()`

## Rollout Story Mapping (Machine-Checkable)
- `013`: `ZW` (parser scaffold baseline)
- `014` (slice A): `W, WI, V`
- `015` (slice B): `VM, WM, WMv, WMp`
- `016` (slice C): `WIP, VP, VPL, VPR`
- `017` (slice D): `WMB, WMBP, WMBW, BE`

## Slice Test Registration (Source-Controlled)
- `fixed_energy_slice_a`
- `fixed_energy_slice_b`
- `fixed_energy_slice_c`
- `fixed_energy_slice_d`

## Pseudocode
```text
function get_structure_energy(seq, fixed_db):
    norm = normalize_input(seq, fixed_db)              # length/alphabet/brackets validation
    ctx  = build_context(norm)                         # pair-map, topology tags, helpers
    parser = SCFGParser(grammar, oracle, scorer, ctx)

    total = 0
    stack = [ Item("ZW", 1, norm.n) ]

    while stack not empty:
        cur = stack.pop()
        if is_terminal(cur):
            continue

        rules = parser.rules_for(cur.state)
        candidates = parser.applicable_rules(rules, cur.state, cur.i, cur.j, ctx)

        if len(candidates) != 1:
            fail invalid_input_contract with details(cur, candidates)

        rule = candidates[0]
        total += parser.rule_score(rule, cur.state, cur.i, cur.j, ctx)

        children = parser.expand(rule, cur.state, cur.i, cur.j, ctx)
        push children to stack in deterministic order

    return total
```

## Notes
- `applicable_rules` は `is_empty_region`, `is_pair_type_allowed`, `is_transition_allowed` を使って判定する。
- valid 入力では規則選択が一意、invalid 入力では `invalid_input_contract` で失敗する。
- `EnergyBreakdown` は規則列に沿った `rule_score` の集計結果として埋める。
