#include "scfg/rules_core.hh"

#include "scfg/rules_part_helpers.hh"
#include "scfg/rules_part_func.hh"
#include "scfg/transition_weights.hh"
#include "scfg/structure_view.hh"
#include "sparse_tree.hh"

#include <ViennaRNA/params/constants.h>

namespace scfg {

// band/pseudoknot 側: WMBW/WMBP/WMB の分解・スコア・適用判定。

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

    if (spec.predicate == RuleSpec::PredicateKind::WmbpJUnpaired && rules.pair_at(j) >= 0) {
        return splits;
    }
    if (spec.predicate == RuleSpec::PredicateKind::WmbpJUnpairedIpaired) {
        if (rules.pair_at(j) >= 0 || rules.pair_at(i) < 0) {
            return splits;
        }
    }

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
    const RuleSpec &spec = rule_spec(rule);
    std::vector<RuleSplit> splits;
    const scfg::PartFuncModeConfig mode_config{ctx.expPB_penalty(), TURN};
    scfg::PartFuncRuleHelpersView rules(view, mode_config);
    rules.on_traceback_hook(i, j);
    rules.on_fixed_parse_hook(i, j);

    if (spec.predicate == RuleSpec::PredicateKind::WmbpJUnpaired && rules.pair_at(j) >= 0) {
        return splits;
    }
    if (spec.predicate == RuleSpec::PredicateKind::WmbpJUnpairedIpaired) {
        if (rules.pair_at(j) >= 0 || rules.pair_at(i) < 0) {
            return splits;
        }
    }

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
    switch (rule) {
    case RuleId::WMB_EMPTY:
        break;
    case RuleId::WMB_DIRECT_WMBP:
        splits.push_back({});
        break;
    case RuleId::WMB_SPLIT_BE_WMBP_WI:
        if (spec.predicate == RuleSpec::PredicateKind::WmbSplitBeWmbpWi) {
            if (!(tree.tree[j].pair >= 0 && j > tree.tree[j].pair && tree.tree[j].pair > i)) {
                break;
            }
        }
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
    const RuleSpec &spec = rule_spec(rule);
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
        if (spec.predicate == RuleSpec::PredicateKind::WmbSplitBeWmbpWi) {
            if (!(view.pair_square(j) >= 0 && j > view.pair_square(j) && view.pair_square(j) > i)) {
                break;
            }
        }
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
