#include "scfg/rules_core.hh"

#include "scfg/constraint_oracle.hh"
#include "scfg/legacy_adapter.hh"
#include "scfg/rules_part_func.hh"
#include "scfg/structure_view.hh"
#include "sparse_tree.hh"

#include <ViennaRNA/params/constants.h>

namespace scfg {

// band/pseudoknot 側: VP/VPL/VPR/WIP の分解・スコア・適用判定。

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

pf_t transition_weight_wip(RuleId rule,
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

pf_t transition_weight_vpl(RuleId rule,
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

pf_t transition_weight_vpr(RuleId rule,
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

pf_t transition_weight_vp(RuleId rule,
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

} // namespace scfg
