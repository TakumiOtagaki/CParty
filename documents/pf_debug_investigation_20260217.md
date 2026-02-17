# PF Debug Investigation (2026-02-17)

## 目的
- `compare_cli_stdout.sh` に PF（4行目）まで含めた strict check を導入し、
  legacy と current の PF 出力差分を調査・解消する。
- fixed-energy API 検証以前に、PF の legacy/current 一致を回復する。

## 背景
- これまでの strict test は stdout の 3行目（MFE）までを比較。
- PF 行（4行目）を加えると legacy/current が不一致であることが判明。

## 変更概要（観察強化）
### 1) stdout パーサ拡張
- `test/tools/parse_cparty_stdout.sh`
  - 3行目 MFE に加え 4行目 PF をパースし 6カラム出力へ拡張
  - PF energy で `inf/nan` を許容
- `test/tools/parse_cparty_stdout_test.sh`
  - 6カラム対応の期待値に更新
  - live 出力は PF フィールドが非空であることを確認

### 2) strict test の観察拡張
- `test/tools/compare_cli_from_tsv.sh`
  - MFE/PF の mismatch を分離カウント
  - PF 構造/エネルギー mismatch の内訳、inf/nan 件数、括弧種の傾向を出力

## 観察結果（2026-02-17 時点）
- MFE 行は legacy/current で一致（`compare_mfe_failed=0`）
- PF 行は全件不一致（`compare_pf_failed` が全件）
- 典型例
  - legacy PF: `()` + `[]` を含む構造
  - current PF: `[]` を含まない構造（pk が落ちている）

## 現在の核心的な発見
### A) `input_structure_given` の解放後参照
- `cmdline_parser_free(&args_info)` 後に `args_info.input_structure_given` を参照していた
- legacy/current ともに同じ問題があり、MFE が 0 に寄る原因
- 修正済み：free 前に `input_structure_given` を退避

### B) PF が `inf` になっていた根本原因
- current の PF 実行で `pair` 行列が未初期化となる TU が存在
- `ensure_pair_matrix_initialized()` が TU を跨いで安全ではなかった
- 対応：PF コアの TU で `make_pair_matrix()` を明示的に呼ぶ
- 結果：`pf_energy=inf` は解消

### C) 依然として PF 構造が legacy と不一致
- current PF は pk を含まず（`[]` 0件）
- legacy PF は pk を含む（`[]` あり）

## 現在の達成状況
- PF の `inf` 問題は解消（current 側）
- MFE は legacy/current で一致
- PF の構造・エネルギーは依然として不一致（要調査）

## これからの方針（次アクション）
### 1) 観察ログの追加（current / legacy_debug）
- `W_final_pf::hfold_pf` 内で以下を出力
  - `MFE_structure`（入力）
  - `final_structure_pf`（出力）
  - サンプル構造頻度上位（top-N）

### 2) PF 出力の比較条件を固定
- 同一 seq / restricted で legacy_debug と current を比較
- `CPARTY_PF_DEBUG=1` でログ採取

### 3) DP テーブル比較（必要時）
- PF 構造が一致しない場合、`W/WMB/VP` など主要 DP の値を比較

## クリア条件
- `compare_cli_stdout.sh`（PF 行含む）で
  - `compare_pf_failed=0`
  - `compare_mfe_failed=0`
  - `compare_compared >= compare_min`（>=100）
- PF 構造が legacy/current で完全一致
- PF エネルギーが legacy/current で一致（`inf/nan` を含まない）

## 直近の追加観察（2026-02-17）
- `CPARTY_PF_DEBUG=1` で `hfold_pf` の sample_summary/top-N を追加出力
- 単一ケースでの差分
  - `seq=GCAUGC`, `restricted=(....)` で
    - legacy_debug: `pf_energy=2.51`
    - current: `pf_energy=2.83`
    - どちらもサンプル構造は単一（`(....)`）で pk 構造は出現せず
- 追加の確認ポイント
  - legacy 側にも `W[n]` をログ出力し、current の `W[n]` と比較する
  - `scfg::compute_W_restricted` と legacy の W 計算パスの差分を掘る

### 追加更新（2026-02-17）
- 差分の根因
  - current の `S1_` が全て 0 になっており、`HairpinE` が小さくなる
  - `encode_sequence(..., 1)` が `alias` 未初期化状態で呼ばれていた
- 対応
  - current の `W_final_pf` コンストラクタで `make_pair_matrix()` を明示呼び出し
  - `S1_` が正しく初期化され、`HairpinE`/`V[1,n]`/`W[n]` が legacy と一致
- 直近の再確認
  - `seq=GCAUGC, restricted=(....)` で legacy/current の PF エネルギー一致

### 追加更新（2026-02-17 / PM）
- 根因その2（TU 内 pair 初期化の不足）
  - `pair`/`alias` がヘッダ内 `static` のため TU ごとに別インスタンス
  - `ensure_pair_matrix_initialized()` の static ガードが「プログラム全体で 1 回だけ」になり、
    他 TU の `pair` が未初期化のまま使われる問題が発生
  - 影響: `Sample_V` / `PartFuncAdapter::pair_type_of` などで `ptype=0` となり、PF が崩れる
- 対応
  - `ensure_pair_matrix_initialized()` を `static inline` に変更し TU ごとに初期化
  - `part_func_adapter.cc` の `pair_type_of()` で明示的に `ensure_pair_matrix_initialized()` 実行
- 根因その3（legacy 互換の `can_pair_left_span`）
  - legacy は `k==i` を許容しているが、`scfg::can_pair_left_span` は `k==i` を拒否
  - WM/VM の項が欠落し、PF エネルギーにズレが生じる
- 対応
  - `scfg::can_pair_left_span/right_span` を `split==left/right` で true 扱いに修正

### 現状
- strict compare（PF 行含む）が 600 ケースで全件一致

## worktree_legacy との比較について
- `worktree_legacy/` は `input_structure_given` の use-after-free が残っており、
  MFE/PF が 0 クリアされるケースが大量に発生する（PF が 0 に見える）
- `worktree_legacy_debug/` では該当修正済みのため strict compare の基準としては
  `worktree_legacy_debug/` を使うのが妥当
- `worktree_legacy/` を使う場合は `input_structure_given` を free 前に退避する修正が必須

## デバッグ経路の要約
### 1) PF 不一致の原因 1（TU 内 pair 初期化）
- `pair` / `alias` がヘッダ static で TU ごとに独立
- `ensure_pair_matrix_initialized()` の static guard がグローバル 1 回に見えて
  他 TU の pair が未初期化になる
- `PartFuncAdapter::pair_type_of()` / `Sample_V` などで `ptype=0` が発生し、PF が崩れる
- 対応: `ensure_pair_matrix_initialized()` を `static inline` 化 + `pair_type_of()` で明示初期化

### 2) PF 不一致の原因 2（can_pair_left_span 条件差）
- legacy は `k==i` のケースを許容するが `scfg::can_pair_left_span` は拒否
- WM/VM 経路が欠落し、V/VM/内部ループが小さくなる
- 対応: `split==left/right` を true に変更

### 3) PF 不一致の原因 3（スコア選択差の可視化）
- W/WMB/WMBP/VP/WM/VM/V/INTERNAL の各項をトレースし、
  missing term の場所を段階的に特定
- `CPARTY_PF_TRACE_*` による診断が有効

## 次にやること（整理）
1. strict compare の運用基準を `worktree_legacy_debug/` に固定
2. PF デバッグ用の `CPARTY_PF_TRACE_*` を残すか整理するか判断
3. `worktree_legacy/` を基準にしたい場合は use-after-free 修正の適用
4. fixed-energy API の PF 参照テスト設計へ戻る

## 進捗サマリ（2026-02-17 / late）
### クリア条件（現時点で採用）
- strict compare（PF 行含む）で `compare_pf_failed=0` かつ `compare_mfe_failed=0`
- `compare_compared >= compare_min` を満たす
- PF 構造に `[]` が含まれるケースでも legacy/current が一致
- PF エネルギーは `inf/nan` を含まない

### 達成状況
- **達成**: `worktree_legacy_debug/` を基準に strict compare が 600 ケースで全件一致
- **達成**: PF の `inf`/`nan` と pk 欠落問題を解消
- **未達**: `worktree_legacy/` 基準だと PF/MFE が 0 になり一致しない
  - 原因は `input_structure_given` の use-after-free（legacy 側未修正）

### 次のアクション（実行順）
1. `worktree_legacy_debug/` を基準に strict compare を再実行し、再現性を確認
2. `worktree_legacy/` を基準にしたい場合は、UAF 修正を適用（要合意）
3. PF デバッグトレース（`CPARTY_PF_TRACE_*`）の整理方針を決める
4. fixed-energy API 検証・テスト設計に復帰
