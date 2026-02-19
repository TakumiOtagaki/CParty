#include "scfg/rules_core.hh"

#include "scfg/constraint_oracle.hh"
#include "scfg/rules_part_func.hh"
#include "scfg/transition_weights.hh"
#include "scfg/structure_view.hh"
#include "sparse_tree.hh"

namespace scfg {

// band/pseudoknot 側: BE の分解・スコア・適用判定。

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
    const RuleSpec &spec = rule_spec(rule);
    if (spec.rhs_len > 0) {
        RuleSpanContext ctx{i, j, split, ip, jp};
        return expand_rule_rhs(spec, ctx);
    }
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

pf_t transition_weight_be(RuleId rule,
                   cand_pos_t i,
                   cand_pos_t j,
                   cand_pos_t ip,
                   cand_pos_t jp,
                   const RuleSplit &split,
                   PartFuncBEContext &ctx,
                   sparse_tree &tree) {
    TransitionWeights<PartFuncBEContext> oracle(ctx);
    // legacy の子補正（expap/expbp_penalty_sq/scale など）をルール重みに集約。
    (void)ip;
    (void)jp;
    (void)tree;
    switch (rule) {
    case RuleId::BE_BASE_SAMEPAIR:
        return oracle.scale(2);
    case RuleId::BE_STACK:
        return oracle.get_e_stP(i, j) * oracle.scale(2);
    case RuleId::BE_INTERNAL_LOOP: {
        cand_pos_t u1 = split.k - i - 1;
        cand_pos_t u2 = j - split.l - 1;
        return oracle.get_e_intP(i, split.k, split.l, j) * oracle.scale(u1 + u2 + 2);
    }
    case RuleId::BE_WIP_WIP:
        return oracle.expap_penalty() * oracle.expbp_penalty_sq() * oracle.scale(2);
    case RuleId::BE_WIP_BASEPAIR:
        return oracle.expcp_pen(j - split.l - 1) * oracle.expap_penalty() * oracle.expbp_penalty_sq() * oracle.scale(2);
    case RuleId::BE_BASEPAIR_WIP:
        return oracle.expcp_pen(split.k - i - 1) * oracle.expap_penalty() * oracle.expbp_penalty_sq() * oracle.scale(2);
    default:
        return 0;
    }
}

} // namespace scfg
