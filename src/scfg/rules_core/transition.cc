#include "scfg/rules_core.hh"

namespace scfg {

// ルールごとの「遷移重み」。確率ではなく、未正規化の重みを返す。
// 現状は rule_score_* と同一の定義を採用する。

pf_t transition_weight_w(RuleId rule, cand_pos_t i, cand_pos_t j, const RuleSplit &split, PartFuncWContext &ctx) {
    return rule_score_w(rule, i, j, split, ctx);
}

pf_t transition_weight_v(RuleId rule,
                         cand_pos_t i,
                         cand_pos_t j,
                         const RuleSplit &split,
                         PartFuncVContext &ctx,
                         sparse_tree &tree) {
    return rule_score_v(rule, i, j, split, ctx, tree);
}

pf_t transition_weight_wi(RuleId rule,
                          cand_pos_t i,
                          cand_pos_t j,
                          const RuleSplit &split,
                          PartFuncWIContext &ctx) {
    return rule_score_wi(rule, i, j, split, ctx);
}

pf_t transition_weight_vm(RuleId rule,
                          cand_pos_t i,
                          cand_pos_t j,
                          const RuleSplit &split,
                          PartFuncVMContext &ctx,
                          std::vector<int> &up) {
    return rule_score_vm(rule, i, j, split, ctx, up);
}

pf_t transition_weight_wmv_wmp(RuleId rule,
                               cand_pos_t i,
                               cand_pos_t j,
                               const RuleSplit &split,
                               PartFuncWMvWMpContext &ctx,
                               std::vector<Node> &tree) {
    return rule_score_wmv_wmp(rule, i, j, split, ctx, tree);
}

pf_t transition_weight_wm(RuleId rule,
                          cand_pos_t i,
                          cand_pos_t j,
                          const RuleSplit &split,
                          PartFuncWMContext &ctx) {
    return rule_score_wm(rule, i, j, split, ctx);
}

pf_t transition_weight_wip(RuleId rule,
                           cand_pos_t i,
                           cand_pos_t j,
                           const RuleSplit &split,
                           PartFuncWIPContext &ctx) {
    return rule_score_wip(rule, i, j, split, ctx);
}

pf_t transition_weight_vpl(RuleId rule,
                           cand_pos_t i,
                           cand_pos_t j,
                           const RuleSplit &split,
                           PartFuncVPLContext &ctx) {
    return rule_score_vpl(rule, i, j, split, ctx);
}

pf_t transition_weight_vpr(RuleId rule,
                           cand_pos_t i,
                           cand_pos_t j,
                           const RuleSplit &split,
                           PartFuncVPRContext &ctx) {
    return rule_score_vpr(rule, i, j, split, ctx);
}

pf_t transition_weight_vp(RuleId rule,
                          cand_pos_t i,
                          cand_pos_t j,
                          const RuleSplit &split,
                          PartFuncVPContext &ctx,
                          sparse_tree &tree) {
    return rule_score_vp(rule, i, j, split, ctx, tree);
}

pf_t transition_weight_wmbw(RuleId rule, cand_pos_t i, cand_pos_t j, const RuleSplit &split, PartFuncWMBWContext &ctx) {
    return rule_score_wmbw(rule, i, j, split, ctx);
}

pf_t transition_weight_wmbp(RuleId rule,
                            cand_pos_t i,
                            cand_pos_t j,
                            const RuleSplit &split,
                            PartFuncWMBPContext &ctx,
                            sparse_tree &tree) {
    return rule_score_wmbp(rule, i, j, split, ctx, tree);
}

pf_t transition_weight_wmb(RuleId rule,
                           cand_pos_t i,
                           cand_pos_t j,
                           const RuleSplit &split,
                           PartFuncWMBContext &ctx,
                           sparse_tree &tree) {
    return rule_score_wmb(rule, i, j, split, ctx, tree);
}

pf_t transition_weight_be(RuleId rule,
                          cand_pos_t i,
                          cand_pos_t j,
                          cand_pos_t ip,
                          cand_pos_t jp,
                          const RuleSplit &split,
                          PartFuncBEContext &ctx,
                          sparse_tree &tree) {
    return rule_score_be(rule, i, j, ip, jp, split, ctx, tree);
}

} // namespace scfg
