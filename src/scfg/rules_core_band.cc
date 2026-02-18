#include "scfg/rules_core.hh"

#include "scfg/constraint_oracle.hh"
#include "scfg/legacy_adapter.hh"
#include "scfg/rules_part_func.hh"
#include "scfg/structure_view.hh"
#include "sparse_tree.hh"

#include <ViennaRNA/params/constants.h>

namespace scfg {

std::vector<ApplicableRule> applicable_rules_wip(cand_pos_t i,
                                                 cand_pos_t j,
                                                 PartFuncWIPContext &ctx,
                                                 sparse_tree &tree) {
    std::vector<ApplicableRule> out;
    for (RuleId rule : rules_for(NonTerminal::WIP)) {
        const auto splits = enumerate_splits_wip(rule, i, j, ctx, tree);
        for (const auto &split : splits) {
            out.push_back({rule, split});
        }
    }
    return out;
}

std::vector<ApplicableRule> applicable_rules_wip(cand_pos_t i,
                                                 cand_pos_t j,
                                                 PartFuncWIPContext &ctx,
                                                 const StructureView &view) {
    std::vector<ApplicableRule> out;
    for (RuleId rule : rules_for(NonTerminal::WIP)) {
        const auto splits = enumerate_splits_wip(rule, i, j, ctx, view);
        for (const auto &split : splits) {
            out.push_back({rule, split});
        }
    }
    return out;
}

std::vector<RuleSplit> enumerate_splits_wip(RuleId rule,
                                            cand_pos_t i,
                                            cand_pos_t j,
                                            PartFuncWIPContext &ctx,
                                            sparse_tree &tree) {
    std::vector<RuleSplit> splits;
    const cand_pos_t turn = ctx.turn();
    switch (rule) {
    case RuleId::WIP_BASE_V:
    case RuleId::WIP_BASE_WMB:
        splits.push_back({});
        break;
    case RuleId::WIP_SPLIT_V:
    case RuleId::WIP_SPLIT_WMB: {
        for (cand_pos_t k = i + 1; k < j - turn - 1; ++k) {
            splits.push_back({k, -1});
        }
    } break;
    case RuleId::WIP_BASEPAIR_V:
    case RuleId::WIP_BASEPAIR_WMB: {
        for (cand_pos_t k = i + 1; k < j - turn - 1; ++k) {
            if (scfg::can_pair_left_span(tree, i, k)) {
                splits.push_back({k, -1});
            }
        }
    } break;
    case RuleId::WIP_EXTEND_UNPAIRED:
        if (tree.tree[j].pair < 0) {
            splits.push_back({});
        }
        break;
    default:
        break;
    }
    return splits;
}

std::vector<RuleSplit> enumerate_splits_wip(RuleId rule,
                                            cand_pos_t i,
                                            cand_pos_t j,
                                            PartFuncWIPContext &ctx,
                                            const StructureView &view) {
    std::vector<RuleSplit> splits;
    const cand_pos_t turn = ctx.turn();
    switch (rule) {
    case RuleId::WIP_BASE_V:
    case RuleId::WIP_BASE_WMB:
        splits.push_back({});
        break;
    case RuleId::WIP_SPLIT_V:
    case RuleId::WIP_SPLIT_WMB: {
        for (cand_pos_t k = i + 1; k < j - turn - 1; ++k) {
            splits.push_back({k, -1});
        }
    } break;
    case RuleId::WIP_BASEPAIR_V:
    case RuleId::WIP_BASEPAIR_WMB: {
        for (cand_pos_t k = i + 1; k < j - turn - 1; ++k) {
            if (view.can_pair_left_span(i, k)) {
                splits.push_back({k, -1});
            }
        }
    } break;
    case RuleId::WIP_EXTEND_UNPAIRED:
        if (view.is_unpaired(j)) {
            splits.push_back({});
        }
        break;
    default:
        break;
    }
    return splits;
}

std::vector<RuleChild> expand_wip(RuleId rule, cand_pos_t i, cand_pos_t j, const RuleSplit &split) {
    std::vector<RuleChild> children;
    switch (rule) {
    case RuleId::WIP_BASE_V:
        children.push_back({NonTerminal::V, i, j});
        break;
    case RuleId::WIP_BASE_WMB:
        children.push_back({NonTerminal::WMB, i, j});
        break;
    case RuleId::WIP_SPLIT_V:
        children.push_back({NonTerminal::WIP, i, split.k - 1});
        children.push_back({NonTerminal::V, split.k, j});
        break;
    case RuleId::WIP_SPLIT_WMB:
        children.push_back({NonTerminal::WIP, i, split.k - 1});
        children.push_back({NonTerminal::WMB, split.k, j});
        break;
    case RuleId::WIP_BASEPAIR_V:
        children.push_back({NonTerminal::V, split.k, j});
        break;
    case RuleId::WIP_BASEPAIR_WMB:
        children.push_back({NonTerminal::WMB, split.k, j});
        break;
    case RuleId::WIP_EXTEND_UNPAIRED:
        children.push_back({NonTerminal::WIP, i, j - 1});
        break;
    default:
        break;
    }
    return children;
}

pf_t rule_score_wip(RuleId rule,
                    cand_pos_t i,
                    cand_pos_t j,
                    const RuleSplit &split,
                    PartFuncWIPContext &ctx) {
    (void)j;
    switch (rule) {
    case RuleId::WIP_BASE_V:
    case RuleId::WIP_SPLIT_V:
        return ctx.expbp_penalty();
    case RuleId::WIP_BASE_WMB:
    case RuleId::WIP_SPLIT_WMB:
        return ctx.expbp_penalty() * ctx.expPSM_penalty();
    case RuleId::WIP_BASEPAIR_V:
        return ctx.expcp_pen(split.k - i) * ctx.expbp_penalty();
    case RuleId::WIP_BASEPAIR_WMB:
        return ctx.expcp_pen(split.k - i) * ctx.expbp_penalty() * ctx.expPSM_penalty();
    case RuleId::WIP_EXTEND_UNPAIRED:
        return ctx.expcp_pen(1);
    default:
        return 0;
    }
}

std::vector<ApplicableRule> applicable_rules_vpl(cand_pos_t i,
                                                 cand_pos_t j,
                                                 PartFuncVPLContext &ctx,
                                                 sparse_tree &tree) {
    std::vector<ApplicableRule> out;
    for (RuleId rule : rules_for(NonTerminal::VPL)) {
        const auto splits = enumerate_splits_vpl(rule, i, j, ctx, tree);
        for (const auto &split : splits) {
            out.push_back({rule, split});
        }
    }
    return out;
}

std::vector<ApplicableRule> applicable_rules_vpl(cand_pos_t i,
                                                 cand_pos_t j,
                                                 PartFuncVPLContext &ctx,
                                                 const StructureView &view) {
    std::vector<ApplicableRule> out;
    for (RuleId rule : rules_for(NonTerminal::VPL)) {
        const auto splits = enumerate_splits_vpl(rule, i, j, ctx, view);
        for (const auto &split : splits) {
            out.push_back({rule, split});
        }
    }
    return out;
}

std::vector<RuleSplit> enumerate_splits_vpl(RuleId rule,
                                            cand_pos_t i,
                                            cand_pos_t j,
                                            PartFuncVPLContext &ctx,
                                            sparse_tree &tree) {
    (void)ctx;
    std::vector<RuleSplit> splits;
    if (rule != RuleId::VPL_SPLIT_VP) {
        return splits;
    }
    cand_pos_t min_Bp_j = std::min((cand_pos_tu)tree.b(i, j), (cand_pos_tu)tree.Bp(i, j));
    for (cand_pos_t k = i + 1; k < min_Bp_j; ++k) {
        if (scfg::can_pair_left_span(tree, i, k)) {
            splits.push_back({k, -1});
        }
    }
    return splits;
}

std::vector<RuleSplit> enumerate_splits_vpl(RuleId rule,
                                            cand_pos_t i,
                                            cand_pos_t j,
                                            PartFuncVPLContext &ctx,
                                            const StructureView &view) {
    (void)ctx;
    std::vector<RuleSplit> splits;
    if (rule != RuleId::VPL_SPLIT_VP) {
        return splits;
    }
    cand_pos_t min_Bp_j = std::min((cand_pos_tu)view.b(i, j), (cand_pos_tu)view.Bp(i, j));
    for (cand_pos_t k = i + 1; k < min_Bp_j; ++k) {
        if (view.can_pair_left_span(i, k)) {
            splits.push_back({k, -1});
        }
    }
    return splits;
}

std::vector<RuleChild> expand_vpl(RuleId rule, cand_pos_t i, cand_pos_t j, const RuleSplit &split) {
    (void)i;
    std::vector<RuleChild> children;
    switch (rule) {
    case RuleId::VPL_SPLIT_VP:
        children.push_back({NonTerminal::VP, split.k, j});
        break;
    default:
        break;
    }
    return children;
}

pf_t rule_score_vpl(RuleId rule,
                    cand_pos_t i,
                    cand_pos_t j,
                    const RuleSplit &split,
                    PartFuncVPLContext &ctx) {
    (void)j;
    switch (rule) {
    case RuleId::VPL_SPLIT_VP:
        return ctx.expcp_pen(split.k - i);
    default:
        return 0;
    }
}

std::vector<ApplicableRule> applicable_rules_vpr(cand_pos_t i,
                                                 cand_pos_t j,
                                                 PartFuncVPRContext &ctx,
                                                 sparse_tree &tree) {
    std::vector<ApplicableRule> out;
    for (RuleId rule : rules_for(NonTerminal::VPR)) {
        const auto splits = enumerate_splits_vpr(rule, i, j, ctx, tree);
        for (const auto &split : splits) {
            out.push_back({rule, split});
        }
    }
    return out;
}

std::vector<ApplicableRule> applicable_rules_vpr(cand_pos_t i,
                                                 cand_pos_t j,
                                                 PartFuncVPRContext &ctx,
                                                 const StructureView &view) {
    std::vector<ApplicableRule> out;
    for (RuleId rule : rules_for(NonTerminal::VPR)) {
        const auto splits = enumerate_splits_vpr(rule, i, j, ctx, view);
        for (const auto &split : splits) {
            out.push_back({rule, split});
        }
    }
    return out;
}

std::vector<RuleSplit> enumerate_splits_vpr(RuleId rule,
                                            cand_pos_t i,
                                            cand_pos_t j,
                                            PartFuncVPRContext &ctx,
                                            sparse_tree &tree) {
    (void)ctx;
    std::vector<RuleSplit> splits;
    const cand_pos_t max_i_bp = std::max(tree.B(i, j), tree.bp(i, j));
    switch (rule) {
    case RuleId::VPR_SPLIT_VP_WIP:
        for (cand_pos_t k = max_i_bp + 1; k < j; ++k) {
            splits.push_back({k, -1});
        }
        break;
    case RuleId::VPR_SPLIT_VP_BASEPAIR:
        for (cand_pos_t k = max_i_bp + 1; k < j; ++k) {
            if (scfg::can_pair_right_span(tree, k, j)) {
                splits.push_back({k, -1});
            }
        }
        break;
    default:
        break;
    }
    return splits;
}

std::vector<RuleSplit> enumerate_splits_vpr(RuleId rule,
                                            cand_pos_t i,
                                            cand_pos_t j,
                                            PartFuncVPRContext &ctx,
                                            const StructureView &view) {
    (void)ctx;
    std::vector<RuleSplit> splits;
    const cand_pos_t max_i_bp = std::max(view.B(i, j), view.bp(i, j));
    switch (rule) {
    case RuleId::VPR_SPLIT_VP_WIP:
        for (cand_pos_t k = max_i_bp + 1; k < j; ++k) {
            splits.push_back({k, -1});
        }
        break;
    case RuleId::VPR_SPLIT_VP_BASEPAIR:
        for (cand_pos_t k = max_i_bp + 1; k < j; ++k) {
            if (view.can_pair_right_span(k, j)) {
                splits.push_back({k, -1});
            }
        }
        break;
    default:
        break;
    }
    return splits;
}

std::vector<RuleChild> expand_vpr(RuleId rule, cand_pos_t i, cand_pos_t j, const RuleSplit &split) {
    std::vector<RuleChild> children;
    switch (rule) {
    case RuleId::VPR_SPLIT_VP_WIP:
        children.push_back({NonTerminal::VP, i, split.k});
        children.push_back({NonTerminal::WIP, split.k + 1, j});
        break;
    case RuleId::VPR_SPLIT_VP_BASEPAIR:
        children.push_back({NonTerminal::VP, i, split.k});
        break;
    default:
        break;
    }
    return children;
}

pf_t rule_score_vpr(RuleId rule,
                    cand_pos_t i,
                    cand_pos_t j,
                    const RuleSplit &split,
                    PartFuncVPRContext &ctx) {
    (void)j;
    switch (rule) {
    case RuleId::VPR_SPLIT_VP_WIP:
        return 1;
    case RuleId::VPR_SPLIT_VP_BASEPAIR:
        return ctx.expcp_pen(split.k - i);
    default:
        return 0;
    }
}

std::vector<ApplicableRule> applicable_rules_vp(cand_pos_t i,
                                                cand_pos_t j,
                                                PartFuncVPContext &ctx,
                                                sparse_tree &tree) {
    std::vector<ApplicableRule> out;
    for (RuleId rule : rules_for(NonTerminal::VP)) {
        const auto splits = enumerate_splits_vp(rule, i, j, ctx, tree);
        for (const auto &split : splits) {
            out.push_back({rule, split});
        }
    }
    return out;
}

std::vector<ApplicableRule> applicable_rules_vp(cand_pos_t i,
                                                cand_pos_t j,
                                                PartFuncVPContext &ctx,
                                                const StructureView &view) {
    std::vector<ApplicableRule> out;
    for (RuleId rule : rules_for(NonTerminal::VP)) {
        const auto splits = enumerate_splits_vp(rule, i, j, ctx, view);
        for (const auto &split : splits) {
            out.push_back({rule, split});
        }
    }
    return out;
}

std::vector<RuleSplit> enumerate_splits_vp(RuleId rule,
                                           cand_pos_t i,
                                           cand_pos_t j,
                                           PartFuncVPContext &ctx,
                                           sparse_tree &tree) {
    std::vector<RuleSplit> splits;
    cand_pos_t Bp_ij = tree.Bp(i, j);
    cand_pos_t B_ij = tree.B(i, j);
    cand_pos_t b_ij = tree.b(i, j);
    cand_pos_t bp_ij = tree.bp(i, j);

    switch (rule) {
    case RuleId::VP_WI_CASE1:
        if ((tree.tree[i].parent->index) > 0 && (tree.tree[j].parent->index) < (tree.tree[i].parent->index) &&
            Bp_ij >= 0 && B_ij >= 0 && bp_ij < 0) {
            splits.push_back({-1, -1, Bp_ij, B_ij});
        }
        break;
    case RuleId::VP_WI_CASE2:
        if ((tree.tree[i].parent->index) < (tree.tree[j].parent->index) && (tree.tree[j].parent->index) > 0 &&
            b_ij >= 0 && bp_ij >= 0 && Bp_ij < 0) {
            splits.push_back({-1, -1, b_ij, bp_ij});
        }
        break;
    case RuleId::VP_WI_CASE3:
        if ((tree.tree[i].parent->index) > 0 && (tree.tree[j].parent->index) > 0 && Bp_ij >= 0 && B_ij >= 0 &&
            b_ij >= 0 && bp_ij >= 0) {
            splits.push_back({b_ij, bp_ij, Bp_ij, B_ij});
        }
        break;
    case RuleId::VP_STACK: {
        pair_type ptype_closingip1jm1 = ctx.pair_type_of(i + 1, j - 1);
        if ((tree.tree[i + 1].pair) < -1 && (tree.tree[j - 1].pair) < -1 &&
            scfg::is_pair_type_allowed(ptype_closingip1jm1)) {
            splits.push_back({});
        }
    } break;
    case RuleId::VP_INTERNAL_LOOP: {
        cand_pos_t min_borders = std::min((cand_pos_tu)Bp_ij, (cand_pos_tu)b_ij);
        cand_pos_t edge_i = std::min(static_cast<cand_pos_t>(i + MAXLOOP + 1), static_cast<cand_pos_t>(j - TURN - 1));
        min_borders = std::min(min_borders, edge_i);
        for (cand_pos_t k = i + 1; k < min_borders; ++k) {
            if (scfg::is_unpaired_position(tree, k) && scfg::is_empty_region(tree, i, k)) {
                cand_pos_t max_borders = std::max(bp_ij, B_ij) + 1;
                cand_pos_t edge_j = k + j - i - MAXLOOP - 2;
                max_borders = std::max(max_borders, edge_j);
                for (cand_pos_t l = j - 1; l > max_borders; --l) {
                    pair_type ptype_closingkj = ctx.pair_type_of(k, l);
                    if (k == i + 1 && l == j - 1) continue;
                    if (scfg::is_unpaired_position(tree, l) && scfg::is_pair_type_allowed(ptype_closingkj) &&
                        scfg::is_empty_region(tree, l, j)) {
                        splits.push_back({k, l});
                    }
                }
            }
        }
    } break;
    case RuleId::VP_WIP_VP_LEFT: {
        cand_pos_t min_Bp_j = std::min((cand_pos_tu)tree.b(i, j), (cand_pos_tu)tree.Bp(i, j));
        for (cand_pos_t k = i + 1; k < min_Bp_j; ++k) {
            splits.push_back({k, -1});
        }
    } break;
    case RuleId::VP_VP_WIP_RIGHT: {
        cand_pos_t max_i_bp = std::max(tree.B(i, j), tree.bp(i, j));
        for (cand_pos_t k = max_i_bp + 1; k < j; ++k) {
            splits.push_back({k, -1});
        }
    } break;
    case RuleId::VP_WIP_VPR: {
        cand_pos_t min_Bp_j = std::min((cand_pos_tu)tree.b(i, j), (cand_pos_tu)tree.Bp(i, j));
        for (cand_pos_t k = i + 1; k < min_Bp_j; ++k) {
            splits.push_back({k, -1});
        }
    } break;
    case RuleId::VP_VPL_WIP: {
        cand_pos_t max_i_bp = std::max(tree.B(i, j), tree.bp(i, j));
        for (cand_pos_t k = max_i_bp + 1; k < j; ++k) {
            splits.push_back({k, -1});
        }
    } break;
    default:
        break;
    }
    return splits;
}

std::vector<RuleSplit> enumerate_splits_vp(RuleId rule,
                                           cand_pos_t i,
                                           cand_pos_t j,
                                           PartFuncVPContext &ctx,
                                           const StructureView &view) {
    std::vector<RuleSplit> splits;
    cand_pos_t Bp_ij = view.Bp(i, j);
    cand_pos_t B_ij = view.B(i, j);
    cand_pos_t b_ij = view.b(i, j);
    cand_pos_t bp_ij = view.bp(i, j);

    switch (rule) {
    case RuleId::VP_WI_CASE1:
        if (view.parent_index(i) > 0 && view.parent_index(j) < view.parent_index(i) && Bp_ij >= 0 && B_ij >= 0 &&
            bp_ij < 0) {
            splits.push_back({-1, -1, Bp_ij, B_ij});
        }
        break;
    case RuleId::VP_WI_CASE2:
        if (view.parent_index(i) < view.parent_index(j) && view.parent_index(j) > 0 && b_ij >= 0 && bp_ij >= 0 &&
            Bp_ij < 0) {
            splits.push_back({-1, -1, b_ij, bp_ij});
        }
        break;
    case RuleId::VP_WI_CASE3:
        if (view.parent_index(i) > 0 && view.parent_index(j) > 0 && Bp_ij >= 0 && B_ij >= 0 && b_ij >= 0 &&
            bp_ij >= 0) {
            splits.push_back({b_ij, bp_ij, Bp_ij, B_ij});
        }
        break;
    case RuleId::VP_STACK: {
        pair_type ptype_closingip1jm1 = ctx.pair_type_of(i + 1, j - 1);
        if (view.is_unpaired(i + 1) && view.is_unpaired(j - 1) && scfg::is_pair_type_allowed(ptype_closingip1jm1)) {
            splits.push_back({});
        }
    } break;
    case RuleId::VP_INTERNAL_LOOP: {
        cand_pos_t min_borders = std::min((cand_pos_tu)Bp_ij, (cand_pos_tu)b_ij);
        cand_pos_t edge_i = std::min(static_cast<cand_pos_t>(i + MAXLOOP + 1), static_cast<cand_pos_t>(j - TURN - 1));
        min_borders = std::min(min_borders, edge_i);
        for (cand_pos_t k = i + 1; k < min_borders; ++k) {
            if (view.is_unpaired(k) && view.is_empty_region(i, k)) {
                cand_pos_t max_borders = std::max(bp_ij, B_ij) + 1;
                cand_pos_t edge_j = k + j - i - MAXLOOP - 2;
                max_borders = std::max(max_borders, edge_j);
                for (cand_pos_t l = j - 1; l > max_borders; --l) {
                    pair_type ptype_closingkj = ctx.pair_type_of(k, l);
                    if (k == i + 1 && l == j - 1) continue;
                    if (view.is_unpaired(l) && scfg::is_pair_type_allowed(ptype_closingkj) &&
                        view.is_empty_region(l, j)) {
                        splits.push_back({k, l});
                    }
                }
            }
        }
    } break;
    case RuleId::VP_WIP_VP_LEFT: {
        cand_pos_t min_Bp_j = std::min((cand_pos_tu)view.b(i, j), (cand_pos_tu)view.Bp(i, j));
        for (cand_pos_t k = i + 1; k < min_Bp_j; ++k) {
            splits.push_back({k, -1});
        }
    } break;
    case RuleId::VP_VP_WIP_RIGHT: {
        cand_pos_t max_i_bp = std::max(view.B(i, j), view.bp(i, j));
        for (cand_pos_t k = max_i_bp + 1; k < j; ++k) {
            splits.push_back({k, -1});
        }
    } break;
    case RuleId::VP_WIP_VPR: {
        cand_pos_t min_Bp_j = std::min((cand_pos_tu)view.b(i, j), (cand_pos_tu)view.Bp(i, j));
        for (cand_pos_t k = i + 1; k < min_Bp_j; ++k) {
            splits.push_back({k, -1});
        }
    } break;
    case RuleId::VP_VPL_WIP: {
        cand_pos_t max_i_bp = std::max(view.B(i, j), view.bp(i, j));
        for (cand_pos_t k = max_i_bp + 1; k < j; ++k) {
            splits.push_back({k, -1});
        }
    } break;
    default:
        break;
    }
    return splits;
}

std::vector<RuleChild> expand_vp(RuleId rule, cand_pos_t i, cand_pos_t j, const RuleSplit &split) {
    (void)i;
    (void)j;
    std::vector<RuleChild> children;
    switch (rule) {
    case RuleId::VP_WI_CASE1:
        children.push_back({NonTerminal::WI, i + 1, split.p - 1});
        children.push_back({NonTerminal::WI, split.q + 1, j - 1});
        break;
    case RuleId::VP_WI_CASE2:
        children.push_back({NonTerminal::WI, i + 1, split.p - 1});
        children.push_back({NonTerminal::WI, split.q + 1, j - 1});
        break;
    case RuleId::VP_WI_CASE3:
        children.push_back({NonTerminal::WI, i + 1, split.p - 1});
        children.push_back({NonTerminal::WI, split.q + 1, split.k - 1});
        children.push_back({NonTerminal::WI, split.l + 1, j - 1});
        break;
    case RuleId::VP_STACK:
        children.push_back({NonTerminal::VP, i + 1, j - 1});
        break;
    case RuleId::VP_INTERNAL_LOOP:
        children.push_back({NonTerminal::VP, split.k, split.l});
        break;
    case RuleId::VP_WIP_VP_LEFT:
        children.push_back({NonTerminal::WIP, i + 1, split.k - 1});
        children.push_back({NonTerminal::VP, split.k, j - 1});
        break;
    case RuleId::VP_VP_WIP_RIGHT:
        children.push_back({NonTerminal::VP, i + 1, split.k});
        children.push_back({NonTerminal::WIP, split.k + 1, j - 1});
        break;
    case RuleId::VP_WIP_VPR:
        children.push_back({NonTerminal::WIP, i + 1, split.k - 1});
        children.push_back({NonTerminal::VPR, split.k, j - 1});
        break;
    case RuleId::VP_VPL_WIP:
        children.push_back({NonTerminal::VPL, i + 1, split.k});
        children.push_back({NonTerminal::WIP, split.k + 1, j - 1});
        break;
    default:
        break;
    }
    return children;
}

pf_t rule_score_vp(RuleId rule,
                   cand_pos_t i,
                   cand_pos_t j,
                   const RuleSplit &split,
                   PartFuncVPContext &ctx,
                   sparse_tree &tree) {
    (void)tree;
    switch (rule) {
    case RuleId::VP_WI_CASE1:
    case RuleId::VP_WI_CASE2:
    case RuleId::VP_WI_CASE3:
        return ctx.scale(2);
    case RuleId::VP_STACK:
        return ctx.get_e_stP(i, j) * ctx.scale(2);
    case RuleId::VP_INTERNAL_LOOP: {
        cand_pos_t u1 = split.k - i - 1;
        cand_pos_t u2 = j - split.l - 1;
        return ctx.get_e_intP(i, split.k, split.l, j) * ctx.scale(u1 + u2 + 2);
    }
    case RuleId::VP_WIP_VP_LEFT:
    case RuleId::VP_VP_WIP_RIGHT:
    case RuleId::VP_WIP_VPR:
    case RuleId::VP_VPL_WIP:
        return ctx.expap_penalty() * ctx.expbp_penalty_sq() * ctx.scale(2);
    default:
        return 0;
    }
}

std::vector<ApplicableRule> applicable_rules_wmbw(cand_pos_t i,
                                                  cand_pos_t j,
                                                  PartFuncWMBWContext &ctx,
                                                  sparse_tree &tree) {
    std::vector<ApplicableRule> out;
    for (RuleId rule : rules_for(NonTerminal::WMBW)) {
        const auto splits = enumerate_splits_wmbw(rule, i, j, ctx, tree);
        for (const auto &split : splits) {
            out.push_back({rule, split});
        }
    }
    return out;
}

std::vector<ApplicableRule> applicable_rules_wmbw(cand_pos_t i,
                                                  cand_pos_t j,
                                                  PartFuncWMBWContext &ctx,
                                                  const StructureView &view) {
    std::vector<ApplicableRule> out;
    for (RuleId rule : rules_for(NonTerminal::WMBW)) {
        const auto splits = enumerate_splits_wmbw(rule, i, j, ctx, view);
        for (const auto &split : splits) {
            out.push_back({rule, split});
        }
    }
    return out;
}

std::vector<RuleSplit> enumerate_splits_wmbw(RuleId rule,
                                             cand_pos_t i,
                                             cand_pos_t j,
                                             PartFuncWMBWContext &ctx,
                                             sparse_tree &tree) {
    (void)ctx;
    std::vector<RuleSplit> splits;
    if (rule != RuleId::WMBW_SPLIT_WMBP_WI) {
        return splits;
    }
    if (!(tree.tree[j].pair < j)) {
        return splits;
    }
    for (cand_pos_t l = i + 1; l < j; l++) {
        if (tree.tree[l].pair < 0 && tree.tree[l].parent->index > -1 && tree.tree[j].parent->index > -1
            && tree.tree[j].parent->index == tree.tree[l].parent->index) {
            splits.push_back({l, -1});
        }
    }
    return splits;
}

std::vector<RuleSplit> enumerate_splits_wmbw(RuleId rule,
                                             cand_pos_t i,
                                             cand_pos_t j,
                                             PartFuncWMBWContext &ctx,
                                             const StructureView &view) {
    (void)ctx;
    std::vector<RuleSplit> splits;
    if (rule != RuleId::WMBW_SPLIT_WMBP_WI) {
        return splits;
    }
    if (!(view.pair_square(j) < j)) {
        return splits;
    }
    const cand_pos_t parent_j = view.parent_index(j);
    for (cand_pos_t l = i + 1; l < j; l++) {
        if (view.is_unpaired(l) && view.parent_index(l) > -1 && parent_j > -1 && parent_j == view.parent_index(l)) {
            splits.push_back({l, -1});
        }
    }
    return splits;
}

std::vector<RuleChild> expand_wmbw(RuleId rule, cand_pos_t i, cand_pos_t j, const RuleSplit &split) {
    std::vector<RuleChild> children;
    if (rule == RuleId::WMBW_SPLIT_WMBP_WI) {
        children.push_back({NonTerminal::WMBP, i, split.k});
        children.push_back({NonTerminal::WI, split.k + 1, j});
    }
    return children;
}

pf_t rule_score_wmbw(RuleId rule, cand_pos_t i, cand_pos_t j, const RuleSplit &split, PartFuncWMBWContext &ctx) {
    (void)i;
    (void)j;
    (void)split;
    (void)ctx;
    switch (rule) {
    case RuleId::WMBW_SPLIT_WMBP_WI:
        return 1;
    default:
        return 0;
    }
}

std::vector<ApplicableRule> applicable_rules_wmbp(cand_pos_t i,
                                                  cand_pos_t j,
                                                  PartFuncWMBPContext &ctx,
                                                  sparse_tree &tree) {
    std::vector<ApplicableRule> out;
    for (RuleId rule : rules_for(NonTerminal::WMBP)) {
        const auto splits = enumerate_splits_wmbp(rule, i, j, ctx, tree);
        for (const auto &split : splits) {
            out.push_back({rule, split});
        }
    }
    return out;
}

std::vector<ApplicableRule> applicable_rules_wmbp(cand_pos_t i,
                                                  cand_pos_t j,
                                                  PartFuncWMBPContext &ctx,
                                                  const StructureView &view,
                                                  sparse_tree &tree) {
    std::vector<ApplicableRule> out;
    for (RuleId rule : rules_for(NonTerminal::WMBP)) {
        const auto splits = enumerate_splits_wmbp(rule, i, j, ctx, view, tree);
        for (const auto &split : splits) {
            out.push_back({rule, split});
        }
    }
    return out;
}

std::vector<RuleSplit> enumerate_splits_wmbp(RuleId rule,
                                             cand_pos_t i,
                                             cand_pos_t j,
                                             PartFuncWMBPContext &ctx,
                                             sparse_tree &tree) {
    std::vector<RuleSplit> splits;
    const scfg::PartFuncModeConfig mode_config{ctx.expPB_penalty(), TURN};
    scfg::PartFuncRuleHelpers rules(tree, mode_config);
    rules.on_traceback_hook(i, j);
    rules.on_fixed_parse_hook(i, j);

    switch (rule) {
    case RuleId::WMBP_SPLIT_BE_WMBP_VP:
    case RuleId::WMBP_SPLIT_BE_WMBW_VP:
        if (rules.pair_at(j) < 0) {
            const cand_pos_t b_ij = rules.border_b(i, j);
            rules.for_each_split(i, j, [&](cand_pos_t l) {
                int ext_case = ctx.compute_exterior_cases(l, j, tree);
                if (rules.allow_exterior_split(l, j, b_ij, ext_case)) {
                    if (rules.has_valid_band_borders(i, l, j)) {
                        const cand_pos_t B_lj = rules.border_B(l, j);
                        const cand_pos_t Bp_lj = rules.border_Bp(l, j);
                        if (rules.parent_within_interval_and_turn(i, l, j)) {
                            splits.push_back({l, -1, B_lj, Bp_lj});
                        }
                    }
                }
            });
        }
        break;
    case RuleId::WMBP_DIRECT_VP:
        splits.push_back({});
        break;
    case RuleId::WMBP_SPLIT_BE_WI_VP:
        if (rules.pair_at(j) < 0 && rules.pair_at(i) >= 0) {
            rules.for_each_split(i, j, [&](cand_pos_t l) {
                if (rules.has_valid_inner_arc_split(i, l, j, ctx.n()) && rules.parent_within_interval_and_turn(i, l, j)) {
                    const cand_pos_t bp_il = rules.border_bp(i, l);
                    splits.push_back({l, -1, bp_il, -1});
                }
            });
        }
        break;
    default:
        break;
    }
    return splits;
}

std::vector<RuleSplit> enumerate_splits_wmbp(RuleId rule,
                                             cand_pos_t i,
                                             cand_pos_t j,
                                             PartFuncWMBPContext &ctx,
                                             const StructureView &view,
                                             sparse_tree &tree) {
    std::vector<RuleSplit> splits;
    const scfg::PartFuncModeConfig mode_config{ctx.expPB_penalty(), TURN};
    scfg::PartFuncRuleHelpersView rules(view, mode_config);
    rules.on_traceback_hook(i, j);
    rules.on_fixed_parse_hook(i, j);

    switch (rule) {
    case RuleId::WMBP_SPLIT_BE_WMBP_VP:
    case RuleId::WMBP_SPLIT_BE_WMBW_VP:
        if (rules.pair_at(j) < 0) {
            const cand_pos_t b_ij = rules.border_b(i, j);
            rules.for_each_split(i, j, [&](cand_pos_t l) {
                int ext_case = ctx.compute_exterior_cases(l, j, tree);
                if (rules.allow_exterior_split(l, j, b_ij, ext_case)) {
                    if (rules.has_valid_band_borders(i, l, j)) {
                        const cand_pos_t B_lj = rules.border_B(l, j);
                        const cand_pos_t Bp_lj = rules.border_Bp(l, j);
                        if (rules.parent_within_interval_and_turn(i, l, j)) {
                            splits.push_back({l, -1, B_lj, Bp_lj});
                        }
                    }
                }
            });
        }
        break;
    case RuleId::WMBP_DIRECT_VP:
        splits.push_back({});
        break;
    case RuleId::WMBP_SPLIT_BE_WI_VP:
        if (rules.pair_at(j) < 0 && rules.pair_at(i) >= 0) {
            rules.for_each_split(i, j, [&](cand_pos_t l) {
                if (rules.has_valid_inner_arc_split(i, l, j, ctx.n()) && rules.parent_within_interval_and_turn(i, l, j)) {
                    const cand_pos_t bp_il = rules.border_bp(i, l);
                    splits.push_back({l, -1, bp_il, -1});
                }
            });
        }
        break;
    default:
        break;
    }
    return splits;
}

std::vector<RuleChild> expand_wmbp(RuleId rule, cand_pos_t i, cand_pos_t j, const RuleSplit &split) {
    std::vector<RuleChild> children;
    switch (rule) {
    case RuleId::WMBP_SPLIT_BE_WMBP_VP:
        children.push_back({NonTerminal::WMBP, i, split.k - 1});
        children.push_back({NonTerminal::VP, split.k, j});
        break;
    case RuleId::WMBP_SPLIT_BE_WMBW_VP:
        children.push_back({NonTerminal::WMBW, i, split.k - 1});
        children.push_back({NonTerminal::VP, split.k, j});
        break;
    case RuleId::WMBP_DIRECT_VP:
        children.push_back({NonTerminal::VP, i, j});
        break;
    case RuleId::WMBP_SPLIT_BE_WI_VP:
        children.push_back({NonTerminal::WI, split.p + 1, split.k - 1});
        children.push_back({NonTerminal::VP, split.k, j});
        break;
    default:
        break;
    }
    return children;
}

pf_t rule_score_wmbp(RuleId rule,
                     cand_pos_t i,
                     cand_pos_t j,
                     const RuleSplit &split,
                     PartFuncWMBPContext &ctx,
                     sparse_tree &tree) {
    switch (rule) {
    case RuleId::WMBP_SPLIT_BE_WMBP_VP:
    case RuleId::WMBP_SPLIT_BE_WMBW_VP: {
        const scfg::PartFuncModeConfig mode_config{ctx.expPB_penalty(), TURN};
        scfg::PartFuncRuleHelpers rules(tree, mode_config);
        rules.on_traceback_hook(i, j);
        rules.on_fixed_parse_hook(i, j);
        pf_t m = ctx.get_BE(tree.tree[split.p].pair, split.p, tree.tree[split.q].pair, split.q, tree);
        return rules.apply_double_pb_penalty(m);
    }
    case RuleId::WMBP_DIRECT_VP:
        return ctx.expPB_penalty();
    case RuleId::WMBP_SPLIT_BE_WI_VP: {
        const scfg::PartFuncModeConfig mode_config{ctx.expPB_penalty(), TURN};
        scfg::PartFuncRuleHelpers rules(tree, mode_config);
        rules.on_traceback_hook(i, j);
        rules.on_fixed_parse_hook(i, j);
        pf_t m = ctx.get_BE(i, tree.tree[i].pair, split.p, tree.tree[split.p].pair, tree);
        return rules.apply_double_pb_penalty(m);
    }
    default:
        return 0;
    }
}

std::vector<ApplicableRule> applicable_rules_wmb(cand_pos_t i,
                                                 cand_pos_t j,
                                                 PartFuncWMBContext &ctx,
                                                 sparse_tree &tree) {
    std::vector<ApplicableRule> out;
    for (RuleId rule : rules_for(NonTerminal::WMB)) {
        const auto splits = enumerate_splits_wmb(rule, i, j, ctx, tree);
        for (const auto &split : splits) {
            out.push_back({rule, split});
        }
    }
    return out;
}

std::vector<ApplicableRule> applicable_rules_wmb(cand_pos_t i,
                                                 cand_pos_t j,
                                                 PartFuncWMBContext &ctx,
                                                 const StructureView &view) {
    std::vector<ApplicableRule> out;
    for (RuleId rule : rules_for(NonTerminal::WMB)) {
        const auto splits = enumerate_splits_wmb(rule, i, j, ctx, view);
        for (const auto &split : splits) {
            out.push_back({rule, split});
        }
    }
    return out;
}

std::vector<RuleSplit> enumerate_splits_wmb(RuleId rule,
                                            cand_pos_t i,
                                            cand_pos_t j,
                                            PartFuncWMBContext &ctx,
                                            sparse_tree &tree) {
    std::vector<RuleSplit> splits;
    if (i == j) {
        if (rule == RuleId::WMB_EMPTY) {
            splits.push_back({});
        }
        return splits;
    }
    switch (rule) {
    case RuleId::WMB_EMPTY:
        break;
    case RuleId::WMB_DIRECT_WMBP:
        splits.push_back({});
        break;
    case RuleId::WMB_SPLIT_BE_WMBP_WI:
        if (tree.tree[j].pair >= 0 && j > tree.tree[j].pair && tree.tree[j].pair > i) {
            cand_pos_t bp_j = tree.tree[j].pair;
            for (cand_pos_t l = (bp_j + 1); (l < j); ++l) {
                cand_pos_t Bp_lj = tree.Bp(l, j);
                if (Bp_lj >= 0 && Bp_lj < ctx.n()) {
                    splits.push_back({l, -1, bp_j, Bp_lj});
                }
            }
        }
        break;
    default:
        break;
    }
    return splits;
}

std::vector<RuleSplit> enumerate_splits_wmb(RuleId rule,
                                            cand_pos_t i,
                                            cand_pos_t j,
                                            PartFuncWMBContext &ctx,
                                            const StructureView &view) {
    std::vector<RuleSplit> splits;
    if (i == j) {
        if (rule == RuleId::WMB_EMPTY) {
            splits.push_back({});
        }
        return splits;
    }
    switch (rule) {
    case RuleId::WMB_EMPTY:
        break;
    case RuleId::WMB_DIRECT_WMBP:
        splits.push_back({});
        break;
    case RuleId::WMB_SPLIT_BE_WMBP_WI:
        if (view.pair_square(j) >= 0 && j > view.pair_square(j) && view.pair_square(j) > i) {
            cand_pos_t bp_j = view.pair_square(j);
            for (cand_pos_t l = (bp_j + 1); (l < j); ++l) {
                cand_pos_t Bp_lj = view.Bp(l, j);
                if (Bp_lj >= 0 && Bp_lj < ctx.n()) {
                    splits.push_back({l, -1, bp_j, Bp_lj});
                }
            }
        }
        break;
    default:
        break;
    }
    return splits;
}

std::vector<RuleChild> expand_wmb(RuleId rule, cand_pos_t i, cand_pos_t j, const RuleSplit &split) {
    std::vector<RuleChild> children;
    switch (rule) {
    case RuleId::WMB_EMPTY:
        break;
    case RuleId::WMB_DIRECT_WMBP:
        children.push_back({NonTerminal::WMBP, i, j});
        break;
    case RuleId::WMB_SPLIT_BE_WMBP_WI:
        children.push_back({NonTerminal::WMBP, i, split.k});
        children.push_back({NonTerminal::WI, split.k + 1, split.q - 1});
        break;
    default:
        break;
    }
    return children;
}

pf_t rule_score_wmb(RuleId rule,
                    cand_pos_t i,
                    cand_pos_t j,
                    const RuleSplit &split,
                    PartFuncWMBContext &ctx,
                    sparse_tree &tree) {
    (void)i;
    (void)j;
    switch (rule) {
    case RuleId::WMB_EMPTY:
        return 0;
    case RuleId::WMB_DIRECT_WMBP:
        return 1;
    case RuleId::WMB_SPLIT_BE_WMBP_WI: {
        pf_t m = ctx.get_BE(split.p, tree.tree[split.p].pair, tree.tree[split.q].pair, split.q, tree);
        return m * ctx.expPB_penalty();
    }
    default:
        return 0;
    }
}

std::vector<ApplicableRule> applicable_rules_be(cand_pos_t i,
                                                cand_pos_t j,
                                                cand_pos_t ip,
                                                cand_pos_t jp,
                                                PartFuncBEContext &ctx,
                                                sparse_tree &tree) {
    std::vector<ApplicableRule> out;
    for (RuleId rule : rules_for(NonTerminal::BE)) {
        const auto splits = enumerate_splits_be(rule, i, j, ip, jp, ctx, tree);
        for (const auto &split : splits) {
            out.push_back({rule, split});
        }
    }
    return out;
}

std::vector<ApplicableRule> applicable_rules_be(cand_pos_t i,
                                                cand_pos_t j,
                                                cand_pos_t ip,
                                                cand_pos_t jp,
                                                PartFuncBEContext &ctx,
                                                const StructureView &view) {
    std::vector<ApplicableRule> out;
    for (RuleId rule : rules_for(NonTerminal::BE)) {
        const auto splits = enumerate_splits_be(rule, i, j, ip, jp, ctx, view);
        for (const auto &split : splits) {
            out.push_back({rule, split});
        }
    }
    return out;
}

std::vector<RuleSplit> enumerate_splits_be(RuleId rule,
                                           cand_pos_t i,
                                           cand_pos_t j,
                                           cand_pos_t ip,
                                           cand_pos_t jp,
                                           PartFuncBEContext &ctx,
                                           sparse_tree &tree) {
    std::vector<RuleSplit> splits;
    if (!(i >= 1 && i <= ip && ip < jp && jp <= j && j <= ctx.n() && tree.tree[i].pair > 0 && tree.tree[j].pair > 0 &&
          tree.tree[ip].pair > 0 && tree.tree[jp].pair > 0 && tree.tree[i].pair == j && tree.tree[j].pair == i &&
          tree.tree[ip].pair == jp && tree.tree[jp].pair == ip)) {
        return splits;
    }
    if (tree.tree[i].pair != j || tree.tree[ip].pair != jp) {
        return splits;
    }
    if (i == ip && j == jp && i < j) {
        if (rule == RuleId::BE_BASE_SAMEPAIR) {
            splits.push_back({});
        }
        return splits;
    }

    switch (rule) {
    case RuleId::BE_STACK:
        if (tree.tree[i + 1].pair == j - 1) {
            splits.push_back({});
        }
        break;
    case RuleId::BE_INTERNAL_LOOP:
    case RuleId::BE_WIP_WIP:
    case RuleId::BE_WIP_BASEPAIR:
    case RuleId::BE_BASEPAIR_WIP:
        for (cand_pos_t l = i + 1; l <= ip; l++) {
            if (tree.tree[l].pair >= -1 && jp <= tree.tree[l].pair && tree.tree[l].pair < j) {
                cand_pos_t lp = tree.tree[l].pair;
                bool empty_region_il = scfg::is_empty_region(tree, i, l);
                bool empty_region_lpj = scfg::is_empty_region(tree, lp, j);
                bool weakly_closed_il = tree.weakly_closed(i + 1, l - 1);
                bool weakly_closed_lpj = tree.weakly_closed(lp + 1, j - 1);

                if (rule == RuleId::BE_INTERNAL_LOOP) {
                    if (empty_region_il && empty_region_lpj) {
                        splits.push_back({l, lp});
                    }
                } else if (rule == RuleId::BE_WIP_WIP) {
                    if (weakly_closed_il && weakly_closed_lpj) {
                        splits.push_back({l, lp});
                    }
                } else if (rule == RuleId::BE_WIP_BASEPAIR) {
                    if (weakly_closed_il && empty_region_lpj) {
                        splits.push_back({l, lp});
                    }
                } else if (rule == RuleId::BE_BASEPAIR_WIP) {
                    if (empty_region_il && weakly_closed_lpj) {
                        splits.push_back({l, lp});
                    }
                }
            }
        }
        break;
    default:
        break;
    }
    return splits;
}

std::vector<RuleSplit> enumerate_splits_be(RuleId rule,
                                           cand_pos_t i,
                                           cand_pos_t j,
                                           cand_pos_t ip,
                                           cand_pos_t jp,
                                           PartFuncBEContext &ctx,
                                           const StructureView &view) {
    std::vector<RuleSplit> splits;
    if (!(i >= 1 && i <= ip && ip < jp && jp <= j && j <= ctx.n() && view.is_pair_square(i, j) &&
          view.is_pair_square(ip, jp))) {
        return splits;
    }
    if (i == ip && j == jp && i < j) {
        if (rule == RuleId::BE_BASE_SAMEPAIR) {
            splits.push_back({});
        }
        return splits;
    }

    switch (rule) {
    case RuleId::BE_STACK:
        if (view.is_pair_square(i + 1, j - 1)) {
            splits.push_back({});
        }
        break;
    case RuleId::BE_INTERNAL_LOOP:
    case RuleId::BE_WIP_WIP:
    case RuleId::BE_WIP_BASEPAIR:
    case RuleId::BE_BASEPAIR_WIP:
        for (cand_pos_t l = i + 1; l <= ip; l++) {
            const cand_pos_t lp = view.pair_square(l);
            if (lp >= -1 && jp <= lp && lp < j) {
                bool empty_region_il = view.is_empty_region(i, l);
                bool empty_region_lpj = view.is_empty_region(lp, j);
                bool weakly_closed_il = view.weakly_closed(i + 1, l - 1);
                bool weakly_closed_lpj = view.weakly_closed(lp + 1, j - 1);

                if (rule == RuleId::BE_INTERNAL_LOOP) {
                    if (empty_region_il && empty_region_lpj) {
                        splits.push_back({l, lp});
                    }
                } else if (rule == RuleId::BE_WIP_WIP) {
                    if (weakly_closed_il && weakly_closed_lpj) {
                        splits.push_back({l, lp});
                    }
                } else if (rule == RuleId::BE_WIP_BASEPAIR) {
                    if (weakly_closed_il && empty_region_lpj) {
                        splits.push_back({l, lp});
                    }
                } else if (rule == RuleId::BE_BASEPAIR_WIP) {
                    if (empty_region_il && weakly_closed_lpj) {
                        splits.push_back({l, lp});
                    }
                }
            }
        }
        break;
    default:
        break;
    }
    return splits;
}

std::vector<RuleChild> expand_be(RuleId rule,
                                 cand_pos_t i,
                                 cand_pos_t j,
                                 cand_pos_t ip,
                                 cand_pos_t jp,
                                 const RuleSplit &split) {
    (void)ip;
    (void)jp;
    std::vector<RuleChild> children;
    switch (rule) {
    case RuleId::BE_BASE_SAMEPAIR:
        break;
    case RuleId::BE_STACK:
        children.push_back({NonTerminal::BE, i + 1, j - 1});
        break;
    case RuleId::BE_INTERNAL_LOOP:
        children.push_back({NonTerminal::BE, split.k, split.l});
        break;
    case RuleId::BE_WIP_WIP:
        children.push_back({NonTerminal::WIP, i + 1, split.k - 1});
        children.push_back({NonTerminal::BE, split.k, split.l});
        children.push_back({NonTerminal::WIP, split.l + 1, j - 1});
        break;
    case RuleId::BE_WIP_BASEPAIR:
        children.push_back({NonTerminal::WIP, i + 1, split.k - 1});
        children.push_back({NonTerminal::BE, split.k, split.l});
        break;
    case RuleId::BE_BASEPAIR_WIP:
        children.push_back({NonTerminal::BE, split.k, split.l});
        children.push_back({NonTerminal::WIP, split.l + 1, j - 1});
        break;
    default:
        break;
    }
    return children;
}

pf_t rule_score_be(RuleId rule,
                   cand_pos_t i,
                   cand_pos_t j,
                   cand_pos_t ip,
                   cand_pos_t jp,
                   const RuleSplit &split,
                   PartFuncBEContext &ctx,
                   sparse_tree &tree) {
    (void)ip;
    (void)jp;
    (void)tree;
    switch (rule) {
    case RuleId::BE_BASE_SAMEPAIR:
        return ctx.scale(2);
    case RuleId::BE_STACK:
        return ctx.get_e_stP(i, j) * ctx.scale(2);
    case RuleId::BE_INTERNAL_LOOP: {
        cand_pos_t u1 = split.k - i - 1;
        cand_pos_t u2 = j - split.l - 1;
        return ctx.get_e_intP(i, split.k, split.l, j) * ctx.scale(u1 + u2 + 2);
    }
    case RuleId::BE_WIP_WIP:
        return ctx.expap_penalty() * ctx.expbp_penalty_sq() * ctx.scale(2);
    case RuleId::BE_WIP_BASEPAIR:
        return ctx.expcp_pen(j - split.l - 1) * ctx.expap_penalty() * ctx.expbp_penalty_sq() * ctx.scale(2);
    case RuleId::BE_BASEPAIR_WIP:
        return ctx.expcp_pen(split.k - i - 1) * ctx.expap_penalty() * ctx.expbp_penalty_sq() * ctx.scale(2);
    default:
        return 0;
    }
}

} // namespace scfg
