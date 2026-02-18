#include "scfg/inside_fill/core.hh"

#include "scfg/rules_core.hh"
#include "scfg/rules_debug.hh"

namespace scfg {

// 現状は inside_fill の実装に委譲する。今後このファイルで機械的DPコアを実装する。

void compute_W_restricted_core(PartFuncWContext &ctx, sparse_tree &tree, const RulesConfig &config) {
    compute_W_restricted_rules(ctx, tree, config);
}

void compute_V_restricted_core(PartFuncVContext &ctx, cand_pos_t i, cand_pos_t j, sparse_tree &tree, const RulesConfig &config) {
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

pf_t compute_VM_restricted_core(PartFuncVMContext &ctx, cand_pos_t i, cand_pos_t j, std::vector<int> &up, const RulesConfig &config) {
    return compute_VM_restricted_rules(ctx, i, j, up, config);
}

void compute_WI_restricted_core(PartFuncWIContext &ctx, cand_pos_t i, cand_pos_t j, sparse_tree &tree, const RulesConfig &config) {
    compute_WI_restricted_rules(ctx, i, j, tree, config);
}

void compute_WMv_WMp_restricted_core(PartFuncWMvWMpContext &ctx,
                                     cand_pos_t i,
                                     cand_pos_t j,
                                     std::vector<Node> &tree,
                                     const RulesConfig &config) {
    compute_WMv_WMp_restricted_rules(ctx, i, j, tree, config);
}

void compute_WM_restricted_core(PartFuncWMContext &ctx, cand_pos_t i, cand_pos_t j, sparse_tree &tree, const RulesConfig &config) {
    compute_WM_restricted_rules(ctx, i, j, tree, config);
}

void compute_WIP_restricted_core(PartFuncWIPContext &ctx,
                                 cand_pos_t i,
                                 cand_pos_t j,
                                 const StructureView &view,
                                 const RulesConfig &config) {
    compute_WIP_restricted_rules(ctx, i, j, view, config);
}

void compute_VPL_restricted_core(PartFuncVPLContext &ctx,
                                 cand_pos_t i,
                                 cand_pos_t j,
                                 const StructureView &view,
                                 const RulesConfig &config) {
    compute_VPL_restricted_rules(ctx, i, j, view, config);
}

void compute_VPR_restricted_core(PartFuncVPRContext &ctx,
                                 cand_pos_t i,
                                 cand_pos_t j,
                                 const StructureView &view,
                                 const RulesConfig &config) {
    compute_VPR_restricted_rules(ctx, i, j, view, config);
}

void compute_VP_restricted_core(PartFuncVPContext &ctx,
                                cand_pos_t i,
                                cand_pos_t j,
                                const StructureView &view,
                                sparse_tree &tree,
                                const RulesConfig &config) {
    compute_VP_restricted_rules(ctx, i, j, view, tree, config);
}

void compute_WMBW_restricted_core(PartFuncWMBWContext &ctx,
                                  cand_pos_t i,
                                  cand_pos_t j,
                                  const StructureView &view,
                                  const RulesConfig &config) {
    compute_WMBW_restricted_rules(ctx, i, j, view, config);
}

void compute_WMBP_restricted_core(PartFuncWMBPContext &ctx,
                                  cand_pos_t i,
                                  cand_pos_t j,
                                  const StructureView &view,
                                  sparse_tree &tree,
                                  const RulesConfig &config) {
    compute_WMBP_restricted_rules(ctx, i, j, view, tree, config);
}

void compute_WMB_restricted_core(PartFuncWMBContext &ctx,
                                 cand_pos_t i,
                                 cand_pos_t j,
                                 const StructureView &view,
                                 sparse_tree &tree,
                                 const RulesConfig &config) {
    compute_WMB_restricted_rules(ctx, i, j, view, tree, config);
}

void compute_BE_restricted_core(PartFuncBEContext &ctx,
                                cand_pos_t i,
                                cand_pos_t j,
                                cand_pos_t ip,
                                cand_pos_t jp,
                                const StructureView &view,
                                sparse_tree &tree,
                                const RulesConfig &config) {
    compute_BE_restricted_rules(ctx, i, j, ip, jp, view, tree, config);
}

} // namespace scfg
