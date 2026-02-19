#include "scfg/rules_core.hh"

#include "scfg/constraint_oracle.hh"
#include "scfg/legacy_adapter.hh"
#include "scfg/rules_part_func.hh"
#include "scfg/transition_weights.hh"
#include "scfg/structure_view.hh"
#include "sparse_tree.hh"

#include <ViennaRNA/params/constants.h>

namespace scfg {

// band/pseudoknot 側: VP/VPL/VPR/WIP の分解・スコア・適用判定。

template <typename MinFn, typename MaxFn, typename CanLeftFn, typename CanRightFn>
static inline std::vector<RuleSplit> enumerate_splits_band_range(const RuleSpec &spec,
                                                                  cand_pos_t i,
                                                                  cand_pos_t j,
                                                                  MinFn min_fn,
                                                                  MaxFn max_fn,
                                                                  CanLeftFn can_left,
                                                                  CanRightFn can_right) {
    std::vector<RuleSplit> splits;
    switch (spec.split_gen) {
    case RuleSpec::SplitGenKind::BandMinBpRange: {
        const cand_pos_t min_bp = min_fn(i, j);
        for (cand_pos_t k = i + 1; k < min_bp; ++k) {
            if (spec.split_filter == RuleSpec::SplitFilterKind::CanPairLeft && !can_left(i, k)) continue;
            splits.push_back({k, -1});
        }
    } break;
    case RuleSpec::SplitGenKind::BandMaxBpRange: {
        const cand_pos_t max_bp = max_fn(i, j);
        for (cand_pos_t k = max_bp + 1; k < j; ++k) {
            if (spec.split_filter == RuleSpec::SplitFilterKind::CanPairRight && !can_right(k, j)) continue;
            splits.push_back({k, -1});
        }
    } break;
    default:
        break;
    }
    return splits;
}

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
    const RuleSpec &spec = rule_spec(rule);
    if (spec.predicate == RuleSpec::PredicateKind::UnpairedAtJ && tree.tree[j].pair >= 0) {
        return {};
    }
    if (spec.split_gen == RuleSpec::SplitGenKind::KRange) {
        RuleSpanContext span_ctx{i, j, {}};
        return enumerate_splits_k_range(spec, span_ctx, ctx.turn());
    }
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
    const RuleSpec &spec = rule_spec(rule);
    if (spec.predicate == RuleSpec::PredicateKind::UnpairedAtJ && !view.is_unpaired(j)) {
        return {};
    }
    if (spec.split_gen == RuleSpec::SplitGenKind::KRange) {
        RuleSpanContext span_ctx{i, j, {}};
        return enumerate_splits_k_range(spec, span_ctx, ctx.turn());
    }
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
    const RuleSpec &spec = rule_spec(rule);
    if (spec.rhs_len > 0) {
        RuleSpanContext ctx{i, j, split};
        return expand_rule_rhs(spec, ctx);
    }
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
    TransitionWeights<PartFuncWIPContext> oracle(ctx);
    // legacy の子補正（expbp_penalty / expPSM_penalty / expcp_pen）をルール重みに集約。
    (void)j;
    switch (rule) {
    case RuleId::WIP_BASE_V:
    case RuleId::WIP_SPLIT_V:
        return oracle.expbp_penalty();
    case RuleId::WIP_BASE_WMB:
    case RuleId::WIP_SPLIT_WMB:
        return oracle.expbp_penalty() * oracle.expPSM_penalty();
    case RuleId::WIP_BASEPAIR_V:
        return oracle.expcp_pen(split.k - i) * oracle.expbp_penalty();
    case RuleId::WIP_BASEPAIR_WMB:
        return oracle.expcp_pen(split.k - i) * oracle.expbp_penalty() * oracle.expPSM_penalty();
    case RuleId::WIP_EXTEND_UNPAIRED:
        return oracle.expcp_pen(1);
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
    const RuleSpec &spec = rule_spec(rule);
    if (spec.split_gen == RuleSpec::SplitGenKind::BandMinBpRange) {
        auto min_fn = [&](cand_pos_t li, cand_pos_t lj) {
            return std::min((cand_pos_tu)tree.b(li, lj), (cand_pos_tu)tree.Bp(li, lj));
        };
        auto max_fn = [&](cand_pos_t, cand_pos_t) { return static_cast<cand_pos_t>(-1); };
        auto can_left = [&](cand_pos_t li, cand_pos_t lk) { return scfg::can_pair_left_span(tree, li, lk); };
        auto can_right = [&](cand_pos_t, cand_pos_t) { return true; };
        return enumerate_splits_band_range(spec, i, j, min_fn, max_fn, can_left, can_right);
    }
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
    const RuleSpec &spec = rule_spec(rule);
    if (spec.split_gen == RuleSpec::SplitGenKind::BandMinBpRange) {
        auto min_fn = [&](cand_pos_t li, cand_pos_t lj) {
            return std::min((cand_pos_tu)view.b(li, lj), (cand_pos_tu)view.Bp(li, lj));
        };
        auto max_fn = [&](cand_pos_t, cand_pos_t) { return static_cast<cand_pos_t>(-1); };
        auto can_left = [&](cand_pos_t li, cand_pos_t lk) { return view.can_pair_left_span(li, lk); };
        auto can_right = [&](cand_pos_t, cand_pos_t) { return true; };
        return enumerate_splits_band_range(spec, i, j, min_fn, max_fn, can_left, can_right);
    }
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
    const RuleSpec &spec = rule_spec(rule);
    if (spec.rhs_len > 0) {
        RuleSpanContext ctx{i, j, split};
        return expand_rule_rhs(spec, ctx);
    }
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
    TransitionWeights<PartFuncVPLContext> oracle(ctx);
    // legacy の子補正（expcp_pen）をルール重みに集約。
    (void)j;
    switch (rule) {
    case RuleId::VPL_SPLIT_VP:
        return oracle.expcp_pen(split.k - i);
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
    const RuleSpec &spec = rule_spec(rule);
    if (spec.split_gen == RuleSpec::SplitGenKind::BandMaxBpRange) {
        auto min_fn = [&](cand_pos_t, cand_pos_t) { return static_cast<cand_pos_t>(-1); };
        auto max_fn = [&](cand_pos_t li, cand_pos_t lj) { return std::max(tree.B(li, lj), tree.bp(li, lj)); };
        auto can_left = [&](cand_pos_t, cand_pos_t) { return true; };
        auto can_right = [&](cand_pos_t lk, cand_pos_t lj) { return scfg::can_pair_right_span(tree, lk, lj); };
        if (spec.predicate == RuleSpec::PredicateKind::VprBasepair) {
            std::vector<RuleSplit> splits;
            const cand_pos_t max_bp = max_fn(i, j);
            for (cand_pos_t k = max_bp + 1; k < j; ++k) {
                if (!can_right(k, j)) continue;
                splits.push_back({k, -1});
            }
            return splits;
        }
        return enumerate_splits_band_range(spec, i, j, min_fn, max_fn, can_left, can_right);
    }
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
    const RuleSpec &spec = rule_spec(rule);
    if (spec.split_gen == RuleSpec::SplitGenKind::BandMaxBpRange) {
        auto min_fn = [&](cand_pos_t, cand_pos_t) { return static_cast<cand_pos_t>(-1); };
        auto max_fn = [&](cand_pos_t li, cand_pos_t lj) { return std::max(view.B(li, lj), view.bp(li, lj)); };
        auto can_left = [&](cand_pos_t, cand_pos_t) { return true; };
        auto can_right = [&](cand_pos_t lk, cand_pos_t lj) { return view.can_pair_right_span(lk, lj); };
        if (spec.predicate == RuleSpec::PredicateKind::VprBasepair) {
            std::vector<RuleSplit> splits;
            const cand_pos_t max_bp = max_fn(i, j);
            for (cand_pos_t k = max_bp + 1; k < j; ++k) {
                if (!can_right(k, j)) continue;
                splits.push_back({k, -1});
            }
            return splits;
        }
        return enumerate_splits_band_range(spec, i, j, min_fn, max_fn, can_left, can_right);
    }
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
    const RuleSpec &spec = rule_spec(rule);
    if (spec.rhs_len > 0) {
        RuleSpanContext ctx{i, j, split};
        return expand_rule_rhs(spec, ctx);
    }
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
    TransitionWeights<PartFuncVPRContext> oracle(ctx);
    // legacy の子補正（expcp_pen）をルール重みに集約。
    (void)j;
    switch (rule) {
    case RuleId::VPR_SPLIT_VP_WIP:
        return 1;
    case RuleId::VPR_SPLIT_VP_BASEPAIR:
        return oracle.expcp_pen(split.k - i);
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
    const RuleSpec &spec = rule_spec(rule);
    std::vector<RuleSplit> splits;
    cand_pos_t Bp_ij = tree.Bp(i, j);
    cand_pos_t B_ij = tree.B(i, j);
    cand_pos_t b_ij = tree.b(i, j);
    cand_pos_t bp_ij = tree.bp(i, j);

    switch (rule) {
    case RuleId::VP_WI_CASE1:
        if (spec.predicate == RuleSpec::PredicateKind::VpWiCase1 &&
            (tree.tree[i].parent->index) > 0 && (tree.tree[j].parent->index) < (tree.tree[i].parent->index) &&
            Bp_ij >= 0 && B_ij >= 0 && bp_ij < 0) {
            splits.push_back({-1, -1, Bp_ij, B_ij});
        }
        break;
    case RuleId::VP_WI_CASE2:
        if (spec.predicate == RuleSpec::PredicateKind::VpWiCase2 &&
            (tree.tree[i].parent->index) < (tree.tree[j].parent->index) && (tree.tree[j].parent->index) > 0 &&
            b_ij >= 0 && bp_ij >= 0 && Bp_ij < 0) {
            splits.push_back({-1, -1, b_ij, bp_ij});
        }
        break;
    case RuleId::VP_WI_CASE3:
        if (spec.predicate == RuleSpec::PredicateKind::VpWiCase3 &&
            (tree.tree[i].parent->index) > 0 && (tree.tree[j].parent->index) > 0 && Bp_ij >= 0 && B_ij >= 0 &&
            b_ij >= 0 && bp_ij >= 0) {
            splits.push_back({b_ij, bp_ij, Bp_ij, B_ij});
        }
        break;
    case RuleId::VP_STACK: {
        if (spec.predicate == RuleSpec::PredicateKind::VpStackPairing) {
            pair_type ptype_closingip1jm1 = ctx.pair_type_of(i + 1, j - 1);
            if ((tree.tree[i + 1].pair) < -1 && (tree.tree[j - 1].pair) < -1 &&
                scfg::is_pair_type_allowed(ptype_closingip1jm1)) {
                splits.push_back({});
            }
            break;
        }
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
                    if (k == i + 1 && l == j - 1) continue;
                    pair_type ptype_closingkj = ctx.pair_type_of(k, l);
                    if (scfg::is_unpaired_position(tree, l) &&
                        (spec.predicate != RuleSpec::PredicateKind::VpInternalLoopPairing ||
                         scfg::is_pair_type_allowed(ptype_closingkj)) &&
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
    const RuleSpec &spec = rule_spec(rule);
    std::vector<RuleSplit> splits;
    cand_pos_t Bp_ij = view.Bp(i, j);
    cand_pos_t B_ij = view.B(i, j);
    cand_pos_t b_ij = view.b(i, j);
    cand_pos_t bp_ij = view.bp(i, j);

    switch (rule) {
    case RuleId::VP_WI_CASE1:
        if (spec.predicate == RuleSpec::PredicateKind::VpWiCase1 &&
            view.parent_index(i) > 0 && view.parent_index(j) < view.parent_index(i) && Bp_ij >= 0 && B_ij >= 0 &&
            bp_ij < 0) {
            splits.push_back({-1, -1, Bp_ij, B_ij});
        }
        break;
    case RuleId::VP_WI_CASE2:
        if (spec.predicate == RuleSpec::PredicateKind::VpWiCase2 &&
            view.parent_index(i) < view.parent_index(j) && view.parent_index(j) > 0 && b_ij >= 0 && bp_ij >= 0 &&
            Bp_ij < 0) {
            splits.push_back({-1, -1, b_ij, bp_ij});
        }
        break;
    case RuleId::VP_WI_CASE3:
        if (spec.predicate == RuleSpec::PredicateKind::VpWiCase3 &&
            view.parent_index(i) > 0 && view.parent_index(j) > 0 && Bp_ij >= 0 && B_ij >= 0 && b_ij >= 0 &&
            bp_ij >= 0) {
            splits.push_back({b_ij, bp_ij, Bp_ij, B_ij});
        }
        break;
    case RuleId::VP_STACK: {
        if (spec.predicate == RuleSpec::PredicateKind::VpStackPairing) {
            pair_type ptype_closingip1jm1 = ctx.pair_type_of(i + 1, j - 1);
            if (view.is_unpaired(i + 1) && view.is_unpaired(j - 1) &&
                scfg::is_pair_type_allowed(ptype_closingip1jm1)) {
                splits.push_back({});
            }
            break;
        }
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
                    if (k == i + 1 && l == j - 1) continue;
                    pair_type ptype_closingkj = ctx.pair_type_of(k, l);
                    if (view.is_unpaired(l) &&
                        (spec.predicate != RuleSpec::PredicateKind::VpInternalLoopPairing ||
                         scfg::is_pair_type_allowed(ptype_closingkj)) &&
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
    const RuleSpec &spec = rule_spec(rule);
    if (spec.rhs_len > 0) {
        RuleSpanContext ctx{i, j, split};
        return expand_rule_rhs(spec, ctx);
    }
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
    TransitionWeights<PartFuncVPContext> oracle(ctx);
    // legacy の子補正（expap/expbp_penalty_sq/scale など）をルール重みに集約。
    (void)tree;
    switch (rule) {
    case RuleId::VP_WI_CASE1:
    case RuleId::VP_WI_CASE2:
    case RuleId::VP_WI_CASE3:
        return oracle.scale(2);
    case RuleId::VP_STACK:
        return oracle.get_e_stP(i, j) * oracle.scale(2);
    case RuleId::VP_INTERNAL_LOOP: {
        cand_pos_t u1 = split.k - i - 1;
        cand_pos_t u2 = j - split.l - 1;
        return oracle.get_e_intP(i, split.k, split.l, j) * oracle.scale(u1 + u2 + 2);
    }
    case RuleId::VP_WIP_VP_LEFT:
    case RuleId::VP_VP_WIP_RIGHT:
    case RuleId::VP_WIP_VPR:
    case RuleId::VP_VPL_WIP:
        return oracle.expap_penalty() * oracle.expbp_penalty_sq() * oracle.scale(2);
    default:
        return 0;
    }
}

} // namespace scfg
