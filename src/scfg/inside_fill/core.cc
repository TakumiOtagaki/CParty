#include "scfg/inside_fill/core.hh"

#include "scfg/rules_core.hh"
#include "scfg/rules_debug.hh"

#include <cstdlib>
#include <cstring>
#include <iostream>

namespace scfg {

// 現状は inside_fill の実装に委譲する。今後このファイルで機械的DPコアを実装する。

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
        if (config.use_applicable) {
            const auto applicable = applicable_rules_w(1, j, ctx, tree);
            for (const auto &entry : applicable) {
                if (!is_rule_enabled(config, entry.rule)) continue;
                record_rule_hit(entry.rule);
                pf_t coeff = transition_weight_w(entry.rule, 1, j, entry.split, ctx);
                const auto children = expand_w(entry.rule, 1, j, entry.split);
                contributions += product_children(all, children) * coeff;
            }
            ctx.set_W(j, contributions);
            continue;
        }
        for (RuleId rule : rules_for(NonTerminal::W)) {
            if (!is_rule_enabled(config, rule)) continue;
            const auto splits = enumerate_splits_w(rule, 1, j, ctx, tree);
            for (const auto &split : splits) {
                record_rule_hit(rule);
                pf_t coeff = transition_weight_w(rule, 1, j, split, ctx);
                const auto children = expand_w(rule, 1, j, split);
                contributions += product_children(all, children) * coeff;
            }
        }
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
    if (config.use_applicable) {
        const auto applicable = applicable_rules_v(i, j, ctx, tree);
        for (const auto &entry : applicable) {
            if (!is_rule_enabled(config, entry.rule)) continue;
            record_rule_hit(entry.rule);
            contributions += transition_weight_v(entry.rule, i, j, entry.split, ctx, tree);
        }
        ctx.set_V(ij, contributions);
        return;
    }
    for (RuleId rule : rules_for(NonTerminal::V)) {
        if (!is_rule_enabled(config, rule)) continue;
        const auto splits = enumerate_splits_v(rule, i, j, ctx, tree);
        for (const auto &split : splits) {
            record_rule_hit(rule);
            contributions += transition_weight_v(rule, i, j, split, ctx, tree);
        }
    }
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
    if (config.use_applicable) {
        const auto applicable = applicable_rules_vm(i, j, ctx, up);
        for (const auto &entry : applicable) {
            if (entry.rule == RuleId::VM_SCALE2) continue;
            if (!is_rule_enabled(config, entry.rule)) continue;
            record_rule_hit(entry.rule);
            pf_t coeff = transition_weight_vm(entry.rule, i, j, entry.split, ctx, up);
            const auto children = expand_vm(entry.rule, i, j, entry.split);
            pf_t term = product_children(all, children);
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
            pf_t coeff = transition_weight_vm(rule, i, j, split, ctx, up);
            const auto children = expand_vm(rule, i, j, split);
            pf_t term = product_children(all, children);
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

void compute_WI_restricted_core(PartFuncWIContext &ctx,
                                PartFuncAllContext &all,
                                cand_pos_t i,
                                cand_pos_t j,
                                sparse_tree &tree,
                                const RulesConfig &config) {
    const cand_pos_t ij = ctx.index_of(i, j);
    pf_t contributions = 0;
    if (config.use_applicable) {
        const auto applicable = applicable_rules_wi(i, j, ctx, tree);
        for (const auto &entry : applicable) {
            if (!is_rule_enabled(config, entry.rule)) continue;
            record_rule_hit(entry.rule);
            pf_t coeff = transition_weight_wi(entry.rule, i, j, entry.split, ctx);
            const auto children = expand_wi(entry.rule, i, j, entry.split);
            contributions += product_children(all, children) * coeff;
        }
        ctx.set_WI(ij, contributions);
        return;
    }
    for (RuleId rule : rules_for(NonTerminal::WI)) {
        if (!is_rule_enabled(config, rule)) continue;
        const auto splits = enumerate_splits_wi(rule, i, j, ctx, tree);
        for (const auto &split : splits) {
            record_rule_hit(rule);
            pf_t coeff = transition_weight_wi(rule, i, j, split, ctx);
            const auto children = expand_wi(rule, i, j, split);
            contributions += product_children(all, children) * coeff;
        }
    }

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

    if (config.use_applicable) {
        const auto applicable = applicable_rules_wmv_wmp(i, j, ctx, tree);
        for (const auto &entry : applicable) {
            if (!is_rule_enabled(config, entry.rule)) continue;
            record_rule_hit(entry.rule);
            pf_t coeff = transition_weight_wmv_wmp(entry.rule, i, j, entry.split, ctx, tree);
            const auto children = expand_wmv_wmp(entry.rule, i, j, entry.split);
            const pf_t term = product_children(all, children);
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
            pf_t coeff = transition_weight_wmv_wmp(rule, i, j, split, ctx, tree);
            const auto children = expand_wmv_wmp(rule, i, j, split);
            WMv_contributions += product_children(all, children) * coeff;
        }
    }

    for (RuleId rule : rules_for(NonTerminal::WMp)) {
        if (!is_rule_enabled(config, rule)) continue;
        const auto splits = enumerate_splits_wmv_wmp(rule, i, j, ctx, tree);
        for (const auto &split : splits) {
            record_rule_hit(rule);
            pf_t coeff = transition_weight_wmv_wmp(rule, i, j, split, ctx, tree);
            const auto children = expand_wmv_wmp(rule, i, j, split);
            WMp_contributions += product_children(all, children) * coeff;
        }
    }

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

    if (config.use_applicable) {
        const auto applicable = applicable_rules_wm(i, j, ctx, tree);
        for (const auto &entry : applicable) {
            if (!is_rule_enabled(config, entry.rule)) continue;
            record_rule_hit(entry.rule);
            pf_t coeff = transition_weight_wm(entry.rule, i, j, entry.split, ctx);
            const auto children = expand_wm(entry.rule, i, j, entry.split);
            pf_t term = product_children(all, children);
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

void compute_WIP_restricted_core(PartFuncWIPContext &ctx,
                                 PartFuncAllContext &all,
                                 cand_pos_t i,
                                 cand_pos_t j,
                                 const StructureView &view,
                                 const RulesConfig &config) {
    const cand_pos_t ij = ctx.index_of(i, j);
    pf_t contributions = 0;
    if (config.use_applicable) {
        const auto applicable = applicable_rules_wip(i, j, ctx, view);
        for (const auto &entry : applicable) {
            if (!is_rule_enabled(config, entry.rule)) continue;
            record_rule_hit(entry.rule);
            pf_t coeff = transition_weight_wip(entry.rule, i, j, entry.split, ctx);
            const auto children = expand_wip(entry.rule, i, j, entry.split);
            contributions += product_children(all, children) * coeff;
        }
        ctx.set_WIP(ij, contributions);
        return;
    }
    for (RuleId rule : rules_for(NonTerminal::WIP)) {
        if (!is_rule_enabled(config, rule)) continue;
        const auto splits = enumerate_splits_wip(rule, i, j, ctx, view);
        for (const auto &split : splits) {
            record_rule_hit(rule);
            pf_t coeff = transition_weight_wip(rule, i, j, split, ctx);
            const auto children = expand_wip(rule, i, j, split);
            contributions += product_children(all, children) * coeff;
        }
    }
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
    if (config.use_applicable) {
        const auto applicable = applicable_rules_vpl(i, j, ctx, view);
        for (const auto &entry : applicable) {
            if (!is_rule_enabled(config, entry.rule)) continue;
            record_rule_hit(entry.rule);
            pf_t coeff = transition_weight_vpl(entry.rule, i, j, entry.split, ctx);
            const auto children = expand_vpl(entry.rule, i, j, entry.split);
            contributions += product_children(all, children) * coeff;
        }
        ctx.set_VPL(ij, contributions);
        return;
    }
    for (RuleId rule : rules_for(NonTerminal::VPL)) {
        if (!is_rule_enabled(config, rule)) continue;
        const auto splits = enumerate_splits_vpl(rule, i, j, ctx, view);
        for (const auto &split : splits) {
            record_rule_hit(rule);
            pf_t coeff = transition_weight_vpl(rule, i, j, split, ctx);
            const auto children = expand_vpl(rule, i, j, split);
            contributions += product_children(all, children) * coeff;
        }
    }
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
    if (config.use_applicable) {
        const auto applicable = applicable_rules_vpr(i, j, ctx, view);
        for (const auto &entry : applicable) {
            if (!is_rule_enabled(config, entry.rule)) continue;
            record_rule_hit(entry.rule);
            pf_t coeff = transition_weight_vpr(entry.rule, i, j, entry.split, ctx);
            const auto children = expand_vpr(entry.rule, i, j, entry.split);
            contributions += product_children(all, children) * coeff;
        }
        ctx.set_VPR(ij, contributions);
        return;
    }
    for (RuleId rule : rules_for(NonTerminal::VPR)) {
        if (!is_rule_enabled(config, rule)) continue;
        const auto splits = enumerate_splits_vpr(rule, i, j, ctx, view);
        for (const auto &split : splits) {
            record_rule_hit(rule);
            pf_t coeff = transition_weight_vpr(rule, i, j, split, ctx);
            const auto children = expand_vpr(rule, i, j, split);
            contributions += product_children(all, children) * coeff;
        }
    }
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
    if (config.use_applicable) {
        const auto applicable = applicable_rules_vp(i, j, ctx, view);
        for (const auto &entry : applicable) {
            if (!is_rule_enabled(config, entry.rule)) continue;
            record_rule_hit(entry.rule);
            pf_t coeff = transition_weight_vp(entry.rule, i, j, entry.split, ctx, tree);
            const auto children = expand_vp(entry.rule, i, j, entry.split);
            contributions += product_children(all, children) * coeff;
        }
        ctx.set_VP(ij, contributions);
        return;
    }
    for (RuleId rule : rules_for(NonTerminal::VP)) {
        if (!is_rule_enabled(config, rule)) continue;
        const auto splits = enumerate_splits_vp(rule, i, j, ctx, view);
        for (const auto &split : splits) {
            record_rule_hit(rule);
            pf_t coeff = transition_weight_vp(rule, i, j, split, ctx, tree);
            const auto children = expand_vp(rule, i, j, split);
            contributions += product_children(all, children) * coeff;
        }
    }
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
    if (config.use_applicable) {
        const auto applicable = applicable_rules_wmbw(i, j, ctx, view);
        for (const auto &entry : applicable) {
            if (!is_rule_enabled(config, entry.rule)) continue;
            record_rule_hit(entry.rule);
            pf_t coeff = transition_weight_wmbw(entry.rule, i, j, entry.split, ctx);
            const auto children = expand_wmbw(entry.rule, i, j, entry.split);
            contributions += product_children(all, children) * coeff;
        }
        ctx.set_WMBW(ij, contributions);
        return;
    }
    for (RuleId rule : rules_for(NonTerminal::WMBW)) {
        if (!is_rule_enabled(config, rule)) continue;
        const auto splits = enumerate_splits_wmbw(rule, i, j, ctx, view);
        for (const auto &split : splits) {
            record_rule_hit(rule);
            pf_t coeff = transition_weight_wmbw(rule, i, j, split, ctx);
            const auto children = expand_wmbw(rule, i, j, split);
            contributions += product_children(all, children) * coeff;
        }
    }
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
    if (config.use_applicable) {
        const auto applicable = applicable_rules_wmbp(i, j, ctx, view, tree);
        for (const auto &entry : applicable) {
            if (!is_rule_enabled(config, entry.rule)) continue;
            record_rule_hit(entry.rule);
            pf_t coeff = transition_weight_wmbp(entry.rule, i, j, entry.split, ctx, tree);
            const auto children = expand_wmbp(entry.rule, i, j, entry.split);
            contributions += product_children_be(all, children, entry.split.p, entry.split.q, tree) * coeff;
        }
        ctx.set_WMBP(ij, contributions);
        return;
    }
    for (RuleId rule : rules_for(NonTerminal::WMBP)) {
        if (!is_rule_enabled(config, rule)) continue;
        const auto splits = enumerate_splits_wmbp(rule, i, j, ctx, view, tree);
        for (const auto &split : splits) {
            record_rule_hit(rule);
            pf_t coeff = transition_weight_wmbp(rule, i, j, split, ctx, tree);
            const auto children = expand_wmbp(rule, i, j, split);
            contributions += product_children_be(all, children, split.p, split.q, tree) * coeff;
        }
    }
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
    if (config.use_applicable) {
        const auto applicable = applicable_rules_wmb(i, j, ctx, view);
        for (const auto &entry : applicable) {
            if (!is_rule_enabled(config, entry.rule)) continue;
            record_rule_hit(entry.rule);
            pf_t coeff = transition_weight_wmb(entry.rule, i, j, entry.split, ctx, tree);
            const auto children = expand_wmb(entry.rule, i, j, entry.split);
            contributions += product_children_be(all, children, entry.split.p, entry.split.q, tree) * coeff;
        }
        ctx.set_WMB(ij, contributions);
        return;
    }
    for (RuleId rule : rules_for(NonTerminal::WMB)) {
        if (!is_rule_enabled(config, rule)) continue;
        const auto splits = enumerate_splits_wmb(rule, i, j, ctx, view);
        for (const auto &split : splits) {
            record_rule_hit(rule);
            pf_t coeff = transition_weight_wmb(rule, i, j, split, ctx, tree);
            const auto children = expand_wmb(rule, i, j, split);
            contributions += product_children_be(all, children, split.p, split.q, tree) * coeff;
        }
    }
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
    if (config.use_applicable) {
        const auto applicable = applicable_rules_be(i, j, ip, jp, ctx, view);
        for (const auto &entry : applicable) {
            if (!is_rule_enabled(config, entry.rule)) continue;
            record_rule_hit(entry.rule);
            pf_t coeff = transition_weight_be(entry.rule, i, j, ip, jp, entry.split, ctx, tree);
            const auto children = expand_be(entry.rule, i, j, ip, jp, entry.split);
            contributions += product_children_be(all, children, ip, jp, tree) * coeff;
        }
        ctx.set_BE(iip, contributions);
        return;
    }
    for (RuleId rule : rules_for(NonTerminal::BE)) {
        if (!is_rule_enabled(config, rule)) continue;
        const auto splits = enumerate_splits_be(rule, i, j, ip, jp, ctx, view);
        for (const auto &split : splits) {
            record_rule_hit(rule);
            pf_t coeff = transition_weight_be(rule, i, j, ip, jp, split, ctx, tree);
            const auto children = expand_be(rule, i, j, ip, jp, split);
            contributions += product_children_be(all, children, ip, jp, tree) * coeff;
        }
    }
    ctx.set_BE(iip, contributions);
}

} // namespace scfg
