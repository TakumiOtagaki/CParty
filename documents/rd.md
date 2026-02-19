# CParty SCFG Refactoring Plan (V2, Strict)

## 1. 目的
CParty の非曖昧な密度2分解ロジックを、次の3用途で共通利用できる形に分離する。
- Partition Function: 全パスのボルツマン重み和
- Fixed-Structure Energy: 指定した単一構造パスのエネルギー評価
- MFE Prediction: 最小エネルギーパスの選択

## 2. フェーズ0: 評価基盤の先行整備 (最優先)
実装変更の前に、比較可能なベースラインと機械判定ゲートを用意する。

### 2.1 APIプロトタイプ
- `get_structure_energy(seq, db_full)` を試験導入する。
- まずは `-d2` 固定で評価する。
- 入力文字は `A,U,G,C` のみを受理し、`T` は明示的に失敗させる。

### 2.2 機械的DoD (厳格)
以下を `ctest` で自動判定する。
- `alignment_compared >= 100`
- early phase: `alignment_mismatched` を必ず報告し、比較が機械的に実行されること
- final strict gate: `alignment_mismatched = 0` (X=0)
- `finite_rate == 100%` (valid density-2 ケースのみ)
- invalidケース契約: 不正入力は単一の失敗契約（固定エラーコード or 例外型）で決定論的に失敗する
- 数値比較は `abs_tol` と `rel_tol` を明記する

### 2.2.1 指標の意味（誤解防止）
- `refactor_strict_mismatched`: リファクタ前後で既存CLI挙動がずれた件数（挙動不変チェック用）。
- `alignment_mismatched`: fixed-structure API と CLI density-2 baseline の不一致件数（機能整合チェック用）。
- `refactor_strict_mismatched` と `alignment_mismatched` は別物として常に分離して記録する。

### 2.3 生成データの数学的前提
- ランダム生成は「解析用文法」ではなく「テストデータ生成器」として扱う。
- input 二次構造 G の生成規則は `S -> S '.' | S '(' S ')' | epsilon`（PK-free）とし、塩基対は `AU/GC/GU` のみ許容する。
- 生成器は固定seedで再現可能にし、同一seed・同一設定で同一データを再生成できることを保証する。

### 2.4 反チート実行規約
- テストは必ず fresh build で実行する (`rm -rf build && cmake -S . -B build && cmake --build build`)。
- `ctest` は `build/CTestTestfile.cmake` の生成時刻が当該コミット後であることを確認してから実行する。
- `api_cli_density2_energy_alignment` は「比較件数0件」で pass してはならない。`compared` を標準出力またはレポートに必ず出力し、`compared >= 100` を機械判定する。
- DoD 判定は source-controlled なテストコード/データのみを対象にし、`build/` 配下の既存生成物を正解として扱わない。
- ドキュメントだけ変更して `passes: true` にしてはならない。コード変更とテスト証跡を同一コミット群で提示する。

## 3. フェーズ1-2: 挙動不変リファクタリング
`src/part_func.cc`, `src/pseudo_loop.cc` の再帰ロジックを、式を変えずに関数/オブジェクトへ抽出する。

### 3.1 分離対象
- 16状態 (`ZW`, `ZV`, `ZP`, `ZVP` ほか) を状態単位で分離
- Constraint Oracle を独立させ、空区間判定・対合種判定・構造整合判定を一本化

### 3.2 検証方針
- `static_assert` ではなく実行時 `ctest` で数値一致を監視
- 早期段階は report-gate (差分可視化中心)
- 後期段階は strict-gate (しきい値超過でfail)
- 各リファクタ直後に、seed固定データセットに対する CLI 出力（構造・エネルギー）の一致チェックを必須化する
- 抽出方針は「3回以上出る具体操作の関数化を優先」とし、広すぎる抽象語先行の分離は避ける
- 命名は具体責務を表す（例: `is_empty_region`, `is_pair_type_allowed`, `is_transition_allowed`）

## 4. フェーズ3: ロールバック条件
外部ツール完全一致ではなく、内部ベースライン逸脱をロールバック条件にする。
- Internal Baseline Drift が `abs_tol/rel_tol` を超えたらロールバック
- ViennaRNA との比較は参考値として扱い、ロールバック条件にはしない

## 5. フェーズ4: 固定構造評価器の統合
Rule Object + Constraint Oracle を使って単一パス評価を実装する。
- 入力構造から決定論的にルール列を特定
- `estP=0.89`, `eintP=0.74` などの係数適用を一貫化
- 実装は同一DPテーブル/同一コア上で行い、適用範囲（pk_free/h_type/k_type）を段階的に検証する
- パーサ実装は `rules_for`, `applicable_rules`, `rule_score`, `expand` の分離インターフェースで進める
- 詳細擬似コードは `documents/scfg_fixed_structure_parsing_pseudocode.md` を参照

## 5.1 進捗メモ (2026-02-16 以降)
- `rules_core` を新設し、`rules_for` を RuleSpec テーブル起点に統一
- `SplitKind` を `None / K / KL` に簡略化
- `rules_runtime` から `rules_core` への接続を **全非終端で完了**
  - `V/W/VM/WI/WMv/WMp/WM/WIP/VPL/VPR/VP/WMBW/WMBP/WMB/BE`
- `WMB` の `i==j` は **WMB_EMPTY のみ**に制限（legacy 互換）
- `get_structure_energy_union(seq, G, G')` を追加
  - `G` は `()` のみ、`G'` は `[]` のみを許可
  - `G ∪ G'` を fixed-structure path に合成して評価
- `fixed_energy_union_test` を追加し、`union` と `merged` の一致を確認

### 5.1.1 進捗メモ (2026-02-18)
- `rule_score_*` を **`transition_weight_*`** に統一（呼び出し側も同名へ置換）
- inside コア側の子参照 API を **`get_<NonTerminal>`** で統一（`get_energy*` を廃止）
- `rules_runtime` を `inside_fill` に改名・分割（`basic.cc`, `band.cc`, `core.cc`）
- `rules_core` を `src/scfg/rules_core/` 配下に分割
- `rules_engine` を `rules_config` + `rules_debug` に分割
- `rules_part_func` の補助系を `rules_part_helpers` に分割（legacy 本体は保持）
- **統合コンテキスト** `PartFuncAllContext` を追加（`get_inside(NonTerminal,i,j)` / `get_BE(i,ip,j,jp)` を提供）
- `SCFG_INSIDE_CORE=1` で core 実装を有効化する分岐を追加
- core 実装は **ルール列挙 + transition_weight** で書き直し済み
  - `V/W/WI/VM/WMv/WMp/WM`
  - band 側: `WIP/VPL/VPR/VP/WMBW/WMBP/WMB/BE`
- strict compare で **600/600 match** を継続確認（例）
```
CPARTY_FIXED_ENERGY_REAL_SCORE=1 \
SCFG_RULES_MODE=1 SCFG_RULES_APPLICABLE=1 SCFG_INSIDE_CORE=1 \
test/tools/compare_cli_stdout.sh worktree_legacy_debug/build/CParty build/CParty 42 200 30,50,80
```
- 子補正のルール吸収:
  - `WM` / `VM` の子補正は遷移重みに集約済み
  - band 側（`WIP/VPL/VPR/VP/WMB*/BE`）も遷移重みに集約済み（子側補正は無し）

### 5.1.2 進捗メモ (2026-02-18)
- legacy 実装を明示化:
  - `rules_part_func.cc` を `legacy_rules_part_func.cc` に改名
  - 旧実装は `legacy_compute_*_restricted` にリネーム
  - adapter の legacy fallback 経由でのみ利用
- **Density2View 実体注入の準備**
  - `SCFG_DENSITY2_VIEW=1` で `round/square` を分離構築して注入
  - 既定は従来どおり `Density2View(tree, tree)` を維持
- **Rule catalog の外部化**
  - `rules_core/base.cc` → `rules_catalog.cc`
  - `rule_catalog()` を追加（RuleSpec テーブル参照用）
- **transition_weight の抽象化**
  - `TransitionWeights` を導入し、重み計算を ctx 直接参照から分離

### 5.1.3 SCFG 抽象化の現状と不足点 (2026-02-19)
#### 現状
- inside core は rule/split 列挙 + applicable 分岐を統合済み（`for_each_entry`）。
- `RuleSpec` に RHS を追加し、`expand_*` を RHS 優先に移行済み（W/WI/VM/WIP/VPL/VPR/VP/WMB/WMBP/WMBW/BE）。
- StructureView の round/square 分離はデフォルト ON（フラグで OFF 可能）。
- strict compare は `SCFG_RULES_MODE=1` + `SCFG_RULES_APPLICABLE=1` + `SCFG_INSIDE_CORE=1` で継続一致。

#### 不足点 / ギャップ
1. RHS は expand 置換に留まり、真の文法テーブル化（適用条件/分割生成の data-driven 化）が未完。
2. `TransitionWeights` は導入済みだが、EnergyModel/Oracle の責務分離は未完。
3. StructureView の完全注入はフラグ運用段階（常時分離の最終化は未実施）。
4. 空区間 (`allow_empty`) の仕様化が未完（明示的な空区間表現/契約が未定）。
5. `rules_catalog.cc` が肥大化しつつあり、将来的な分割・生成手段の検討余地あり。

#### 次に詰めるべき論点
- `RuleSpec` に RHS だけでなく適用条件/分割生成の宣言情報を持たせるか。
- `TransitionWeights` を EnergyModel / Oracle インタフェースに昇格するか。
- 空区間の明示的表現と契約を定義するか。

### 5.1.4 SCFG 抽象化の進捗メモ (2026-02-19)
#### Split/Predicate の宣言化
- `RuleSpec` に split 生成 (K-range / band min-max range) と predicate を段階導入。
- K-range: `WI_SPLIT_*`, `VM_SPLIT_WM_*`, `WIP_SPLIT_*` を宣言化。
- band range: `VPL_SPLIT_VP`, `VPR_SPLIT_VP_*`, `VP_WIP_*`, `VP_VPL_WIP` を宣言化。
- predicate: `V`, `W/WI` の unpaired、`VP_STACK / VP_INTERNAL_LOOP / VP_WI_CASE*`,
  `VPR_SPLIT_VP_BASEPAIR`, `WMBP_*`, `WMB_SPLIT_BE_WMBP_WI`, `BE_*` の条件を宣言化。

#### RHS の整理
- W/WI/VM/WIP/VPL/VPR/VP/WMB/WMBP/WMBW/BE は RHS テーブルを優先使用。
- `allow_empty` は必要最小限に絞り、理由コメントを付与。
- RHS 定義は NonTerminal ごとにグルーピング。

#### 残課題
- `applicable_rules` の宣言化（predicate/split の統一的評価）を進める。
- band 側の複合条件（WMBP の外側境界・inner arc 判定など）は未宣言化。
- 空区間の仕様（allow_empty を明示表現へ移行するか）を確定する。

### 5.1.5 predicate 宣言の適用漏れ監査メモ (2026-02-19)
#### 監査結果（現状）
明示的な `PredicateKind::None` の誤適用は見つからず。既存の差分は「per-split 条件」を inline で実装しているケースに集約。

#### 宣言化されていない適用条件（代表）
- `WM_START_V / WM_START_WMB` の `can_pair_left_span(i,k)` 条件は `enumerate_splits_wm` に inline。
- `WIP_BASEPAIR_*` の `can_pair_left_span(i,k)` 条件は `enumerate_splits_wip` に inline。
- `W_SPLIT_V / W_SPLIT_WMB` の `weakly_closed` 条件は `enumerate_splits_w` に inline。
- `V_HAIRPIN` の `canH`（最小ループ長）判定は `enumerate_splits_v` に inline。
- `VP_INTERNAL_LOOP` の `is_unpaired/is_empty_region` と `pair_type` チェックは split 探索中に inline。

#### 次の候補（宣言化へ寄せるなら）
- K-range 用の `SplitFilterKind` 対応を `enumerate_splits_k_range` に追加する（`can_pair_*` を宣言化可能に）。
- per-split predicate を `RuleSpec` に導入するか、`predicate_allows(spec, ctx)` を split 評価の内側で呼ぶ方針に揃える。
- `weakly_closed`/`canH` のような span-only 条件は `PredicateKind` へ追加できる余地あり。

### 5.1.6 BE split 条件の宣言化（設計案メモ, 2026-02-19）
#### 現状の課題
- `enumerate_splits_be` は **span 全体の妥当性チェック**（`i/ip/j/jp` の整合・ペア一致・範囲）と、
  **split ごとの条件**（`l/lp` のペア位置・空区間・weakly_closed 等）を手続き内に混在させている。
- `RuleSpec` は predicate を 1 個しか持てないため、**共通の span ガード**と**ルール個別の split 条件**を同時に宣言できない。

#### 方向性（宣言化の構造）
- **SpanPredicate / SplitPredicate を分離**する。
  - `RuleSpec` に `span_predicate` を追加し、BE 系は全ルール共通で `BeSpanValid` を付与。
  - `predicate` は split レベルの条件（`BeStackPairing` など）に限定する。
- あるいは **複合 predicate** を導入し、`BeSpanValid + BeStackPairing` のような組み合わせを enum 化する。
  - ただし enum が爆発しやすい。

#### 追加で必要になるもの
- `RuleSpanContext` に **`n` (上限)** を入れるか、`predicate_allows` に `ctx.n()` 相当を渡せる設計にする。
  - これがないと `i <= ip < jp <= j <= n` のチェックを predicate 側で表現できない。
- BE の split 判定は `l/lp` に依存するため、**split ループ内で predicate を呼ぶ方式**が必須。

#### 最小移行手順案
1. `RuleSpec` に `span_predicate` を追加（BE 以外は `None`）。
2. BE ルールに `span_predicate=BeSpanValid` を設定。
3. `enumerate_splits_be` の先頭で `span_predicate` 判定を実行し、手続き側のガードを削減。
4. split ループ内では既存の `predicate` を呼び続ける（段階移行）。

### 5.1.7 外部 SCFG API の試作（2026-02-19）
#### 追加
- `cparty::scfg::parse_fixed_energy(seq, G, G')` を追加。
  - 入力は `normalize_union_input` を再利用（`G` は `()`, `G'` は `[]` 前提）。
  - エネルギーは既存の fixed-energy scorer で算出。
  - trace は pk_free / h_type で rules_core trace を優先し、それ以外は shared trace を返す。

#### 既知の制限
- rule chain からの **遷移重み再計算**は未実装（context/oracle の本格実装が必要）。
- k-type の場合は shared trace のみ（rules_core trace は未対応）。

## 5.2 k-type 設計メモ (2026-02-17)
### 5.2.1 前提
- k-type = `()` と `[]` が混在し、相互に交差しうる
- `[]` を band（pseudoknot）側とみなす
- pk-free / h-type は既存 `sparse_tree` で継続運用

### 5.2.2 B/Bp/b/bp の解釈（pk-free）
`sparse_tree` の実装より：
- `bp(i,l)` = 区間 `[i,l]` における「最も内側の左境界」（未ペア `l` の親の左端）
- `Bp(l,j)` = 区間 `[l,j]` における「最も内側の右境界」（未ペア `l` の親の右端）
- `b(i,l)` = 区間 `[i,l]` における「最も外側の左境界」（LCA 起点の外側）
- `B(l,j)` = 区間 `[l,j]` における「最も外側の右境界」（LCA 起点の外側）

### 5.2.3 k-type 構造 view（草案）
- `round_tree` = `()` のみで構成する `sparse_tree`
- `square_tree` = `[]` のみで構成する `sparse_tree`（band 側）
- B/Bp/b/bp は `square_tree` 由来とする
- `is_unpaired(i)` は round/square の両方で未ペアのとき true
- `weakly_closed(i,j)` は band view（square）の結果を使用

### 5.2.4 rules_core 関数ごとの view マッピング（草案）
※ “band view” は `square_tree` を指す
- `enumerate_splits_w` / `enumerate_splits_wi`:
  - `weakly_closed` は band view
  - `tree[j].pair < 0` 判定は **any-pair**（round or square）で未ペアなら true とする
- `enumerate_splits_v` / `rule_score_v`:
  - **round view**
  - `pair(i,j)` は round のみで評価（`V` は通常の塩基対を表すため）
- `enumerate_splits_vm` / `enumerate_splits_wmv_wmp` / `enumerate_splits_wm`:
  - **round view**
  - `can_pair_left_span/right_span` は round view
- `enumerate_splits_wip` / `enumerate_splits_vpl` / `enumerate_splits_vpr` / `enumerate_splits_vp`:
  - **band view**（`square_tree` の B/Bp/b/bp を使用）
  - `pair_at(i)` / `parent_index(i)` は band view
  - `is_unpaired_position` は **any-pair** で未ペアを判定
- `enumerate_splits_wmbw` / `enumerate_splits_wmbp` / `enumerate_splits_wmb` / `enumerate_splits_be`:
  - **band view**
  - `weakly_closed` / `parent_index` / `pair_at` は band view
  - `is_empty_region` は any-pair の空区間判定が必要

### 5.2.5 次の実装ステップ（予定）
- k-type 用の `StructureView` / `Oracle` を用意（round/square の dual-tree を内包）
- rules_core の `enumerate_splits_*` に view を注入できるよう抽象化
- k-type 専用 path を追加し、slice D から段階導入

### 5.2.6 StructureView / Oracle API（草案 v0）
**目的**: pk-free/h-type と k-type を同一インターフェースで扱う。

#### A) 基本 API
- `n()`: 長さ
- `pair_any(i)`: round/square どちらかの相手。未ペアなら <0
- `pair_round(i)` / `pair_square(i)`
- `is_pair_round(i,j)` / `is_pair_square(i,j)`
- `is_unpaired(i)`: round/square 両方で未ペア

#### B) band view（k-type の square を使用）
- `parent_index(i)`: band 側 parent（square）
- `weakly_closed(i,j)`: band view（square）
- `b(i,j)`, `bp(i,j)`, `B(i,j)`, `Bp(i,j)`: band view 境界

#### C) round view（通常の塩基対）
- `unpaired_prefix(i)`: VM/ML などで使用（round）
- `can_pair_left_span(i,k)` / `can_pair_right_span(k,j)`: round view

#### D) empty 判定
- `is_empty_region(i,j)`: any-pair（round or square）で空判定

#### E) 実装モデル
- `PkFreeView`: 既存 `sparse_tree` をそのまま利用
  - `pair_square(i)` は常に <0
  - `weakly_closed`/B 系は round と同義
- `Density2View`: `round_tree` + `square_tree` の dual-tree
  - band view は `square_tree`
  - round view は `round_tree`

#### メモ: Density2View に round_tree / square_tree の実体を渡す必要性
- 現状は `Density2View(tree, tree)` で pk-free と同じ情報を渡しているだけなので、k-type/h-type の band 情報（`[]`）が構造判定に反映されない。
- legacy の pseudo-loop 系（`WMB/BE/VP` など）は band 側境界（`B/Bp/b/bp` や `weakly_closed`）に依存するため、`[]` を `square_tree` に分離して渡さないと専用エネルギー項が欠落する。
- 対応方針:
  - `round_tree`: `()` のみ抽出して構築
  - `square_tree`: `[]` のみ抽出して構築
  - `Density2View(round_tree, square_tree)` を rules/runtime 経由で使用

#### F) rules_core との接続方針
- `enumerate_splits_*` は view を受け取る設計に改修
- k-type では view が提供する関数で分岐し、tree 直接参照を避ける

## 6. 実行制約
- 当面は `-d2` のみを対象に実装・検証する
- オプション差分 (`-p`, `-k`, `-r`, `-d0`) は別フェーズで扱う
- 3ステップごとに code review + refactor + 全テスト通過確認を挟む

## 7. 完了条件 (Final DoD)
- `ctest -R api_cli_density2_energy_alignment` が実データで実行される
- `alignment_compared` が 100 以上で、`alignment_mismatched=0`
- valid density-2 ケースで `finite_rate=100%`
- invalidケース契約テストが全件 pass
- 既存回帰テストを全通過
- 実行ログに `alignment_compared`, `alignment_mismatched`, `skipped`, `finite_rate`, `refactor_strict_mismatched` を出力し、レビューで再計算可能であること

## 8. 現在の実測ステータス (2026-02-13, Story 020)
- Fresh build directory: `build_story020_fresh_20260213_run1`
- Full regression: `ctest --test-dir build_story020_fresh_20260213_run1 --output-on-failure` は `13/13` pass
- Alignment gate metrics (`ctest -V -R api_cli_density2_energy_alignment`):
  - `refactor_compared=152`
  - `refactor_strict_mismatched=0`
  - `alignment_compared=152`
  - `alignment_mismatched=0`
  - `skipped=0`
  - `finite_rate=100%`
  - `max_abs_diff=0`
  - `max_rel_diff=0`
- 補足: 実行環境ポリシーで `rm -rf build` が拒否されるため、ストーリー実行では一意な fresh build ディレクトリを使用して anti-stale 条件を満たした。

## 9. リファクタ作業フロー (2026-02-15 追記)
以下は分割リファクタ時の基本フロー。原則として「機械的抽出 → build → strict test → commit」を徹底する。

### 9.1 再ビルド・Strict Test
```
cmake --build build
test/tools/compare_cli_stdout.sh worktree_legacy/build/CParty build/CParty 42 200 30,50,80
```

### 9.2 コミット運用
- 変更が小さく機械的な単位になるように分割し、各ステップでコミットする。
- 例:
```
git add <changed files...>
git commit -m "<short summary>"
```

## 10. 分割進捗まとめ (2026-02-15)
`src/part_func.cc` の分割を段階的に実施。すべての段階で build と strict test を通過。

### 10.1 反映済みの分割内容
- `boustrophedon` ヘルパを `src/part_func.hh` の `inline` に移動
- BPP 関連:
  - `src/part_func_bpp.cc`
- サンプリング系:
  - `src/part_func_sample.cc`
  - `pair[]` 参照のため TU 内で `make_pair_matrix()` を一度だけ呼ぶガード追加
- エネルギー系:
  - `src/part_func_energy.cc`
  - `pair[]` 参照のため TU 内で `make_pair_matrix()` を一度だけ呼ぶガード追加
- PK 計算系:
  - `src/part_func_pk.cc`
  - `pair[]` 参照のため TU 内で `make_pair_matrix()` を一度だけ呼ぶガード追加
- コア計算系:
  - `src/part_func_core.cc`
  - `pair[]` 参照のため TU 内で `make_pair_matrix()` を一度だけ呼ぶガード追加
- rescale 系:
  - `src/part_func_rescale.cc`
- driver 系:
  - `src/part_func_driver.cc`
  - `pair[]` 参照のため TU 内で `make_pair_matrix()` を一度だけ呼ぶガード追加
- init 系:
  - `src/part_func_init.cc`
  - `pair[]` 参照のため TU 内で `make_pair_matrix()` を一度だけ呼ぶガード追加
- `compute_exterior_cases` は `src/part_func_pk.cc` に移動
- `src/part_func.cc` は分割完了後に削除

### 10.2 CMake 更新
以下を `CMakeLists.txt` の `SOURCE` に追加:
- `src/part_func_bpp.cc`
- `src/part_func_sample.cc`
- `src/part_func_energy.cc`
- `src/part_func_pk.cc`
- `src/part_func_core.cc`
- `src/part_func_rescale.cc`
- `src/part_func_driver.cc`
- `src/part_func_init.cc`
- `src/part_func.cc` を削除

## 11. 既知の課題 (2026-02-16)
- MEA の backtrack が `CL/CLPK` 空のケースで失敗する可能性があり、stderr に `backtrack failed` を出して継続する挙動が既存。
- 対応案: CLPK 候補に由来種別を持たせ、backtrack 分岐と整合させる。
- 進捗: `debug-mea-clpk` で候補種別化を実装し、再現ケースでは失敗が解消。strict test は pass。広い検証は未完。
- Issue: `https://github.com/TakumiOtagaki/CParty/issues/2`
### 11.1 Strict Test と MEA 出力の扱い
- `compare_cli_stdout.sh` は stdout の「1〜3行目（seq / restricted / MFE）」のみを比較する。
- MEA/centroid などの追加行は比較対象外で、stderr の `backtrack failed` も gate に影響しない。
### 11.2 MEA/centroid の確率入力
- 現実装の MEA/centroid は、厳密な bpp 行列ではなく `samples` の出現頻度（`count / num_samples`）をペア確率として利用している。

## 12. SCFG ルール棚卸しメモ (2026-02-16)
`src/scfg/rules_part_func.cc` を起点に、非終端ごとのルールと依存関係を整理した。

### 12.1 ルール一覧（概要）
- `V`: Hairpin / Internal / VM
- `WI`: PK 内部ループの分割・伸張
- `W`: 外側 W（V/WMB 分割）
- `VM`: マルチループ閉じ
- `WMv/WMp`: マルチループの stem 開始・伸張
- `WM`: マルチループ本体
- `WIP`: PK 内部ループ系の分割・伸張
- `VPL/VPR`: VP 補助
- `VP`: PK 中核（WI/WIP/VPL/VPR 組み合わせ + stP/intP）
- `WMBW/WMBP/WMB`: PK band/外部ループ
- `BE`: band/exterior

### 12.2 依存関係の代表例
- `V` は `hairpin_energy / internal_energy / vm_energy` と `tree.up`
- `W` は `V/WMB` と `exp_Extloop/expPS_penalty/scale1`
- `VP` は `WI/WIP/VPL/VPR` と `get_e_stP/get_e_intP`, `pair_type_of`, `tree.B/Bp/b/bp`
- `WMBP/WMB/BE` は `BE/WI/VP` と `expPB_penalty`, `PartFuncRuleHelpers` の境界判定

### 12.3 初期レビューで見つかった不整合候補
- `compute_BE_restricted` は `pair > 0` を要求するが `W_final_pf::get_BE` は `pair >= 0` を許容している。
- `PartFuncRuleHelpers::parent_index` は `parent` null を想定しておらず、`sparse_tree` 契約の明文化が必要。
- `compute_WMBW_restricted` の `tree.tree[j].pair < j` が意図通りか要確認（unpaired だけなら `<0` が自然）。

## 13. ルールベース API 最小契約案 (2026-02-16)
`rules_for / applicable / expand / rule_score` による最小 API を提案する。

### 13.1 4 つの中核 API
- `rules_for(nonterminal, i, j, ctx)`:
  - 対象非終端が取り得るルール候補を列挙する（候補は過不足なく列挙）
- `applicable(rule, i, j, ctx)`:
  - 構造制約・境界・親関係・turn などを判定する
  - 現在 `compute_*` に散在している条件分岐を集約する
- `expand(rule, i, j, ctx)`:
  - 右辺の子非終端と区間（i, j, k, l など）を返す
  - DP 値の参照はしない（純粋に構文木の展開のみ）
- `rule_score(rule, i, j, ctx)`:
  - ルール固有の係数（エネルギー・ペナルティ・スケール）を返す
  - `exp_*`, `get_e_*`, `scale(u)` をここへ集約する

### 13.2 RuleSpec の最小表現（案）
```
Rule {
  lhs: NonTerminal
  rhs: [Symbol]      // NonTerminal or Terminal (energy / penalty / scale)
  split: SplitSpec   // k, l の走査範囲を束縛する仕様
}
```
- `SplitSpec` は `k`/`l` の走査範囲と依存境界（`tree.B/Bp/b/bp` など）を明示できる形が必要。

### 13.3 再マッピング例（代表ルール）
#### W
- `W(j) -> W(k-1) V(k,j)`
  - `applicable`: `weakly_closed(1,j)` かつ `weakly_closed(1,k-1)`
  - `expand`: `(W,1,k-1), (V,k,j)`
  - `rule_score`: `exp_Extloop(k,j)`
- `W(j) -> W(k-1) WMB(k,j)`
  - `rule_score`: `expPS_penalty()`
- `W(j) -> W(j-1)`
  - `applicable`: `tree[j].pair < 0`
  - `rule_score`: `scale1`

#### V
- `V(i,j) -> Hairpin(i,j)`
  - `applicable`: paired or unpaired 条件
  - `rule_score`: `hairpin_energy(i,j)`
- `V(i,j) -> Internal(i,j)`
  - `rule_score`: `internal_energy(i,j)`
- `V(i,j) -> VM(i,j)`
  - `rule_score`: `vm_energy(i,j)`

#### VP（PK 中核）
- `rules_for` で `VP` ルール群を列挙（WI/WIP/VPL/VPR/VP の組合せ）
- `applicable` で `tree.B/Bp/b/bp` 境界・`is_empty_region`・`pair_type` を判定
- `rule_score` で `scale`, `expap_penalty`, `expbp_penalty_sq`, `get_e_stP`, `get_e_intP` を集約

#### WMBP
- `PartFuncRuleHelpers` の境界判定は `applicable` に移す
- `rule_score` に `expPB_penalty` と `apply_double_pb_penalty` をまとめる

#### BE
- `rule_score` に `get_e_stP`, `get_e_intP`, `scale(u)`, `expcp_pen`, `expap_penalty`, `expbp_penalty_sq` を集約

### 13.4 コア契約（重要）
- ルール生成と制約判定は完全に分離する。
- `applicable(rule, ...) == false` のルールは `expand` しない。
- `rule_score` は「DP値に掛ける係数のみ」を返す。
- `expand` は DP 値に触れず、子非終端と区間だけを返す。

### 13.5 未決定事項（次の検討項目）
- `RuleId` の表現（enum / struct / テーブル駆動）
- `SplitSpec` の実装形式（単純分割 / 境界依存 / 複数走査変数）
- `rule_score` の戻り型を `pf_t` 固定にするか、Energy 型を導入するか

## 14. RuleId / SplitSpec 調査まとめ (2026-02-16)
`rules_part_func.cc` を基準に、ルール単位と分割パターンを整理した。

### 14.1 RuleId 一覧（非終端別）
**V**
- `V_HAIRPIN`
- `V_INTERNAL`
- `V_VM`

**WI**
- `WI_BASE_SINGLE`
- `WI_SPLIT_V`
- `WI_SPLIT_WMB`
- `WI_EXTEND_UNPAIRED`
Note: `WI_BASE_SINGLE` の係数は `expPUP_pen1`。

**W**
- `W_EXTEND_UNPAIRED`
- `W_SPLIT_V`
- `W_SPLIT_WMB`
Note: `W_SPLIT_*` は `k == 1` のとき左項が `1` になる特例がある（`W_EMPTY` 相当の扱いが必要）。

**VM**
- `VM_SPLIT_WM_WMv`
- `VM_SPLIT_WM_WMp`
- `VM_SPLIT_WMp_BASE`
- `VM_SCALE2` // 合計後に `scale2` を掛ける全体係数

**WMv/WMp**
- `WMv_STEM_V`
- `WMp_STEM_WMB`
- `WMv_EXTEND_UNPAIRED`
- `WMp_EXTEND_UNPAIRED`
Note: `*_EXTEND_UNPAIRED` は `tree.tree[j].pair < 0` が必要。

**WM**
- `WM_START_V`
- `WM_START_WMB`
- `WM_SPLIT_V`
- `WM_SPLIT_WMB`
- `WM_EXTEND_UNPAIRED`
Note: `WM_START_*` と `WM_SPLIT_*` の両方で `can_pair_left_span` が必要。

**WIP**
- `WIP_BASE_V`
- `WIP_BASE_WMB`
- `WIP_SPLIT_V`
- `WIP_SPLIT_WMB`
- `WIP_BASEPAIR_V`
- `WIP_BASEPAIR_WMB`
- `WIP_EXTEND_UNPAIRED`
Note: `WIP_EXTEND_UNPAIRED` は `tree.tree[j].pair < 0` が必要。

**VPL**
- `VPL_SPLIT_VP`

**VPR**
- `VPR_SPLIT_VP_WIP`
- `VPR_SPLIT_VP_BASEPAIR`

**VP**
- `VP_WI_CASE1`   // Bp/B 系の WI 二分割
- `VP_WI_CASE2`   // b/bp 系の WI 二分割
- `VP_WI_CASE3`   // WI 三分割
- `VP_STACK`      // get_e_stP * VP(i+1,j-1)
- `VP_INTERNAL_LOOP` // `k == i+1 && l == j-1` は除外する `skip` 条件が必要
- `VP_WIP_VP_LEFT`
- `VP_VP_WIP_RIGHT`
- `VP_WIP_VPR`
- `VP_VPL_WIP`

**WMBW**
- `WMBW_SPLIT_WMBP_WI`

**WMBP**
- `WMBP_SPLIT_BE_WMBP_VP`
- `WMBP_SPLIT_BE_WMBW_VP`
- `WMBP_DIRECT_VP`
- `WMBP_SPLIT_BE_WI_VP`

**WMB**
- `WMB_SPLIT_BE_WMBP_WI`
- `WMB_DIRECT_WMBP`
- `WMB_EMPTY` // `i == j` の base ケース (`0` を返す)

**BE**
- `BE_BASE_SAMEPAIR`
- `BE_STACK`
- `BE_INTERNAL_LOOP`
- `BE_WIP_WIP`
- `BE_WIP_BASEPAIR`
- `BE_BASEPAIR_WIP`

### 14.2 SplitSpec 種別（最小セット）
- **SplitSpec-0: NoSplit**
  - i,j 固定で完結するルール
  - 例: `V_HAIRPIN`, `V_INTERNAL`, `V_VM`, `VP_STACK`, `WMBP_DIRECT_VP`, `WMB_DIRECT_WMBP`, `BE_BASE_SAMEPAIR`
- **SplitSpec-1: SplitK**
  - 単一変数 `k` を `[lo, hi]` で走査
  - 例: `W_SPLIT_*`, `WI_SPLIT_*`, `WM_SPLIT_*`, `WIP_SPLIT_*`, `VPL_SPLIT_VP`
- **SplitSpec-2: SplitK + predicate**
  - `k` を走査しつつ `weakly_closed` / `can_pair_left_span` 等で追加判定
  - 例: `W` の分割条件、`WM/WIP` の `can_pair_left_span`
- **SplitSpec-3: SplitK + data-dependent bounds**
  - `tree.B/Bp/b/bp` による動的境界
  - 例: `VPL`, `VPR`
- **SplitSpec-4: SplitK + SplitL**
  - 二重変数 `k,l` 走査
  - 例: `VP_INTERNAL_LOOP`, `BE_INTERNAL_LOOP`
- **SplitSpec-5: SplitK/L + mixed bounds/predicate**
  - `MAXLOOP`, `tree.B/Bp/b/bp`, `is_unpaired_position`, `is_empty_region` が混在する複合条件
  - 例: `VP_INTERNAL_LOOP`（内側ループ）
- **SplitSpec-6: SplitK + band/parent 条件**
  - `PartFuncRuleHelpers` の境界判定と親関係判定を含む
  - 例: `WMBP_SPLIT_*`, `WMBW_SPLIT_*`
