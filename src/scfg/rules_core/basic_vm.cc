#include "scfg/rules_core.hh"

#include "scfg/constraint_oracle.hh"
#include "scfg/legacy_adapter.hh"
#include "scfg/rules_part_func.hh"
#include "sparse_tree.hh"

namespace scfg {

// pk-free 側: VM/WMv/WMp の分解・スコア・適用判定。
std::vector<ApplicableRule> applicable_rules_vm(cand_pos_t i,
                                                cand_pos_t j,
                                                PartFuncVMContext &ctx,
                                                std::vector<int> &up) {
    std::vector<ApplicableRule> out;
    for (RuleId rule : rules_for(NonTerminal::VM)) {
        const auto splits = enumerate_splits_vm(rule, i, j, ctx, up);
        for (const auto &split : splits) {
            out.push_back({rule, split});
        }
    }
    return out;
}

std::vector<RuleSplit> enumerate_splits_vm(RuleId rule,
                                           cand_pos_t i,
                                           cand_pos_t j,
                                           PartFuncVMContext &ctx,
                                           std::vector<int> &up) {
    std::vector<RuleSplit> splits;
    const cand_pos_t turn = ctx.turn();
    switch (rule) {
    case RuleId::VM_SPLIT_WM_WMv:
    case RuleId::VM_SPLIT_WM_WMp: {
        for (cand_pos_t k = i + 1; k <= j - turn - 1; ++k) {
            splits.push_back({k, -1});
        }
    } break;
    case RuleId::VM_SPLIT_WMp_BASE: {
        for (cand_pos_t k = i + 1; k <= j - turn - 1; ++k) {
            if (up[k - 1] >= (k - (i + 1))) {
                splits.push_back({k, -1});
            }
        }
    } break;
    default:
        break;
    }
    return splits;
}

std::vector<RuleChild> expand_vm(RuleId rule, cand_pos_t i, cand_pos_t j, const RuleSplit &split) {
    std::vector<RuleChild> children;
    switch (rule) {
    case RuleId::VM_SPLIT_WM_WMv:
        children.push_back({NonTerminal::WM, i + 1, split.k - 1});
        children.push_back({NonTerminal::WMv, split.k, j - 1});
        break;
    case RuleId::VM_SPLIT_WM_WMp:
        children.push_back({NonTerminal::WM, i + 1, split.k - 1});
        children.push_back({NonTerminal::WMp, split.k, j - 1});
        break;
    case RuleId::VM_SPLIT_WMp_BASE:
        children.push_back({NonTerminal::WMp, split.k, j - 1});
        break;
    default:
        break;
    }
    return children;
}

pf_t transition_weight_vm(RuleId rule,
                   cand_pos_t i,
                   cand_pos_t j,
                   const RuleSplit &split,
                   PartFuncVMContext &ctx,
                   std::vector<int> &up) {
    (void)up;
    switch (rule) {
    case RuleId::VM_SPLIT_WM_WMv:
    case RuleId::VM_SPLIT_WM_WMp:
        return ctx.exp_Mbloop(i, j) * ctx.expMLclosing();
    case RuleId::VM_SPLIT_WMp_BASE:
        return ctx.exp_Mbloop(i, j) * ctx.expMLclosing() * ctx.expMLbase(split.k - i - 1);
    default:
        return 0;
    }
}


} // namespace scfg
