#include "scfg/rules_core.hh"

#include "scfg/constraint_oracle.hh"
#include "scfg/legacy_adapter.hh"
#include "scfg/rules_part_func.hh"
#include "scfg/transition_weights.hh"
#include "sparse_tree.hh"

namespace scfg {

// pk-free 側: WM の分解・スコア・適用判定。
std::vector<ApplicableRule> applicable_rules_wmv_wmp(cand_pos_t i,
                                                     cand_pos_t j,
                                                     PartFuncWMvWMpContext &ctx,
                                                     std::vector<Node> &tree) {
    std::vector<ApplicableRule> out;
    for (RuleId rule : rules_for(NonTerminal::WMv)) {
        const auto splits = enumerate_splits_wmv_wmp(rule, i, j, ctx, tree);
        for (const auto &split : splits) {
            out.push_back({rule, split});
        }
    }
    for (RuleId rule : rules_for(NonTerminal::WMp)) {
        const auto splits = enumerate_splits_wmv_wmp(rule, i, j, ctx, tree);
        for (const auto &split : splits) {
            out.push_back({rule, split});
        }
    }
    return out;
}

std::vector<RuleSplit> enumerate_splits_wmv_wmp(RuleId rule,
                                                cand_pos_t i,
                                                cand_pos_t j,
                                                PartFuncWMvWMpContext &ctx,
                                                std::vector<Node> &tree) {
    std::vector<RuleSplit> splits;
    if (j - i - 1 < ctx.turn()) {
        return splits;
    }
    switch (rule) {
    case RuleId::WMv_STEM_V:
    case RuleId::WMp_STEM_WMB:
        splits.push_back({});
        break;
    case RuleId::WMv_EXTEND_UNPAIRED:
    case RuleId::WMp_EXTEND_UNPAIRED:
        if (tree[j].pair < 0) {
            splits.push_back({});
        }
        break;
    default:
        break;
    }
    return splits;
}

std::vector<RuleChild> expand_wmv_wmp(RuleId rule, cand_pos_t i, cand_pos_t j, const RuleSplit &split) {
    (void)split;
    std::vector<RuleChild> children;
    switch (rule) {
    case RuleId::WMv_STEM_V:
        children.push_back({NonTerminal::V, i, j});
        break;
    case RuleId::WMp_STEM_WMB:
        children.push_back({NonTerminal::WMB, i, j});
        break;
    case RuleId::WMv_EXTEND_UNPAIRED:
        children.push_back({NonTerminal::WMv, i, j - 1});
        break;
    case RuleId::WMp_EXTEND_UNPAIRED:
        children.push_back({NonTerminal::WMp, i, j - 1});
        break;
    default:
        break;
    }
    return children;
}

pf_t transition_weight_wmv_wmp(RuleId rule,
                        cand_pos_t i,
                        cand_pos_t j,
                        const RuleSplit &split,
                        PartFuncWMvWMpContext &ctx,
                        std::vector<Node> &tree) {
    TransitionWeights<PartFuncWMvWMpContext> oracle(ctx);
    (void)split;
    (void)tree;
    switch (rule) {
    case RuleId::WMv_STEM_V:
        return oracle.exp_MLstem(i, j);
    case RuleId::WMp_STEM_WMB:
        return oracle.expPSM_penalty() * oracle.expb_penalty();
    case RuleId::WMv_EXTEND_UNPAIRED:
        return oracle.expMLbase1();
    case RuleId::WMp_EXTEND_UNPAIRED:
        return oracle.expMLbase1();
    default:
        return 0;
    }
}

std::vector<ApplicableRule> applicable_rules_wm(cand_pos_t i,
                                                cand_pos_t j,
                                                PartFuncWMContext &ctx,
                                                sparse_tree &tree) {
    std::vector<ApplicableRule> out;
    for (RuleId rule : rules_for(NonTerminal::WM)) {
        const auto splits = enumerate_splits_wm(rule, i, j, ctx, tree);
        for (const auto &split : splits) {
            out.push_back({rule, split});
        }
    }
    return out;
}

std::vector<RuleSplit> enumerate_splits_wm(RuleId rule,
                                           cand_pos_t i,
                                           cand_pos_t j,
                                           PartFuncWMContext &ctx,
                                           sparse_tree &tree) {
    std::vector<RuleSplit> splits;
    if (j - i + 1 < 4) {
        return splits;
    }
    const cand_pos_t turn = ctx.turn();
    switch (rule) {
    case RuleId::WM_START_V:
    case RuleId::WM_START_WMB:
    case RuleId::WM_SPLIT_V:
    case RuleId::WM_SPLIT_WMB: {
        cand_pos_t k_start = i;
        if (rule == RuleId::WM_SPLIT_V || rule == RuleId::WM_SPLIT_WMB) {
            k_start = i + 1;
        }
        for (cand_pos_t k = k_start; k < j - turn; ++k) {
            if (rule == RuleId::WM_START_V || rule == RuleId::WM_START_WMB) {
                if (!scfg::can_pair_left_span(tree, i, k)) continue;
            }
            splits.push_back({k, -1});
        }
    } break;
    case RuleId::WM_EXTEND_UNPAIRED:
        if (tree.tree[j].pair < 0) {
            splits.push_back({});
        }
        break;
    default:
        break;
    }
    return splits;
}

std::vector<RuleChild> expand_wm(RuleId rule, cand_pos_t i, cand_pos_t j, const RuleSplit &split) {
    std::vector<RuleChild> children;
    switch (rule) {
    case RuleId::WM_START_V:
        children.push_back({NonTerminal::V, split.k, j});
        break;
    case RuleId::WM_START_WMB:
        children.push_back({NonTerminal::WMB, split.k, j});
        break;
    case RuleId::WM_SPLIT_V:
        if (split.k > i) {
            children.push_back({NonTerminal::WM, i, split.k - 1});
        }
        children.push_back({NonTerminal::V, split.k, j});
        break;
    case RuleId::WM_SPLIT_WMB:
        if (split.k > i) {
            children.push_back({NonTerminal::WM, i, split.k - 1});
        }
        children.push_back({NonTerminal::WMB, split.k, j});
        break;
    case RuleId::WM_EXTEND_UNPAIRED:
        children.push_back({NonTerminal::WM, i, j - 1});
        break;
    default:
        break;
    }
    return children;
}

pf_t transition_weight_wm(RuleId rule,
                   cand_pos_t i,
                   cand_pos_t j,
                   const RuleSplit &split,
                   PartFuncWMContext &ctx) {
    TransitionWeights<PartFuncWMContext> oracle(ctx);
    (void)i;
    (void)j;
    switch (rule) {
    case RuleId::WM_START_V:
        return oracle.expMLbase(split.k - i) * oracle.exp_MLstem(split.k, j);
    case RuleId::WM_START_WMB:
        return oracle.expMLbase(split.k - i) * oracle.expPSM_penalty() * oracle.expb_penalty();
    case RuleId::WM_SPLIT_V:
        return oracle.exp_MLstem(split.k, j);
    case RuleId::WM_SPLIT_WMB:
        return oracle.expPSM_penalty() * oracle.expb_penalty();
    case RuleId::WM_EXTEND_UNPAIRED:
        return oracle.expMLbase(1);
    default:
        return 0;
    }
}


} // namespace scfg
