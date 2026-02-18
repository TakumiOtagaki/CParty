#include "scfg/inside_fill.hh"

#include "scfg/constraint_oracle.hh"
#include "scfg/rules_debug.hh"
#include "scfg/legacy_adapter.hh"
#include "scfg/rules_core.hh"
#include "scfg/rules_part_func.hh"
#include "scfg/structure_view.hh"

#include <ViennaRNA/params/constants.h>

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <iostream>

namespace scfg {

void compute_V_restricted_rules(PartFuncVContext &ctx, cand_pos_t i, cand_pos_t j, sparse_tree &tree, const RulesConfig &config) {
    const cand_pos_t ij = ctx.index_of(i, j);
    pf_t contributions = 0;
    if (config.use_applicable) {
        const auto applicable = applicable_rules_v(i, j, ctx, tree);
        for (const auto &entry : applicable) {
            if (!is_rule_enabled(config, entry.rule)) continue;
            record_rule_hit(entry.rule);
            contributions += rule_score_v(entry.rule, i, j, entry.split, ctx, tree);
        }
        ctx.set_V(ij, contributions);
        return;
    }
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
    if (config.use_applicable) {
        const auto applicable = applicable_rules_vm(i, j, ctx, up);
        for (const auto &entry : applicable) {
            if (entry.rule == RuleId::VM_SCALE2) continue;
            if (!is_rule_enabled(config, entry.rule)) continue;
            record_rule_hit(entry.rule);
            pf_t coeff = rule_score_vm(entry.rule, i, j, entry.split, ctx, up);
            pf_t term = 1;
            const auto children = expand_vm(entry.rule, i, j, entry.split);
            for (const auto &child : children) {
                if (child.nonterminal == NonTerminal::WM) {
                    term *= ctx.get_energy_WM(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::WMv) {
                    term *= ctx.get_energy_WMv(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::WMp) {
                    term *= ctx.get_energy_WMp(child.i, child.j);
                }
            }
            if (entry.rule == RuleId::VM_SPLIT_WMp_BASE) {
                term *= ctx.expMLbase(entry.split.k - i - 1);
            }
            contributions += term * coeff;
        }
        if (apply_scale2) {
            record_rule_hit(RuleId::VM_SCALE2);
            contributions *= ctx.scale2();
        }
        ctx.set_VM(ij, contributions);
        return contributions;
    }
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
        if (config.use_applicable) {
            const auto applicable = applicable_rules_w(1, j, ctx, tree);
            for (const auto &entry : applicable) {
                if (!is_rule_enabled(config, entry.rule)) continue;
                record_rule_hit(entry.rule);
                pf_t coeff = rule_score_w(entry.rule, 1, j, entry.split, ctx);
                pf_t term = 1;
                const auto children = expand_w(entry.rule, 1, j, entry.split);
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
            ctx.set_W(j, contributions);
            continue;
        }
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
    if (config.use_applicable) {
        const auto applicable = applicable_rules_wi(i, j, ctx, tree);
        for (const auto &entry : applicable) {
            if (!is_rule_enabled(config, entry.rule)) continue;
            record_rule_hit(entry.rule);
            pf_t coeff = rule_score_wi(entry.rule, i, j, entry.split, ctx);
            pf_t term = 1;
            const auto children = expand_wi(entry.rule, i, j, entry.split);
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
        ctx.set_WI(ij, contributions);
        return;
    }
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

    if (config.use_applicable) {
        const auto applicable = applicable_rules_wmv_wmp(i, j, ctx, tree);
        for (const auto &entry : applicable) {
            if (!is_rule_enabled(config, entry.rule)) continue;
            record_rule_hit(entry.rule);
            pf_t coeff = rule_score_wmv_wmp(entry.rule, i, j, entry.split, ctx, tree);
            pf_t term = 1;
            const auto children = expand_wmv_wmp(entry.rule, i, j, entry.split);
            for (const auto &child : children) {
                if (child.nonterminal == NonTerminal::V) {
                    term *= ctx.get_energy(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::WMv) {
                    term *= ctx.get_energy_WMv(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::WMB) {
                    term *= ctx.get_energy_WMB(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::WMp) {
                    term *= ctx.get_energy_WMp(child.i, child.j);
                }
            }
            if (entry.rule == RuleId::WMv_STEM_V || entry.rule == RuleId::WMv_EXTEND_UNPAIRED) {
                WMv_contributions += term * coeff;
            } else {
                WMp_contributions += term * coeff;
            }
        }
        ctx.set_WMv_WMp(ij, WMv_contributions, WMp_contributions);
        return;
    }
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
    const char *trace_env = std::getenv("CPARTY_PF_TRACE_WM");
    bool trace = false;
    if (trace_env && *trace_env != '\0' && std::strcmp(trace_env, "0") != 0) {
        const char *comma = std::strchr(trace_env, ',');
        if (comma) {
            const int ti = std::atoi(trace_env);
            const int tj = std::atoi(comma + 1);
            if (ti == i && tj == j) {
                trace = true;
            }
        }
    }

    if (config.use_applicable) {
        const auto applicable = applicable_rules_wm(i, j, ctx, tree);
        for (const auto &entry : applicable) {
            if (!is_rule_enabled(config, entry.rule)) continue;
            record_rule_hit(entry.rule);
            pf_t coeff = rule_score_wm(entry.rule, i, j, entry.split, ctx);
            const auto children = expand_wm(entry.rule, i, j, entry.split);
            pf_t term = 1;
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
            if (trace) {
                std::cerr << "[PF_TRACE_WM_RULES] rule=" << rule_id_name(entry.rule)
                          << " i=" << i
                          << " j=" << j
                          << " k=" << entry.split.k
                          << " term=" << term
                          << " coeff=" << coeff
                          << std::endl;
            }
        }
        ctx.set_WM(ij, contributions);
        if (trace) {
            std::cerr << "[PF_TRACE_WM_RULES] i=" << i
                      << " j=" << j
                      << " total=" << contributions
                      << std::endl;
        }
        return;
    }

    for (RuleId rule : rules_for(NonTerminal::WM)) {
        if (!is_rule_enabled(config, rule)) continue;
        const auto splits = enumerate_splits_wm(rule, i, j, ctx, tree);
        for (const auto &split : splits) {
            record_rule_hit(rule);
            pf_t coeff = rule_score_wm(rule, i, j, split, ctx);
            const auto children = expand_wm(rule, i, j, split);
            pf_t term = 1;
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
            if (trace) {
                std::cerr << "[PF_TRACE_WM_RULES] rule=" << rule_id_name(rule)
                          << " i=" << i
                          << " j=" << j
                          << " k=" << split.k
                          << " term=" << term
                          << " coeff=" << coeff
                          << std::endl;
            }
        }
    }
    ctx.set_WM(ij, contributions);
    if (trace) {
        std::cerr << "[PF_TRACE_WM_RULES] i=" << i
                  << " j=" << j
                  << " total=" << contributions
                  << std::endl;
    }
}


} // namespace scfg
