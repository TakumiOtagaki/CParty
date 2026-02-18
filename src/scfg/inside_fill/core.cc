#include "scfg/inside_fill/core.hh"

#include "scfg/rules_core.hh"
#include "scfg/rules_debug.hh"

#include <cstdlib>
#include <cstring>
#include <iostream>

namespace scfg {

// 現状は inside_fill の実装に委譲する。今後このファイルで機械的DPコアを実装する。

template <typename ApplicableFn, typename EnumerateFn, typename Visitor>
static inline void for_each_entry(const RulesConfig &config,
                                  NonTerminal nonterminal,
                                  ApplicableFn applicable_fn,
                                  EnumerateFn enumerate_fn,
                                  Visitor visit) {
    if (config.use_applicable) {
        const auto applicable = applicable_fn();
        for (const auto &entry : applicable) {
            if (!is_rule_enabled(config, entry.rule)) continue;
            visit(entry.rule, entry.split);
        }
        return;
    }
    for (RuleId rule : rules_for(nonterminal)) {
        if (!is_rule_enabled(config, rule)) continue;
        const auto splits = enumerate_fn(rule);
        for (const auto &split : splits) {
            visit(rule, split);
        }
    }
}

template <typename RuleListFn, typename ApplicableFn, typename EnumerateFn, typename Visitor>
static inline void for_each_entry_list(const RulesConfig &config,
                                       RuleListFn rules_fn,
                                       ApplicableFn applicable_fn,
                                       EnumerateFn enumerate_fn,
                                       Visitor visit) {
    if (config.use_applicable) {
        const auto applicable = applicable_fn();
        for (const auto &entry : applicable) {
            if (!is_rule_enabled(config, entry.rule)) continue;
            visit(entry.rule, entry.split);
        }
        return;
    }
    const auto &rules = rules_fn();
    for (RuleId rule : rules) {
        if (!is_rule_enabled(config, rule)) continue;
        const auto splits = enumerate_fn(rule);
        for (const auto &split : splits) {
            visit(rule, split);
        }
    }
}

static const std::vector<RuleId> &wmv_wmp_rules() {
    static const std::vector<RuleId> rules = [] {
        std::vector<RuleId> out = rules_for(NonTerminal::WMv);
        const auto &wmprules = rules_for(NonTerminal::WMp);
        out.insert(out.end(), wmprules.begin(), wmprules.end());
        return out;
    }();
    return rules;
}

static inline pf_t product_children(PartFuncAllContext &all, const std::vector<RuleChild> &children) {
    pf_t term = 1;
    for (const auto &child : children) {
        term *= all.get_inside(child.nonterminal, child.i, child.j);
    }
    return term;
}

static inline pf_t product_children_be(PartFuncAllContext &all,
                                       const std::vector<RuleChild> &children,
                                       cand_pos_t ip,
                                       cand_pos_t jp,
                                       sparse_tree &tree) {
    pf_t term = 1;
    for (const auto &child : children) {
        if (child.nonterminal == NonTerminal::BE) {
            term *= all.get_BE(child.i, child.j, ip, jp, tree);
        } else {
            term *= all.get_inside(child.nonterminal, child.i, child.j);
        }
    }
    return term;
}

void compute_W_restricted_core(PartFuncWContext &ctx,
                               PartFuncAllContext &all,
                               sparse_tree &tree,
                               const RulesConfig &config) {
    const cand_pos_t n = ctx.n();
    const cand_pos_t turn = ctx.turn();

    for (cand_pos_t j = turn + 1; j <= n; j++) {
        pf_t contributions = 0;
        for_each_entry(
            config,
            NonTerminal::W,
            [&]() { return applicable_rules_w(1, j, ctx, tree); },
            [&](RuleId rule) { return enumerate_splits_w(rule, 1, j, ctx, tree); },
            [&](RuleId rule, const RuleSplit &split) {
                record_rule_hit(rule);
                pf_t coeff = transition_weight_w(rule, 1, j, split, ctx);
                const auto children = expand_w(rule, 1, j, split);
                contributions += product_children(all, children) * coeff;
            });
        ctx.set_W(j, contributions);
    }
}

void compute_V_restricted_core(PartFuncVContext &ctx,
                               PartFuncAllContext &all,
                               cand_pos_t i,
                               cand_pos_t j,
                               sparse_tree &tree,
                               const RulesConfig &config) {
    (void)all;
    const cand_pos_t ij = ctx.index_of(i, j);
    pf_t contributions = 0;
    for_each_entry(
        config,
        NonTerminal::V,
        [&]() { return applicable_rules_v(i, j, ctx, tree); },
        [&](RuleId rule) { return enumerate_splits_v(rule, i, j, ctx, tree); },
        [&](RuleId rule, const RuleSplit &split) {
            record_rule_hit(rule);
            contributions += transition_weight_v(rule, i, j, split, ctx, tree);
        });
    ctx.set_V(ij, contributions);
}

pf_t compute_VM_restricted_core(PartFuncVMContext &ctx,
                                PartFuncAllContext &all,
                                cand_pos_t i,
                                cand_pos_t j,
                                std::vector<int> &up,
                                const RulesConfig &config) {
    pf_t contributions = 0;
    const cand_pos_t ij = ctx.index_of(i, j);
    const bool apply_scale2 = is_rule_enabled(config, RuleId::VM_SCALE2);
    for_each_entry(
        config,
        NonTerminal::VM,
        [&]() { return applicable_rules_vm(i, j, ctx, up); },
        [&](RuleId rule) { return enumerate_splits_vm(rule, i, j, ctx, up); },
        [&](RuleId rule, const RuleSplit &split) {
            if (rule == RuleId::VM_SCALE2) return;
            record_rule_hit(rule);
            pf_t coeff = transition_weight_vm(rule, i, j, split, ctx, up);
            const auto children = expand_vm(rule, i, j, split);
            pf_t term = product_children(all, children);
            contributions += term * coeff;
        });
    if (apply_scale2) {
        record_rule_hit(RuleId::VM_SCALE2);
        contributions *= ctx.scale2();
    }
    ctx.set_VM(ij, contributions);
    return contributions;
}

void compute_WI_restricted_core(PartFuncWIContext &ctx,
                                PartFuncAllContext &all,
                                cand_pos_t i,
                                cand_pos_t j,
                                sparse_tree &tree,
                                const RulesConfig &config) {
    const cand_pos_t ij = ctx.index_of(i, j);
    pf_t contributions = 0;
    for_each_entry(
        config,
        NonTerminal::WI,
        [&]() { return applicable_rules_wi(i, j, ctx, tree); },
        [&](RuleId rule) { return enumerate_splits_wi(rule, i, j, ctx, tree); },
        [&](RuleId rule, const RuleSplit &split) {
            record_rule_hit(rule);
            pf_t coeff = transition_weight_wi(rule, i, j, split, ctx);
            const auto children = expand_wi(rule, i, j, split);
            contributions += product_children(all, children) * coeff;
        });
    ctx.set_WI(ij, contributions);
}

void compute_WMv_WMp_restricted_core(PartFuncWMvWMpContext &ctx,
                                     PartFuncAllContext &all,
                                     cand_pos_t i,
                                     cand_pos_t j,
                                     std::vector<Node> &tree,
                                     const RulesConfig &config) {
    const cand_pos_t ij = ctx.index_of(i, j);
    pf_t WMv_contributions = 0;
    pf_t WMp_contributions = 0;

    auto accumulate = [&](RuleId rule, const RuleSplit &split) {
        record_rule_hit(rule);
        pf_t coeff = transition_weight_wmv_wmp(rule, i, j, split, ctx, tree);
        const auto children = expand_wmv_wmp(rule, i, j, split);
        const pf_t term = product_children(all, children);
        if (rule == RuleId::WMv_STEM_V || rule == RuleId::WMv_EXTEND_UNPAIRED) {
            WMv_contributions += term * coeff;
        } else {
            WMp_contributions += term * coeff;
        }
    };

    for_each_entry_list(
        config,
        []() -> const std::vector<RuleId> & { return wmv_wmp_rules(); },
        [&]() { return applicable_rules_wmv_wmp(i, j, ctx, tree); },
        [&](RuleId rule) { return enumerate_splits_wmv_wmp(rule, i, j, ctx, tree); },
        [&](RuleId rule, const RuleSplit &split) { accumulate(rule, split); });

    ctx.set_WMv_WMp(ij, WMv_contributions, WMp_contributions);
}

void compute_WM_restricted_core(PartFuncWMContext &ctx,
                                PartFuncAllContext &all,
                                cand_pos_t i,
                                cand_pos_t j,
                                sparse_tree &tree,
                                const RulesConfig &config) {
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

    for_each_entry(
        config,
        NonTerminal::WM,
        [&]() { return applicable_rules_wm(i, j, ctx, tree); },
        [&](RuleId rule) { return enumerate_splits_wm(rule, i, j, ctx, tree); },
        [&](RuleId rule, const RuleSplit &split) {
            record_rule_hit(rule);
            pf_t coeff = transition_weight_wm(rule, i, j, split, ctx);
            const auto children = expand_wm(rule, i, j, split);
            pf_t term = product_children(all, children);
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
        });
    ctx.set_WM(ij, contributions);
    if (trace) {
        std::cerr << "[PF_TRACE_WM_RULES] i=" << i
                  << " j=" << j
                  << " total=" << contributions
                  << std::endl;
    }
}

void compute_WIP_restricted_core(PartFuncWIPContext &ctx,
                                 PartFuncAllContext &all,
                                 cand_pos_t i,
                                 cand_pos_t j,
                                 const StructureView &view,
                                 const RulesConfig &config) {
    const cand_pos_t ij = ctx.index_of(i, j);
    pf_t contributions = 0;
    for_each_entry(
        config,
        NonTerminal::WIP,
        [&]() { return applicable_rules_wip(i, j, ctx, view); },
        [&](RuleId rule) { return enumerate_splits_wip(rule, i, j, ctx, view); },
        [&](RuleId rule, const RuleSplit &split) {
            record_rule_hit(rule);
            pf_t coeff = transition_weight_wip(rule, i, j, split, ctx);
            const auto children = expand_wip(rule, i, j, split);
            contributions += product_children(all, children) * coeff;
        });
    ctx.set_WIP(ij, contributions);
}

void compute_VPL_restricted_core(PartFuncVPLContext &ctx,
                                 PartFuncAllContext &all,
                                 cand_pos_t i,
                                 cand_pos_t j,
                                 const StructureView &view,
                                 const RulesConfig &config) {
    const cand_pos_t ij = ctx.index_of(i, j);
    pf_t contributions = 0;
    for_each_entry(
        config,
        NonTerminal::VPL,
        [&]() { return applicable_rules_vpl(i, j, ctx, view); },
        [&](RuleId rule) { return enumerate_splits_vpl(rule, i, j, ctx, view); },
        [&](RuleId rule, const RuleSplit &split) {
            record_rule_hit(rule);
            pf_t coeff = transition_weight_vpl(rule, i, j, split, ctx);
            const auto children = expand_vpl(rule, i, j, split);
            contributions += product_children(all, children) * coeff;
        });
    ctx.set_VPL(ij, contributions);
}

void compute_VPR_restricted_core(PartFuncVPRContext &ctx,
                                 PartFuncAllContext &all,
                                 cand_pos_t i,
                                 cand_pos_t j,
                                 const StructureView &view,
                                 const RulesConfig &config) {
    const cand_pos_t ij = ctx.index_of(i, j);
    pf_t contributions = 0;
    for_each_entry(
        config,
        NonTerminal::VPR,
        [&]() { return applicable_rules_vpr(i, j, ctx, view); },
        [&](RuleId rule) { return enumerate_splits_vpr(rule, i, j, ctx, view); },
        [&](RuleId rule, const RuleSplit &split) {
            record_rule_hit(rule);
            pf_t coeff = transition_weight_vpr(rule, i, j, split, ctx);
            const auto children = expand_vpr(rule, i, j, split);
            contributions += product_children(all, children) * coeff;
        });
    ctx.set_VPR(ij, contributions);
}

void compute_VP_restricted_core(PartFuncVPContext &ctx,
                                PartFuncAllContext &all,
                                cand_pos_t i,
                                cand_pos_t j,
                                const StructureView &view,
                                sparse_tree &tree,
                                const RulesConfig &config) {
    const cand_pos_t ij = ctx.index_of(i, j);
    pf_t contributions = 0;
    for_each_entry(
        config,
        NonTerminal::VP,
        [&]() { return applicable_rules_vp(i, j, ctx, view); },
        [&](RuleId rule) { return enumerate_splits_vp(rule, i, j, ctx, view); },
        [&](RuleId rule, const RuleSplit &split) {
            record_rule_hit(rule);
            pf_t coeff = transition_weight_vp(rule, i, j, split, ctx, tree);
            const auto children = expand_vp(rule, i, j, split);
            contributions += product_children(all, children) * coeff;
        });
    ctx.set_VP(ij, contributions);
}

void compute_WMBW_restricted_core(PartFuncWMBWContext &ctx,
                                  PartFuncAllContext &all,
                                  cand_pos_t i,
                                  cand_pos_t j,
                                  const StructureView &view,
                                  const RulesConfig &config) {
    const cand_pos_t ij = ctx.index_of(i, j);
    pf_t contributions = 0;
    for_each_entry(
        config,
        NonTerminal::WMBW,
        [&]() { return applicable_rules_wmbw(i, j, ctx, view); },
        [&](RuleId rule) { return enumerate_splits_wmbw(rule, i, j, ctx, view); },
        [&](RuleId rule, const RuleSplit &split) {
            record_rule_hit(rule);
            pf_t coeff = transition_weight_wmbw(rule, i, j, split, ctx);
            const auto children = expand_wmbw(rule, i, j, split);
            contributions += product_children(all, children) * coeff;
        });
    ctx.set_WMBW(ij, contributions);
}

void compute_WMBP_restricted_core(PartFuncWMBPContext &ctx,
                                  PartFuncAllContext &all,
                                  cand_pos_t i,
                                  cand_pos_t j,
                                  const StructureView &view,
                                  sparse_tree &tree,
                                  const RulesConfig &config) {
    const cand_pos_t ij = ctx.index_of(i, j);
    pf_t contributions = 0;
    for_each_entry(
        config,
        NonTerminal::WMBP,
        [&]() { return applicable_rules_wmbp(i, j, ctx, view, tree); },
        [&](RuleId rule) { return enumerate_splits_wmbp(rule, i, j, ctx, view, tree); },
        [&](RuleId rule, const RuleSplit &split) {
            record_rule_hit(rule);
            pf_t coeff = transition_weight_wmbp(rule, i, j, split, ctx, tree);
            const auto children = expand_wmbp(rule, i, j, split);
            contributions += product_children_be(all, children, split.p, split.q, tree) * coeff;
        });
    ctx.set_WMBP(ij, contributions);
}

void compute_WMB_restricted_core(PartFuncWMBContext &ctx,
                                 PartFuncAllContext &all,
                                 cand_pos_t i,
                                 cand_pos_t j,
                                 const StructureView &view,
                                 sparse_tree &tree,
                                 const RulesConfig &config) {
    const cand_pos_t ij = ctx.index_of(i, j);
    pf_t contributions = 0;
    for_each_entry(
        config,
        NonTerminal::WMB,
        [&]() { return applicable_rules_wmb(i, j, ctx, view); },
        [&](RuleId rule) { return enumerate_splits_wmb(rule, i, j, ctx, view); },
        [&](RuleId rule, const RuleSplit &split) {
            record_rule_hit(rule);
            pf_t coeff = transition_weight_wmb(rule, i, j, split, ctx, tree);
            const auto children = expand_wmb(rule, i, j, split);
            contributions += product_children_be(all, children, split.p, split.q, tree) * coeff;
        });
    ctx.set_WMB(ij, contributions);
}

void compute_BE_restricted_core(PartFuncBEContext &ctx,
                                PartFuncAllContext &all,
                                cand_pos_t i,
                                cand_pos_t j,
                                cand_pos_t ip,
                                cand_pos_t jp,
                                const StructureView &view,
                                sparse_tree &tree,
                                const RulesConfig &config) {
    const cand_pos_t iip = ctx.index_of(i, ip);
    pf_t contributions = 0;
    for_each_entry(
        config,
        NonTerminal::BE,
        [&]() { return applicable_rules_be(i, j, ip, jp, ctx, view); },
        [&](RuleId rule) { return enumerate_splits_be(rule, i, j, ip, jp, ctx, view); },
        [&](RuleId rule, const RuleSplit &split) {
            record_rule_hit(rule);
            pf_t coeff = transition_weight_be(rule, i, j, ip, jp, split, ctx, tree);
            const auto children = expand_be(rule, i, j, ip, jp, split);
            contributions += product_children_be(all, children, ip, jp, tree) * coeff;
        });
    ctx.set_BE(iip, contributions);
}

} // namespace scfg
