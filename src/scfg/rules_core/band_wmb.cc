#include "scfg/rules_core.hh"

#include "scfg/rules_part_helpers.hh"
#include "scfg/rules_part_func.hh"
#include "scfg/transition_weights.hh"
#include "scfg/structure_view.hh"
#include "sparse_tree.hh"

#include <ViennaRNA/params/constants.h>

namespace scfg {

// band/pseudoknot 側: WMBW/WMBP/WMB の分解・スコア・適用判定。

static bool split_filter_allows_wmbp(const RuleSpec &spec,
                                     const RuleSpanContext &span_ctx,
                                     PartFuncWMBPContext &ctx,
                                     PartFuncRuleHelpers &rules,
                                     sparse_tree &tree) {
    switch (spec.split_filter) {
    case RuleSpec::SplitFilterKind::WmbpExteriorSplit: {
        const cand_pos_t b_ij = rules.border_b(span_ctx.i, span_ctx.j);
        const int ext_case = ctx.compute_exterior_cases(span_ctx.split.k, span_ctx.j, tree);
        if (!rules.allow_exterior_split(span_ctx.split.k, span_ctx.j, b_ij, ext_case)) return false;
        if (!rules.has_valid_band_borders(span_ctx.i, span_ctx.split.k, span_ctx.j)) return false;
        return rules.parent_within_interval_and_turn(span_ctx.i, span_ctx.split.k, span_ctx.j);
    }
    case RuleSpec::SplitFilterKind::WmbpInnerArcSplit:
        if (!rules.has_valid_inner_arc_split(span_ctx.i, span_ctx.split.k, span_ctx.j, ctx.n())) return false;
        return rules.parent_within_interval_and_turn(span_ctx.i, span_ctx.split.k, span_ctx.j);
    default:
        return true;
    }
}

static bool split_filter_allows_wmbp(const RuleSpec &spec,
                                     const RuleSpanContext &span_ctx,
                                     PartFuncWMBPContext &ctx,
                                     PartFuncRuleHelpersView &rules,
                                     sparse_tree &tree) {
    switch (spec.split_filter) {
    case RuleSpec::SplitFilterKind::WmbpExteriorSplit: {
        const cand_pos_t b_ij = rules.border_b(span_ctx.i, span_ctx.j);
        const int ext_case = ctx.compute_exterior_cases(span_ctx.split.k, span_ctx.j, tree);
        if (!rules.allow_exterior_split(span_ctx.split.k, span_ctx.j, b_ij, ext_case)) return false;
        if (!rules.has_valid_band_borders(span_ctx.i, span_ctx.split.k, span_ctx.j)) return false;
        return rules.parent_within_interval_and_turn(span_ctx.i, span_ctx.split.k, span_ctx.j);
    }
    case RuleSpec::SplitFilterKind::WmbpInnerArcSplit:
        if (!rules.has_valid_inner_arc_split(span_ctx.i, span_ctx.split.k, span_ctx.j, ctx.n())) return false;
        return rules.parent_within_interval_and_turn(span_ctx.i, span_ctx.split.k, span_ctx.j);
    default:
        return true;
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
    for (cand_pos_t l = i + 1; l < j; l++) {
        RuleSpanContext span_ctx{i, j, {l, -1}};
        if (predicate_allows(rule_spec(rule), span_ctx, tree)) {
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
    for (cand_pos_t l = i + 1; l < j; l++) {
        RuleSpanContext span_ctx{i, j, {l, -1}};
        if (predicate_allows(rule_spec(rule), span_ctx, view)) {
            splits.push_back({l, -1});
        }
    }
    return splits;
}

std::vector<RuleChild> expand_wmbw(RuleId rule, cand_pos_t i, cand_pos_t j, const RuleSplit &split) {
    const RuleSpec &spec = rule_spec(rule);
    if (spec.rhs_len > 0) {
        RuleSpanContext ctx{i, j, split};
        return expand_rule_rhs(spec, ctx);
    }
    std::vector<RuleChild> children;
    if (rule == RuleId::WMBW_SPLIT_WMBP_WI) {
        children.push_back({NonTerminal::WMBP, i, split.k});
        children.push_back({NonTerminal::WI, split.k + 1, j});
    }
    return children;
}

pf_t transition_weight_wmbw(RuleId rule, cand_pos_t i, cand_pos_t j, const RuleSplit &split, PartFuncWMBWContext &ctx) {
    TransitionWeights<PartFuncWMBWContext> oracle(ctx);
    // child 側に補正は掛けず、必要な係数はルール重み側に集約する方針。
    (void)i;
    (void)j;
    (void)split;
    (void)oracle;
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
    const RuleSpec &spec = rule_spec(rule);
    std::vector<RuleSplit> splits;
    const scfg::PartFuncModeConfig mode_config{ctx.expPB_penalty(), TURN};
    scfg::PartFuncRuleHelpers rules(tree, mode_config);
    rules.on_traceback_hook(i, j);
    rules.on_fixed_parse_hook(i, j);
    RuleSpanContext span_ctx{i, j, {}};
    if (!predicate_allows(spec, span_ctx, rules)) {
        return splits;
    }

    switch (rule) {
    case RuleId::WMBP_SPLIT_BE_WMBP_VP:
    case RuleId::WMBP_SPLIT_BE_WMBW_VP:
        if (rules.pair_at(j) < 0) {
            rules.for_each_split(i, j, [&](cand_pos_t l) {
                RuleSpanContext split_ctx{i, j, {l, -1}};
                if (!split_filter_allows_wmbp(spec, split_ctx, ctx, rules, tree)) return;
                const cand_pos_t B_lj = rules.border_B(l, j);
                const cand_pos_t Bp_lj = rules.border_Bp(l, j);
                splits.push_back({l, -1, B_lj, Bp_lj});
            });
        }
        break;
    case RuleId::WMBP_DIRECT_VP:
        splits.push_back({});
        break;
    case RuleId::WMBP_SPLIT_BE_WI_VP:
        if (rules.pair_at(j) < 0 && rules.pair_at(i) >= 0) {
            rules.for_each_split(i, j, [&](cand_pos_t l) {
                RuleSpanContext split_ctx{i, j, {l, -1}};
                if (!split_filter_allows_wmbp(spec, split_ctx, ctx, rules, tree)) return;
                const cand_pos_t bp_il = rules.border_bp(i, l);
                splits.push_back({l, -1, bp_il, -1});
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
    const RuleSpec &spec = rule_spec(rule);
    std::vector<RuleSplit> splits;
    const scfg::PartFuncModeConfig mode_config{ctx.expPB_penalty(), TURN};
    scfg::PartFuncRuleHelpersView rules(view, mode_config);
    rules.on_traceback_hook(i, j);
    rules.on_fixed_parse_hook(i, j);
    RuleSpanContext span_ctx{i, j, {}};
    if (!predicate_allows(spec, span_ctx, rules)) {
        return splits;
    }

    switch (rule) {
    case RuleId::WMBP_SPLIT_BE_WMBP_VP:
    case RuleId::WMBP_SPLIT_BE_WMBW_VP:
        if (rules.pair_at(j) < 0) {
            rules.for_each_split(i, j, [&](cand_pos_t l) {
                RuleSpanContext split_ctx{i, j, {l, -1}};
                if (!split_filter_allows_wmbp(spec, split_ctx, ctx, rules, tree)) return;
                const cand_pos_t B_lj = rules.border_B(l, j);
                const cand_pos_t Bp_lj = rules.border_Bp(l, j);
                splits.push_back({l, -1, B_lj, Bp_lj});
            });
        }
        break;
    case RuleId::WMBP_DIRECT_VP:
        splits.push_back({});
        break;
    case RuleId::WMBP_SPLIT_BE_WI_VP:
        if (rules.pair_at(j) < 0 && rules.pair_at(i) >= 0) {
            rules.for_each_split(i, j, [&](cand_pos_t l) {
                RuleSpanContext split_ctx{i, j, {l, -1}};
                if (!split_filter_allows_wmbp(spec, split_ctx, ctx, rules, tree)) return;
                const cand_pos_t bp_il = rules.border_bp(i, l);
                splits.push_back({l, -1, bp_il, -1});
            });
        }
        break;
    default:
        break;
    }
    return splits;
}

std::vector<RuleChild> expand_wmbp(RuleId rule, cand_pos_t i, cand_pos_t j, const RuleSplit &split) {
    const RuleSpec &spec = rule_spec(rule);
    if (spec.rhs_len > 0) {
        RuleSpanContext ctx{i, j, split};
        return expand_rule_rhs(spec, ctx);
    }
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

pf_t transition_weight_wmbp(RuleId rule,
                     cand_pos_t i,
                     cand_pos_t j,
                     const RuleSplit &split,
                     PartFuncWMBPContext &ctx,
                     sparse_tree &tree) {
    TransitionWeights<PartFuncWMBPContext> oracle(ctx);
    // legacy の補正（expPB_penalty / double_pb_penalty など）をルール重みに集約。
    switch (rule) {
    case RuleId::WMBP_SPLIT_BE_WMBP_VP:
    case RuleId::WMBP_SPLIT_BE_WMBW_VP: {
        const scfg::PartFuncModeConfig mode_config{oracle.expPB_penalty(), TURN};
        scfg::PartFuncRuleHelpers rules(tree, mode_config);
        rules.on_traceback_hook(i, j);
        rules.on_fixed_parse_hook(i, j);
        pf_t m = ctx.get_BE(tree.tree[split.p].pair, split.p, tree.tree[split.q].pair, split.q, tree);
        return rules.apply_double_pb_penalty(m);
    }
    case RuleId::WMBP_DIRECT_VP:
        return oracle.expPB_penalty();
    case RuleId::WMBP_SPLIT_BE_WI_VP: {
        const scfg::PartFuncModeConfig mode_config{oracle.expPB_penalty(), TURN};
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
    const RuleSpec &spec = rule_spec(rule);
    std::vector<RuleSplit> splits;
    if (i == j) {
        if (rule == RuleId::WMB_EMPTY) {
            splits.push_back({});
        }
        return splits;
    }
    RuleSpanContext span_ctx{i, j, {}};
    if (!predicate_allows(spec, span_ctx, tree)) {
        return splits;
    }
    switch (rule) {
    case RuleId::WMB_EMPTY:
        break;
    case RuleId::WMB_DIRECT_WMBP:
        splits.push_back({});
        break;
    case RuleId::WMB_SPLIT_BE_WMBP_WI:
        {
            const cand_pos_t bp_j = tree.tree[j].pair;
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
    const RuleSpec &spec = rule_spec(rule);
    std::vector<RuleSplit> splits;
    if (i == j) {
        if (rule == RuleId::WMB_EMPTY) {
            splits.push_back({});
        }
        return splits;
    }
    RuleSpanContext span_ctx{i, j, {}};
    if (!predicate_allows(spec, span_ctx, view)) {
        return splits;
    }
    switch (rule) {
    case RuleId::WMB_EMPTY:
        break;
    case RuleId::WMB_DIRECT_WMBP:
        splits.push_back({});
        break;
    case RuleId::WMB_SPLIT_BE_WMBP_WI:
        {
            const cand_pos_t bp_j = view.pair_square(j);
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
    const RuleSpec &spec = rule_spec(rule);
    if (spec.rhs_len > 0) {
        RuleSpanContext ctx{i, j, split};
        return expand_rule_rhs(spec, ctx);
    }
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

pf_t transition_weight_wmb(RuleId rule,
                    cand_pos_t i,
                    cand_pos_t j,
                    const RuleSplit &split,
                    PartFuncWMBContext &ctx,
                    sparse_tree &tree) {
    TransitionWeights<PartFuncWMBContext> oracle(ctx);
    // legacy の補正（expPB_penalty など）をルール重みに集約。
    (void)i;
    (void)j;
    switch (rule) {
    case RuleId::WMB_EMPTY:
        return 0;
    case RuleId::WMB_DIRECT_WMBP:
        return 1;
    case RuleId::WMB_SPLIT_BE_WMBP_WI: {
        pf_t m = ctx.get_BE(split.p, tree.tree[split.p].pair, tree.tree[split.q].pair, split.q, tree);
        return m * oracle.expPB_penalty();
    }
    default:
        return 0;
    }
}

} // namespace scfg
