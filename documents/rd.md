# CParty SCFG Refactoring Plan

## 1. 目的
CPartyの「非曖昧（unambiguous）な密度2構造分解ロジック」を独立したクラスとして抽象化し、以下の機能を機械的に切り替え可能にする。
*   **Partition Function**: 全パスのボルツマン重みの和（現状の機能）。
*   **Fixed-Structure Energy**: 与えられた $G \cup G'$ パスのみのエネルギー評価（新規機能）。
*   **MFE Prediction**: 最もエネルギーの低いパスの選択。

## 2. フェーズ1：コア・インターフェースの設計
16個の非終端記号（行列）を共通のインターフェースを持つメソッドとして定義する。

### 抽象基底クラス: `CPartySCFG`
*   **状態管理**: `ZW`, `ZV`, `ZP`, `ZVP` 等、計16個のDPテーブルを保持。
*   **共通メソッド**:
    *   `evaluate(state, i, j)`: 指定された状態における値を計算（和 or 単一パス）。
    *   `get_valid_rules(state, i, j)`: 現在の領域で適用可能な生成規則（Case 1, 2...）を返す。
*   **入力制約 (Constraint Oracle)**:
    *   `is_pair_allowed(i, j, set_type)`: 塩基対 $(i, j)$ が $G$ または $G'$ の構造と一致するか判定。

## 3. フェーズ2：16の非終端記号（状態）の関数化
ソースコード（`src/part_func.cc`, `src/pseudo_loop.cc` 等）に散らばっている再帰ロジックを、以下の16個のメソッドに完全分離する。

1.  **Global**: `calcZW`
2.  **PK-free**: `calcZV`, `calcZVBI`
3.  **Pseudoloop**: `calcZP`, `calcZPG'`, `calcZPG'w`, `calcZWI`
4.  **Band-crossing**: `calcZVP`, `calcZVPR`, `calcZVPL`, `calcZWI'`, `calcZBE`
5.  **Multiloop**: `calcZVM`, `calcZWM`, `calcZWM1`, `calcZWMP`

全部で 16 個の非終端記号がある。



## 4. フェーズ3：エネルギーモデルとスケーリングの定数化
計算ロジックから物理定数を分離し、一貫性を保つ。
*   **基本項**: $Ps, Pb, Pup, Psm, Psp, Pps$ 等のペナルティ。
*   **スケーリング定数**:
    *   `estP` (Stack spanning band): **0.89倍**
    *   `eintP` (Internal loop spanning band): **0.74倍**
    *   マルチループ項 ($a', b', c'$)

## 5. フェーズ4：機械的パース（Traceback）の実装
「非曖昧性」を活かし、与えられた $G \cup G'$ 文字列から一意なパスを特定するロジック。

1.  **Top-down Parsing**: $ZW(1, n)$ から開始。
2.  **Rule Selection**: `get_valid_rules` を呼び出し、入力構造の塩基対 $(i, j)$ と合致する唯一の Case を選択。
3.  **Energy Aggregation**: 各ノードで算出された $B(e)$ またはエネルギー値を集計。
4.  **Density-2 Validation**: パースの過程で、もし入力構造が文法（密度2）に適合しない場合は `NaN` または例外を返す。

---

### Codex へのフィードバック依頼ポイント
1.  **実装の共通化**: `double`（分配関数用）と `EnergyScore`（固定構造用）をテンプレート化して一つのロジックで共有できるか？
2.  **依存関係の整理**: $ZVP$ と $ZBE$ の相互依存を解消し、ボトムアップな Fill 順序を最適化できるか？
3.  **既存コードの再利用**: 現在の `pseudo_loop.cc` 内の複雑な `if-else` 分岐を、どのように SCFG の生成規則（Rules）として綺麗にマッピングし直すのが効率的か？

---

**見込み**:
CPartyの文法は既に「非曖昧」であることが数学的に証明されているため、このリファクタリングによって**「入力構造がパスを決定し、パスがエネルギーを決定する」**という機械的なパイプラインが完成し、正確な $E(G \cup G', S)$ の評価が可能になります。