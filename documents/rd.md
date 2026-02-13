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
- `compared >= 100`
- `mismatched = 0` (X=0)
- `finite_rate == 100%` (valid density-2 ケースのみ)
- invalidケース契約: 不正構造は必ず失敗コード/例外/NaN のいずれかで統一的に失敗する
- 数値比較は `abs_tol` と `rel_tol` を明記する

### 2.3 反チート実行規約
- テストは必ず fresh build で実行する (`rm -rf build && cmake -S . -B build && cmake --build build`)。
- `ctest` は `build/CTestTestfile.cmake` の生成時刻が当該コミット後であることを確認してから実行する。
- `api_cli_density2_energy_alignment` は「比較件数0件」で pass してはならない。`compared` を標準出力またはレポートに必ず出力し、`compared >= 100` を機械判定する。
- DoD 判定は source-controlled なテストコード/データのみを対象にし、`build/` 配下の既存生成物を正解として扱わない。
- ドキュメントだけ変更して `passes: true` にしてはならない。コード変更とテスト証跡を同一コミット群で提示する。

## 3. フェーズ1-2: 挙動不変リファクタリング
`src/part_func.cc`, `src/pseudo_loop.cc` の再帰ロジックを、式を変えずに関数/オブジェクトへ抽出する。

### 3.1 分離対象
- 16状態 (`ZW`, `ZV`, `ZP`, `ZVP` ほか) を状態単位で分離
- Constraint Oracle を独立させ、can-pair判定と構造整合判定を一本化

### 3.2 検証方針
- `static_assert` ではなく実行時 `ctest` で数値一致を監視
- 早期段階は report-gate (差分可視化中心)
- 後期段階は strict-gate (しきい値超過でfail)

## 4. フェーズ3: ロールバック条件
外部ツール完全一致ではなく、内部ベースライン逸脱をロールバック条件にする。
- Internal Baseline Drift が `abs_tol/rel_tol` を超えたらロールバック
- ViennaRNA との比較は参考値として扱い、ロールバック条件にはしない

## 5. フェーズ4: 固定構造評価器の統合
Rule Object + Constraint Oracle を使って単一パス評価を実装する。
- 入力構造から決定論的にルール列を特定
- `estP=0.89`, `eintP=0.74` などの係数適用を一貫化

## 6. 実行制約
- 当面は `-d2` のみを対象に実装・検証する
- オプション差分 (`-p`, `-k`, `-r`, `-d0`) は別フェーズで扱う
- 3ステップごとに code review + refactor + 全テスト通過確認を挟む

## 7. 完了条件 (Final DoD)
- `ctest -R api_cli_density2_energy_alignment` が実データで実行される
- 比較件数が 100 以上で、`mismatched=0`
- valid density-2 ケースで `finite_rate=100%`
- invalidケース契約テストが全件 pass
- 既存回帰テストを全通過
- 実行ログに `compared`, `mismatched`, `skipped`, `finite_rate` の4指標を必ず出力し、レビューで再計算可能であること
