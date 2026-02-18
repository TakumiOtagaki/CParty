#include "scfg/inside_fill/core.hh"

#include <cstdlib>
#include <cstring>
#include <iostream>

namespace scfg {

// 現状は inside_fill の実装に委譲する。今後このファイルで機械的DPコアを実装する。

void compute_W_restricted_core(PartFuncWContext &ctx, sparse_tree &tree, const RulesConfig &config) {
    compute_W_restricted_rules(ctx, tree, config);
}

void compute_V_restricted_core(PartFuncVContext &ctx, cand_pos_t i, cand_pos_t j, sparse_tree &tree, const RulesConfig &config) {
    (void)config;
    const cand_pos_t ij = ctx.index_of(i, j);

    const bool unpaired = (tree.tree[i].pair < -1 && tree.tree[j].pair < -1);
    const bool paired = (tree.tree[i].pair == j && tree.tree[j].pair == i);

    pf_t contributions = 0;
    pf_t hairpin = 0;
    pf_t internal = 0;
    pf_t vm = 0;

    if (paired || unpaired) {
        const bool canH = !(tree.up[j - 1] < (j - i - 1));
        if (canH) hairpin = ctx.hairpin_energy(i, j);
        internal = ctx.internal_energy(i, j, tree.up);
        vm = ctx.vm_energy(i, j, tree.up);
        contributions = hairpin + internal + vm;
        const char *pf_debug_env = std::getenv("CPARTY_PF_DEBUG");
        if (pf_debug_env && *pf_debug_env != '\0' && std::strcmp(pf_debug_env, "0") != 0 && i == 1 && j == tree.n) {
            std::cerr << "[PF_DEBUG] V_parts"
                      << " paired=" << paired
                      << " unpaired=" << unpaired
                      << " canH=" << canH
                      << " hairpin=" << hairpin
                      << " internal=" << internal
                      << " vm=" << vm
                      << " total=" << contributions
                      << std::endl;
        }
    }

    ctx.set_V(ij, contributions);
    const char *trace_env = std::getenv("CPARTY_PF_TRACE_V");
    if (trace_env && *trace_env != '\0' && std::strcmp(trace_env, "0") != 0) {
        const char *comma = std::strchr(trace_env, ',');
        if (comma) {
            const int ti = std::atoi(trace_env);
            const int tj = std::atoi(comma + 1);
            if (ti == i && tj == j) {
                std::cerr << "[PF_TRACE_V] i=" << i
                          << " j=" << j
                          << " paired=" << paired
                          << " unpaired=" << unpaired
                          << " hairpin=" << hairpin
                          << " internal=" << internal
                          << " vm=" << vm
                          << " total=" << contributions
                          << std::endl;
            }
        }
    }
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
