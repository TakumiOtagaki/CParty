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
