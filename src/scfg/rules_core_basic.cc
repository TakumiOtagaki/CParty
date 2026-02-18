#include "scfg/rules_core.hh"

#include "scfg/constraint_oracle.hh"
#include "scfg/legacy_adapter.hh"
#include "scfg/rules_part_func.hh"
#include "sparse_tree.hh"

namespace scfg {
std::vector<ApplicableRule> applicable_rules_w(cand_pos_t i,
                                               cand_pos_t j,
                                               PartFuncWContext &ctx,
                                               sparse_tree &tree) {
    std::vector<ApplicableRule> out;
    for (RuleId rule : rules_for(NonTerminal::W)) {
        const auto splits = enumerate_splits_w(rule, i, j, ctx, tree);
        for (const auto &split : splits) {
            out.push_back({rule, split});
        }
    }
    return out;
}

std::vector<RuleSplit> enumerate_splits_w(RuleId rule,
                                          cand_pos_t i,
                                          cand_pos_t j,
                                          PartFuncWContext &ctx,
                                          sparse_tree &tree) {
    std::vector<RuleSplit> splits;
    switch (rule) {
    case RuleId::W_EXTEND_UNPAIRED:
        if (tree.tree[j].pair < 0) {
            splits.push_back({});
        }
        break;
    case RuleId::W_SPLIT_V:
    case RuleId::W_SPLIT_WMB: {
        if (!tree.weakly_closed(1, j)) {
            break;
        }
        const cand_pos_t turn = ctx.turn();
        for (cand_pos_t k = i; k <= j - turn - 1; ++k) {
            if (!tree.weakly_closed(1, k - 1)) continue;
            if (rule == RuleId::W_SPLIT_WMB && !(k == i || tree.weakly_closed(k, j))) continue;
            splits.push_back({k, -1});
        }
    } break;
    default:
        break;
    }
    return splits;
}

std::vector<RuleChild> expand_w(RuleId rule, cand_pos_t i, cand_pos_t j, const RuleSplit &split) {
    std::vector<RuleChild> children;
    switch (rule) {
    case RuleId::W_EXTEND_UNPAIRED:
        children.push_back({NonTerminal::W, i, j - 1});
        break;
    case RuleId::W_SPLIT_V:
        if (split.k > i) {
            children.push_back({NonTerminal::W, i, split.k - 1});
        }
        children.push_back({NonTerminal::V, split.k, j});
        break;
    case RuleId::W_SPLIT_WMB:
        if (split.k > i) {
            children.push_back({NonTerminal::W, i, split.k - 1});
        }
        children.push_back({NonTerminal::WMB, split.k, j});
        break;
    default:
        break;
    }
    return children;
}

pf_t rule_score_w(RuleId rule, cand_pos_t i, cand_pos_t j, const RuleSplit &split, PartFuncWContext &ctx) {
    (void)i;
    switch (rule) {
    case RuleId::W_EXTEND_UNPAIRED:
        return ctx.scale1();
    case RuleId::W_SPLIT_V:
        return ctx.exp_Extloop(split.k, j);
    case RuleId::W_SPLIT_WMB:
        return ctx.expPS_penalty();
    default:
        return 0;
    }
}

std::vector<ApplicableRule> applicable_rules_v(cand_pos_t i,
                                               cand_pos_t j,
                                               PartFuncVContext &ctx,
                                               sparse_tree &tree) {
    std::vector<ApplicableRule> out;
    for (RuleId rule : rules_for(NonTerminal::V)) {
        const auto splits = enumerate_splits_v(rule, i, j, ctx, tree);
        for (const auto &split : splits) {
            out.push_back({rule, split});
        }
    }
    return out;
}

std::vector<RuleSplit> enumerate_splits_v(RuleId rule,
                                          cand_pos_t i,
                                          cand_pos_t j,
                                          PartFuncVContext &ctx,
                                          sparse_tree &tree) {
    (void)ctx;
    std::vector<RuleSplit> splits;
    const bool unpaired = (tree.tree[i].pair < -1 && tree.tree[j].pair < -1);
    const bool paired = (tree.tree[i].pair == j && tree.tree[j].pair == i);
    if (!(paired || unpaired)) {
        return splits;
    }

    switch (rule) {
    case RuleId::V_HAIRPIN: {
        const bool canH = !(tree.up[j - 1] < (j - i - 1));
        if (canH) {
            splits.push_back({});
        }
    } break;
    case RuleId::V_INTERNAL:
    case RuleId::V_VM:
        splits.push_back({});
        break;
    default:
        break;
    }
    return splits;
}

std::vector<RuleChild> expand_v(RuleId rule, cand_pos_t i, cand_pos_t j, const RuleSplit &split) {
    (void)rule;
    (void)i;
    (void)j;
    (void)split;
    return {};
}

pf_t rule_score_v(RuleId rule,
                  cand_pos_t i,
                  cand_pos_t j,
                  const RuleSplit &split,
                  PartFuncVContext &ctx,
                  sparse_tree &tree) {
    (void)split;
    switch (rule) {
    case RuleId::V_HAIRPIN:
        return ctx.hairpin_energy(i, j);
    case RuleId::V_INTERNAL:
        return ctx.internal_energy(i, j, tree.up);
    case RuleId::V_VM:
        return ctx.vm_energy(i, j, tree.up);
    default:
        return 0;
    }
}

std::vector<ApplicableRule> applicable_rules_wi(cand_pos_t i,
                                                cand_pos_t j,
                                                PartFuncWIContext &ctx,
                                                sparse_tree &tree) {
    std::vector<ApplicableRule> out;
    for (RuleId rule : rules_for(NonTerminal::WI)) {
        const auto splits = enumerate_splits_wi(rule, i, j, ctx, tree);
        for (const auto &split : splits) {
            out.push_back({rule, split});
        }
    }
    return out;
}

std::vector<RuleSplit> enumerate_splits_wi(RuleId rule,
                                           cand_pos_t i,
                                           cand_pos_t j,
                                           PartFuncWIContext &ctx,
                                           sparse_tree &tree) {
    std::vector<RuleSplit> splits;
    if (i == j) {
        if (rule == RuleId::WI_BASE_SINGLE) {
            splits.push_back({});
        }
        return splits;
    }
    switch (rule) {
    case RuleId::WI_SPLIT_V:
    case RuleId::WI_SPLIT_WMB: {
        const cand_pos_t turn = ctx.turn();
        for (cand_pos_t k = i; k <= j - turn - 1; ++k) {
            splits.push_back({k, -1});
        }
    } break;
    case RuleId::WI_EXTEND_UNPAIRED:
        if (tree.tree[j].pair < 0) {
            splits.push_back({});
        }
        break;
    default:
        break;
    }
    return splits;
}

std::vector<RuleChild> expand_wi(RuleId rule, cand_pos_t i, cand_pos_t j, const RuleSplit &split) {
    std::vector<RuleChild> children;
    switch (rule) {
    case RuleId::WI_BASE_SINGLE:
        break;
    case RuleId::WI_SPLIT_V:
        if (split.k > i) {
            children.push_back({NonTerminal::WI, i, split.k - 1});
        }
        children.push_back({NonTerminal::V, split.k, j});
        break;
    case RuleId::WI_SPLIT_WMB:
        if (split.k > i) {
            children.push_back({NonTerminal::WI, i, split.k - 1});
        }
        children.push_back({NonTerminal::WMB, split.k, j});
        break;
    case RuleId::WI_EXTEND_UNPAIRED:
        children.push_back({NonTerminal::WI, i, j - 1});
        break;
    default:
        break;
    }
    return children;
}

pf_t rule_score_wi(RuleId rule,
                   cand_pos_t i,
                   cand_pos_t j,
                   const RuleSplit &split,
                   PartFuncWIContext &ctx) {
    (void)i;
    (void)j;
    (void)split;
    switch (rule) {
    case RuleId::WI_BASE_SINGLE:
        return ctx.expPUP_pen1();
    case RuleId::WI_SPLIT_V:
        return ctx.expPPS_penalty();
    case RuleId::WI_SPLIT_WMB:
        return ctx.expPSP_penalty() * ctx.expPPS_penalty();
    case RuleId::WI_EXTEND_UNPAIRED:
        return ctx.expPUP_pen1();
    default:
        return 0;
    }
}

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

pf_t rule_score_vm(RuleId rule,
                   cand_pos_t i,
                   cand_pos_t j,
                   const RuleSplit &split,
                   PartFuncVMContext &ctx,
                   std::vector<int> &up) {
    (void)split;
    (void)up;
    switch (rule) {
    case RuleId::VM_SPLIT_WM_WMv:
    case RuleId::VM_SPLIT_WM_WMp:
    case RuleId::VM_SPLIT_WMp_BASE:
        return ctx.exp_Mbloop(i, j) * ctx.expMLclosing();
    default:
        return 0;
    }
}

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

pf_t rule_score_wmv_wmp(RuleId rule,
                        cand_pos_t i,
                        cand_pos_t j,
                        const RuleSplit &split,
                        PartFuncWMvWMpContext &ctx,
                        std::vector<Node> &tree) {
    (void)split;
    (void)tree;
    switch (rule) {
    case RuleId::WMv_STEM_V:
        return ctx.exp_MLstem(i, j);
    case RuleId::WMp_STEM_WMB:
        return ctx.expPSM_penalty() * ctx.expb_penalty();
    case RuleId::WMv_EXTEND_UNPAIRED:
        return ctx.expMLbase1();
    case RuleId::WMp_EXTEND_UNPAIRED:
        return ctx.expMLbase1();
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

pf_t rule_score_wm(RuleId rule,
                   cand_pos_t i,
                   cand_pos_t j,
                   const RuleSplit &split,
                   PartFuncWMContext &ctx) {
    (void)i;
    (void)j;
    switch (rule) {
    case RuleId::WM_START_V:
    case RuleId::WM_START_WMB:
        return ctx.expMLbase(split.k - i);
    case RuleId::WM_SPLIT_V:
    case RuleId::WM_SPLIT_WMB:
        return 1;
    case RuleId::WM_EXTEND_UNPAIRED:
        return ctx.expMLbase(1);
    default:
        return 0;
    }
}

} // namespace scfg
