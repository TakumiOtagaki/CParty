#include "scfg/rules_core.hh"

#include "scfg/constraint_oracle.hh"
#include "scfg/legacy_adapter.hh"
#include "scfg/rules_part_func.hh"
#include "scfg/transition_weights.hh"
#include "sparse_tree.hh"

namespace scfg {

// pk-free 側: W/V/WI の分解・スコア・適用判定。
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
    const RuleSpec &spec = rule_spec(rule);
    if (spec.rhs_len > 0) {
        RuleSpanContext ctx{i, j, split};
        return expand_rule_rhs(spec, ctx);
    }
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

pf_t transition_weight_w(RuleId rule, cand_pos_t i, cand_pos_t j, const RuleSplit &split, PartFuncWContext &ctx) {
    TransitionWeights<PartFuncWContext> oracle(ctx);
    (void)i;
    switch (rule) {
    case RuleId::W_EXTEND_UNPAIRED:
        return oracle.scale1();
    case RuleId::W_SPLIT_V:
        return oracle.exp_Extloop(split.k, j);
    case RuleId::W_SPLIT_WMB:
        return oracle.expPS_penalty();
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
    const RuleSpec &spec = rule_spec(rule);
    std::vector<RuleSplit> splits;
    if (spec.predicate == RuleSpec::PredicateKind::VPairingState) {
        const bool unpaired = (tree.tree[i].pair < -1 && tree.tree[j].pair < -1);
        const bool paired = (tree.tree[i].pair == j && tree.tree[j].pair == i);
        if (!(paired || unpaired)) {
            return splits;
        }
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

pf_t transition_weight_v(RuleId rule,
                  cand_pos_t i,
                  cand_pos_t j,
                  const RuleSplit &split,
                  PartFuncVContext &ctx,
                  sparse_tree &tree) {
    TransitionWeights<PartFuncVContext> oracle(ctx);
    (void)split;
    switch (rule) {
    case RuleId::V_HAIRPIN:
        return oracle.hairpin_energy(i, j);
    case RuleId::V_INTERNAL:
        return oracle.internal_energy(i, j, tree.up);
    case RuleId::V_VM:
        return oracle.vm_energy(i, j, tree.up);
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
    const RuleSpec &spec = rule_spec(rule);
    std::vector<RuleSplit> splits;
    if (i == j) {
        if (rule == RuleId::WI_BASE_SINGLE) {
            splits.push_back({});
        }
        return splits;
    }
    if (spec.split_gen == RuleSpec::SplitGenKind::KRange) {
        RuleSpanContext span_ctx{i, j, {}};
        return enumerate_splits_k_range(spec, span_ctx, ctx.turn());
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
    const RuleSpec &spec = rule_spec(rule);
    if (spec.rhs_len > 0) {
        RuleSpanContext ctx{i, j, split};
        return expand_rule_rhs(spec, ctx);
    }
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

pf_t transition_weight_wi(RuleId rule,
                   cand_pos_t i,
                   cand_pos_t j,
                   const RuleSplit &split,
                   PartFuncWIContext &ctx) {
    TransitionWeights<PartFuncWIContext> oracle(ctx);
    (void)i;
    (void)j;
    (void)split;
    switch (rule) {
    case RuleId::WI_BASE_SINGLE:
        return oracle.expPUP_pen1();
    case RuleId::WI_SPLIT_V:
        return oracle.expPPS_penalty();
    case RuleId::WI_SPLIT_WMB:
        return oracle.expPSP_penalty() * oracle.expPPS_penalty();
    case RuleId::WI_EXTEND_UNPAIRED:
        return oracle.expPUP_pen1();
    default:
        return 0;
    }
}


} // namespace scfg
