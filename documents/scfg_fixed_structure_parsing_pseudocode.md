# SCFG Fixed-Structure Parsing Pseudocode

## Goal
固定構造 `G∪G'` を入力として、`(state, [i,j])` ごとに適用可能な唯一の規則を機械的に選択し、規則列に沿ってスコアを集計する。

## Required Interfaces
- `rules_for(state)`: 状態に対応する規則候補を返す
- `applicable_rules(state, i, j, context)`: 構造制約に合う規則候補のみ返す
- `rule_score(rule, state, i, j, context)`: 局所スコアを返す
- `expand(rule, state, i, j, context)`: 子状態列を返す

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
