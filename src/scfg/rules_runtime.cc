#include "scfg/rules_runtime.hh"

#include "scfg/constraint_oracle.hh"
#include "scfg/legacy_adapter.hh"
#include "scfg/rules_core.hh"
#include "scfg/rules_part_func.hh"

#include <ViennaRNA/params/constants.h>

#include <algorithm>

namespace scfg {

void compute_V_restricted_rules(PartFuncVContext &ctx, cand_pos_t i, cand_pos_t j, sparse_tree &tree, const RulesConfig &config) {
    const cand_pos_t ij = ctx.index_of(i, j);
    pf_t contributions = 0;
    for (RuleId rule : rules_for(NonTerminal::V)) {
        if (!is_rule_enabled(config, rule)) continue;
        const auto splits = enumerate_splits_v(rule, i, j, ctx, tree);
        for (const auto &split : splits) {
            record_rule_hit(rule);
            contributions += rule_score_v(rule, i, j, split, ctx, tree);
        }
    }

    ctx.set_V(ij, contributions);
}

pf_t compute_VM_restricted_rules(PartFuncVMContext &ctx, cand_pos_t i, cand_pos_t j, std::vector<int> &up, const RulesConfig &config) {
    pf_t contributions = 0;
    const cand_pos_t ij = ctx.index_of(i, j);
    const bool apply_scale2 = is_rule_enabled(config, RuleId::VM_SCALE2);
    for (RuleId rule : rules_for(NonTerminal::VM)) {
        if (rule == RuleId::VM_SCALE2) continue;
        if (!is_rule_enabled(config, rule)) continue;
        const auto splits = enumerate_splits_vm(rule, i, j, ctx, up);
        for (const auto &split : splits) {
            record_rule_hit(rule);
            pf_t coeff = rule_score_vm(rule, i, j, split, ctx, up);
            pf_t term = 1;
            const auto children = expand_vm(rule, i, j, split);
            for (const auto &child : children) {
                if (child.nonterminal == NonTerminal::WM) {
                    term *= ctx.get_energy_WM(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::WMv) {
                    term *= ctx.get_energy_WMv(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::WMp) {
                    term *= ctx.get_energy_WMp(child.i, child.j);
                }
            }
            if (rule == RuleId::VM_SPLIT_WMp_BASE) {
                term *= ctx.expMLbase(split.k - i - 1);
            }
            contributions += term * coeff;
        }
    }
    if (apply_scale2) {
        record_rule_hit(RuleId::VM_SCALE2);
        contributions *= ctx.scale2();
    }
    ctx.set_VM(ij, contributions);
    return contributions;
}

void compute_W_restricted_rules(PartFuncWContext &ctx, sparse_tree &tree, const RulesConfig &config) {
    const cand_pos_t n = ctx.n();
    const cand_pos_t turn = ctx.turn();

    for (cand_pos_t j = turn + 1; j <= n; j++) {
        pf_t contributions = 0;
        for (RuleId rule : rules_for(NonTerminal::W)) {
            if (!is_rule_enabled(config, rule)) continue;
            const auto splits = enumerate_splits_w(rule, 1, j, ctx, tree);
            for (const auto &split : splits) {
                record_rule_hit(rule);
                pf_t coeff = rule_score_w(rule, 1, j, split, ctx);
                pf_t term = 1;
                const auto children = expand_w(rule, 1, j, split);
                for (const auto &child : children) {
                    if (child.nonterminal == NonTerminal::W) {
                        term *= ctx.get_W(child.j);
                    } else if (child.nonterminal == NonTerminal::V) {
                        term *= ctx.get_energy(child.i, child.j);
                    } else if (child.nonterminal == NonTerminal::WMB) {
                        term *= ctx.get_energy_WMB(child.i, child.j);
                    }
                }
                contributions += term * coeff;
            }
        }
        ctx.set_W(j, contributions);
    }
}

void compute_WI_restricted_rules(PartFuncWIContext &ctx, cand_pos_t i, cand_pos_t j, sparse_tree &tree, const RulesConfig &config) {
    const cand_pos_t ij = ctx.index_of(i, j);
    pf_t contributions = 0;
    for (RuleId rule : rules_for(NonTerminal::WI)) {
        if (!is_rule_enabled(config, rule)) continue;
        const auto splits = enumerate_splits_wi(rule, i, j, ctx, tree);
        for (const auto &split : splits) {
            record_rule_hit(rule);
            pf_t coeff = rule_score_wi(rule, i, j, split, ctx);
            pf_t term = 1;
            const auto children = expand_wi(rule, i, j, split);
            for (const auto &child : children) {
                if (child.nonterminal == NonTerminal::WI) {
                    term *= ctx.get_WI(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::V) {
                    term *= ctx.get_energy(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::WMB) {
                    term *= ctx.get_energy_WMB(child.i, child.j);
                }
            }
            contributions += term * coeff;
        }
    }

    ctx.set_WI(ij, contributions);
}

void compute_WMv_WMp_restricted_rules(PartFuncWMvWMpContext &ctx, cand_pos_t i, cand_pos_t j, std::vector<Node> &tree, const RulesConfig &config) {
    const cand_pos_t ij = ctx.index_of(i, j);
    pf_t WMv_contributions = 0;
    pf_t WMp_contributions = 0;

    for (RuleId rule : rules_for(NonTerminal::WMv)) {
        if (!is_rule_enabled(config, rule)) continue;
        const auto splits = enumerate_splits_wmv_wmp(rule, i, j, ctx, tree);
        for (const auto &split : splits) {
            record_rule_hit(rule);
            pf_t coeff = rule_score_wmv_wmp(rule, i, j, split, ctx, tree);
            pf_t term = 1;
            const auto children = expand_wmv_wmp(rule, i, j, split);
            for (const auto &child : children) {
                if (child.nonterminal == NonTerminal::V) {
                    term *= ctx.get_energy(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::WMv) {
                    term *= ctx.get_energy_WMv(child.i, child.j);
                }
            }
            WMv_contributions += term * coeff;
        }
    }

    for (RuleId rule : rules_for(NonTerminal::WMp)) {
        if (!is_rule_enabled(config, rule)) continue;
        const auto splits = enumerate_splits_wmv_wmp(rule, i, j, ctx, tree);
        for (const auto &split : splits) {
            record_rule_hit(rule);
            pf_t coeff = rule_score_wmv_wmp(rule, i, j, split, ctx, tree);
            pf_t term = 1;
            const auto children = expand_wmv_wmp(rule, i, j, split);
            for (const auto &child : children) {
                if (child.nonterminal == NonTerminal::WMB) {
                    term *= ctx.get_energy_WMB(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::WMp) {
                    term *= ctx.get_energy_WMp(child.i, child.j);
                }
            }
            WMp_contributions += term * coeff;
        }
    }

    ctx.set_WMv_WMp(ij, WMv_contributions, WMp_contributions);
}

void compute_WM_restricted_rules(PartFuncWMContext &ctx, cand_pos_t i, cand_pos_t j, sparse_tree &tree, const RulesConfig &config) {
    pf_t contributions = 0;
    const cand_pos_t ij = ctx.index_of(i, j);

    for (RuleId rule : rules_for(NonTerminal::WM)) {
        if (!is_rule_enabled(config, rule)) continue;
        const auto splits = enumerate_splits_wm(rule, i, j, ctx, tree);
        for (const auto &split : splits) {
            record_rule_hit(rule);
            pf_t coeff = rule_score_wm(rule, i, j, split, ctx);
            pf_t term = 1;
            const auto children = expand_wm(rule, i, j, split);
            for (const auto &child : children) {
                if (child.nonterminal == NonTerminal::WM) {
                    term *= ctx.get_energy_WM(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::V) {
                    term *= ctx.get_energy(child.i, child.j);
                    term *= ctx.exp_MLstem(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::WMB) {
                    term *= ctx.get_energy_WMB(child.i, child.j);
                    term *= ctx.expPSM_penalty() * ctx.expb_penalty();
                }
            }
            contributions += term * coeff;
        }
    }
    ctx.set_WM(ij, contributions);
}

void compute_WIP_restricted_rules(PartFuncWIPContext &ctx, cand_pos_t i, cand_pos_t j, sparse_tree &tree, const RulesConfig &config) {
    const cand_pos_t ij = ctx.index_of(i, j);
    pf_t contributions = 0;
    for (RuleId rule : rules_for(NonTerminal::WIP)) {
        if (!is_rule_enabled(config, rule)) continue;
        const auto splits = enumerate_splits_wip(rule, i, j, ctx, tree);
        for (const auto &split : splits) {
            record_rule_hit(rule);
            pf_t coeff = rule_score_wip(rule, i, j, split, ctx);
            pf_t term = 1;
            const auto children = expand_wip(rule, i, j, split);
            for (const auto &child : children) {
                if (child.nonterminal == NonTerminal::WIP) {
                    term *= ctx.get_energy_WIP(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::V) {
                    term *= ctx.get_energy(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::WMB) {
                    term *= ctx.get_energy_WMB(child.i, child.j);
                }
            }
            contributions += term * coeff;
        }
    }
    ctx.set_WIP(ij, contributions);
}

void compute_VPL_restricted_rules(PartFuncVPLContext &ctx, cand_pos_t i, cand_pos_t j, sparse_tree &tree, const RulesConfig &config) {
    const cand_pos_t ij = ctx.index_of(i, j);
    pf_t contributions = 0;
    for (RuleId rule : rules_for(NonTerminal::VPL)) {
        if (!is_rule_enabled(config, rule)) continue;
        const auto splits = enumerate_splits_vpl(rule, i, j, ctx, tree);
        for (const auto &split : splits) {
            record_rule_hit(rule);
            pf_t coeff = rule_score_vpl(rule, i, j, split, ctx);
            pf_t term = 1;
            const auto children = expand_vpl(rule, i, j, split);
            for (const auto &child : children) {
                if (child.nonterminal == NonTerminal::VP) {
                    term *= ctx.get_energy_VP(child.i, child.j);
                }
            }
            contributions += term * coeff;
        }
    }
    ctx.set_VPL(ij, contributions);
}

void compute_VPR_restricted_rules(PartFuncVPRContext &ctx, cand_pos_t i, cand_pos_t j, sparse_tree &tree, const RulesConfig &config) {
    const cand_pos_t ij = ctx.index_of(i, j);
    pf_t contributions = 0;
    for (RuleId rule : rules_for(NonTerminal::VPR)) {
        if (!is_rule_enabled(config, rule)) continue;
        const auto splits = enumerate_splits_vpr(rule, i, j, ctx, tree);
        for (const auto &split : splits) {
            record_rule_hit(rule);
            pf_t coeff = rule_score_vpr(rule, i, j, split, ctx);
            pf_t term = 1;
            const auto children = expand_vpr(rule, i, j, split);
            for (const auto &child : children) {
                if (child.nonterminal == NonTerminal::VP) {
                    term *= ctx.get_energy_VP(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::WIP) {
                    term *= ctx.get_energy_WIP(child.i, child.j);
                }
            }
            contributions += term * coeff;
        }
    }
    ctx.set_VPR(ij, contributions);
}

void compute_VP_restricted_rules(PartFuncVPContext &ctx, cand_pos_t i, cand_pos_t j, sparse_tree &tree, const RulesConfig &config) {
    const cand_pos_t ij = ctx.index_of(i, j);
    pf_t contributions = 0;
    for (RuleId rule : rules_for(NonTerminal::VP)) {
        if (!is_rule_enabled(config, rule)) continue;
        const auto splits = enumerate_splits_vp(rule, i, j, ctx, tree);
        for (const auto &split : splits) {
            record_rule_hit(rule);
            pf_t coeff = rule_score_vp(rule, i, j, split, ctx, tree);
            pf_t term = 1;
            const auto children = expand_vp(rule, i, j, split);
            for (const auto &child : children) {
                if (child.nonterminal == NonTerminal::VP) {
                    term *= ctx.get_energy_VP(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::WIP) {
                    term *= ctx.get_energy_WIP(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::VPR) {
                    term *= ctx.get_energy_VPR(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::VPL) {
                    term *= ctx.get_energy_VPL(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::WI) {
                    term *= ctx.get_energy_WI(child.i, child.j);
                }
            }
            contributions += term * coeff;
        }
    }
    ctx.set_VP(ij, contributions);
}

void compute_WMBP_restricted_rules(PartFuncWMBPContext &ctx, cand_pos_t i, cand_pos_t j, sparse_tree &tree, const RulesConfig &config) {
    const cand_pos_t ij = ctx.index_of(i, j);
    pf_t contributions = 0;
    const scfg::PartFuncModeConfig mode_config{ctx.expPB_penalty(), TURN};
    scfg::PartFuncRuleHelpers rules(tree, mode_config);
    rules.on_traceback_hook(i, j);
    rules.on_fixed_parse_hook(i, j);

    if (rules.pair_at(j) < 0 && is_rule_enabled(config, RuleId::WMBP_SPLIT_BE_WMBP_VP)) {
        const cand_pos_t b_ij = rules.border_b(i, j);
        rules.for_each_split(i, j, [&](cand_pos_t l) {
            int ext_case = ctx.compute_exterior_cases(l, j, tree);
            if (rules.allow_exterior_split(l, j, b_ij, ext_case)) {
                if (rules.has_valid_band_borders(i, l, j)) {
                    const cand_pos_t B_lj = rules.border_B(l, j);
                    const cand_pos_t Bp_lj = rules.border_Bp(l, j);
                    if (rules.parent_within_interval_and_turn(i, l, j)) {
                        pf_t m1 = ctx.get_BE(tree.tree[B_lj].pair, B_lj, tree.tree[Bp_lj].pair, Bp_lj, tree) *
                                  ctx.get_energy_WMBP(i, l - 1) * ctx.get_energy_VP(l, j);
                        m1 = rules.apply_double_pb_penalty(m1);
                        record_rule_hit(RuleId::WMBP_SPLIT_BE_WMBP_VP);
                        contributions += m1;
                    }
                }
            }
        });
    }

    if (rules.pair_at(j) < 0 && is_rule_enabled(config, RuleId::WMBP_SPLIT_BE_WMBW_VP)) {
        const cand_pos_t b_ij = rules.border_b(i, j);
        rules.for_each_split(i, j, [&](cand_pos_t l) {
            int ext_case = ctx.compute_exterior_cases(l, j, tree);
            if (rules.allow_exterior_split(l, j, b_ij, ext_case)) {
                if (rules.has_valid_band_borders(i, l, j)) {
                    const cand_pos_t B_lj = rules.border_B(l, j);
                    const cand_pos_t Bp_lj = rules.border_Bp(l, j);
                    if (rules.parent_within_interval_and_turn(i, l, j)) {
                        pf_t m2 = ctx.get_BE(tree.tree[B_lj].pair, B_lj, tree.tree[Bp_lj].pair, Bp_lj, tree) *
                                  ctx.get_energy_WMBW(i, l - 1) * ctx.get_energy_VP(l, j);
                        m2 = rules.apply_double_pb_penalty(m2);
                        record_rule_hit(RuleId::WMBP_SPLIT_BE_WMBW_VP);
                        contributions += m2;
                    }
                }
            }
        });
    }

    if (is_rule_enabled(config, RuleId::WMBP_DIRECT_VP)) {
        pf_t m3 = ctx.get_energy_VP(i, j) * ctx.expPB_penalty();
        record_rule_hit(RuleId::WMBP_DIRECT_VP);
        contributions += m3;
    }

    if (rules.pair_at(j) < 0 && rules.pair_at(i) >= 0 && is_rule_enabled(config, RuleId::WMBP_SPLIT_BE_WI_VP)) {
        rules.for_each_split(i, j, [&](cand_pos_t l) {
            if (rules.has_valid_inner_arc_split(i, l, j, ctx.n()) && rules.parent_within_interval_and_turn(i, l, j)) {
                const cand_pos_t bp_il = rules.border_bp(i, l);
                pf_t m4 = ctx.get_BE(i, rules.pair_at(i), bp_il, rules.pair_at(bp_il), tree) *
                          ctx.get_energy_WI(bp_il + 1, l - 1) * ctx.get_energy_VP(l, j);
                m4 = rules.apply_double_pb_penalty(m4);
                record_rule_hit(RuleId::WMBP_SPLIT_BE_WI_VP);
                contributions += m4;
            }
        });
    }

    ctx.set_WMBP(ij, contributions);
}

void compute_WMBW_restricted_rules(PartFuncWMBWContext &ctx, cand_pos_t i, cand_pos_t j, sparse_tree &tree, const RulesConfig &config) {
    const cand_pos_t ij = ctx.index_of(i, j);
    pf_t contributions = 0;
    for (RuleId rule : rules_for(NonTerminal::WMBW)) {
        if (!is_rule_enabled(config, rule)) continue;
        const auto splits = enumerate_splits_wmbw(rule, i, j, ctx, tree);
        for (const auto &split : splits) {
            record_rule_hit(rule);
            pf_t coeff = rule_score_wmbw(rule, i, j, split, ctx);
            pf_t term = 1;
            const auto children = expand_wmbw(rule, i, j, split);
            for (const auto &child : children) {
                if (child.nonterminal == NonTerminal::WMBP) {
                    term *= ctx.get_energy_WMBP(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::WI) {
                    term *= ctx.get_energy_WI(child.i, child.j);
                }
            }
            contributions += term * coeff;
        }
    }
    ctx.set_WMBW(ij, contributions);
}

void compute_WMB_restricted_rules(PartFuncWMBContext &ctx, cand_pos_t i, cand_pos_t j, sparse_tree &tree, const RulesConfig &config) {
    const cand_pos_t ij = ctx.index_of(i, j);
    pf_t contributions = 0;
    if (i == j) {
        if (is_rule_enabled(config, RuleId::WMB_EMPTY)) {
            record_rule_hit(RuleId::WMB_EMPTY);
            ctx.set_WMB(ij, 0);
        } else {
            ctx.set_WMB(ij, 0);
        }
        return;
    }

    if (tree.tree[j].pair >= 0 && j > tree.tree[j].pair && tree.tree[j].pair > i &&
        is_rule_enabled(config, RuleId::WMB_SPLIT_BE_WMBP_WI)) {
        cand_pos_t bp_j = tree.tree[j].pair;
        for (cand_pos_t l = (bp_j + 1); (l < j); ++l) {
            cand_pos_t Bp_lj = tree.Bp(l, j);
            if (Bp_lj >= 0 && Bp_lj < ctx.n()) {
                record_rule_hit(RuleId::WMB_SPLIT_BE_WMBP_WI);
                contributions += ctx.get_BE(bp_j, j, tree.tree[Bp_lj].pair, Bp_lj, tree) *
                                 ctx.get_energy_WMBP(i, l) *
                                 ctx.get_energy_WI(l + 1, Bp_lj - 1) * ctx.expPB_penalty();
            }
        }
    }

    if (is_rule_enabled(config, RuleId::WMB_DIRECT_WMBP)) {
        record_rule_hit(RuleId::WMB_DIRECT_WMBP);
        contributions += ctx.get_energy_WMBP(i, j);
    }
    ctx.set_WMB(ij, contributions);
}

void compute_BE_restricted_rules(PartFuncBEContext &ctx,
                                 cand_pos_t i,
                                 cand_pos_t j,
                                 cand_pos_t ip,
                                 cand_pos_t jp,
                                 sparse_tree &tree,
                                 const RulesConfig &config) {
    if (!(i >= 1 && i <= ip && ip < jp && jp <= j && j <= ctx.n() && tree.tree[i].pair > 0 && tree.tree[j].pair > 0 &&
          tree.tree[ip].pair > 0 && tree.tree[jp].pair > 0 && tree.tree[i].pair == j && tree.tree[j].pair == i &&
          tree.tree[ip].pair == jp && tree.tree[jp].pair == ip)) {
        if (i >= 1 && i <= ctx.n() && ip >= i && ip <= ctx.n()) {
            cand_pos_t iip = ctx.index_of(i, ip);
            ctx.set_BE(iip, 0);
        }
        return;
    }

    cand_pos_t iip = ctx.index_of(i, ip);
    pf_t contributions = 0;
    if (tree.tree[i].pair != j || tree.tree[ip].pair != jp) {
        ctx.set_BE(iip, 0);
        return;
    }

    if (i == ip && j == jp && i < j) {
        if (is_rule_enabled(config, RuleId::BE_BASE_SAMEPAIR)) {
            record_rule_hit(RuleId::BE_BASE_SAMEPAIR);
            ctx.set_BE(iip, ctx.scale(2));
        } else {
            ctx.set_BE(iip, 0);
        }
        return;
    }

    if (tree.tree[i + 1].pair == j - 1) {
        if (is_rule_enabled(config, RuleId::BE_STACK)) {
            pf_t be_estp = ctx.get_e_stP(i, j) * ctx.get_BE(i + 1, j - 1, ip, jp, tree);
            be_estp *= ctx.scale(2);
            record_rule_hit(RuleId::BE_STACK);
            contributions += be_estp;
        }
    }

    for (cand_pos_t l = i + 1; l <= ip; l++) {
        if (tree.tree[l].pair >= -1 && jp <= tree.tree[l].pair && tree.tree[l].pair < j) {
            cand_pos_t lp = tree.tree[l].pair;

            bool empty_region_il = scfg::is_empty_region(tree, i, l);
            bool empty_region_lpj = scfg::is_empty_region(tree, lp, j);
            bool weakly_closed_il = tree.weakly_closed(i + 1, l - 1);
            bool weakly_closed_lpj = tree.weakly_closed(lp + 1, j - 1);

            if (empty_region_il && empty_region_lpj && is_rule_enabled(config, RuleId::BE_INTERNAL_LOOP)) {
                pf_t eintp = ctx.get_e_intP(i, l, lp, j) * ctx.get_BE(l, lp, ip, jp, tree);
                cand_pos_t u1 = l - i - 1;
                cand_pos_t u2 = j - lp - 1;
                eintp *= ctx.scale(u1 + u2 + 2);
                record_rule_hit(RuleId::BE_INTERNAL_LOOP);
                contributions += eintp;
            }
            if (weakly_closed_il && weakly_closed_lpj && is_rule_enabled(config, RuleId::BE_WIP_WIP)) {
                pf_t m3 = ctx.get_energy_WIP(i + 1, l - 1) * ctx.get_BE(l, lp, ip, jp, tree) *
                          ctx.get_energy_WIP(lp + 1, j - 1) * ctx.expap_penalty() * ctx.expbp_penalty_sq();
                m3 *= ctx.scale(2);
                record_rule_hit(RuleId::BE_WIP_WIP);
                contributions += m3;
            }
            if (weakly_closed_il && empty_region_lpj && is_rule_enabled(config, RuleId::BE_WIP_BASEPAIR)) {
                pf_t m4 = ctx.get_energy_WIP(i + 1, l - 1) * ctx.get_BE(l, lp, ip, jp, tree) *
                          ctx.expcp_pen(j - lp - 1) * ctx.expap_penalty() * ctx.expbp_penalty_sq();
                m4 *= ctx.scale(2);
                record_rule_hit(RuleId::BE_WIP_BASEPAIR);
                contributions += m4;
            }
            if (empty_region_il && weakly_closed_lpj && is_rule_enabled(config, RuleId::BE_BASEPAIR_WIP)) {
                pf_t m5 = ctx.expcp_pen(l - i - 1) * ctx.get_BE(l, lp, ip, jp, tree) *
                          ctx.get_energy_WIP(lp + 1, j - 1) * ctx.expap_penalty() * ctx.expbp_penalty_sq();
                m5 *= ctx.scale(2);
                record_rule_hit(RuleId::BE_BASEPAIR_WIP);
                contributions += m5;
            }
        }
    }

    ctx.set_BE(iip, contributions);
}

} // namespace scfg
