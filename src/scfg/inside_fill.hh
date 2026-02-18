#ifndef SCFG_INSIDE_FILL_HH_
#define SCFG_INSIDE_FILL_HH_

#include "scfg/rules_config.hh"
#include "scfg/rules_part_func.hh"

namespace scfg {

class StructureView;

void compute_W_restricted_rules(PartFuncWContext &ctx, sparse_tree &tree, const RulesConfig &config);
void compute_V_restricted_rules(PartFuncVContext &ctx, cand_pos_t i, cand_pos_t j, sparse_tree &tree, const RulesConfig &config);
pf_t compute_VM_restricted_rules(PartFuncVMContext &ctx, cand_pos_t i, cand_pos_t j, std::vector<int> &up, const RulesConfig &config);
void compute_WI_restricted_rules(PartFuncWIContext &ctx, cand_pos_t i, cand_pos_t j, sparse_tree &tree, const RulesConfig &config);
void compute_WMv_WMp_restricted_rules(PartFuncWMvWMpContext &ctx, cand_pos_t i, cand_pos_t j, std::vector<Node> &tree, const RulesConfig &config);
void compute_WM_restricted_rules(PartFuncWMContext &ctx, cand_pos_t i, cand_pos_t j, sparse_tree &tree, const RulesConfig &config);
void compute_WIP_restricted_rules(PartFuncWIPContext &ctx, cand_pos_t i, cand_pos_t j, sparse_tree &tree, const RulesConfig &config);
void compute_WIP_restricted_rules(PartFuncWIPContext &ctx,
                                  cand_pos_t i,
                                  cand_pos_t j,
                                  const StructureView &view,
                                  const RulesConfig &config);
void compute_VPL_restricted_rules(PartFuncVPLContext &ctx, cand_pos_t i, cand_pos_t j, sparse_tree &tree, const RulesConfig &config);
void compute_VPL_restricted_rules(PartFuncVPLContext &ctx,
                                  cand_pos_t i,
                                  cand_pos_t j,
                                  const StructureView &view,
                                  const RulesConfig &config);
void compute_VPR_restricted_rules(PartFuncVPRContext &ctx, cand_pos_t i, cand_pos_t j, sparse_tree &tree, const RulesConfig &config);
void compute_VPR_restricted_rules(PartFuncVPRContext &ctx,
                                  cand_pos_t i,
                                  cand_pos_t j,
                                  const StructureView &view,
                                  const RulesConfig &config);
void compute_VP_restricted_rules(PartFuncVPContext &ctx, cand_pos_t i, cand_pos_t j, sparse_tree &tree, const RulesConfig &config);
void compute_VP_restricted_rules(PartFuncVPContext &ctx,
                                 cand_pos_t i,
                                 cand_pos_t j,
                                 const StructureView &view,
                                 sparse_tree &tree,
                                 const RulesConfig &config);
void compute_WMBP_restricted_rules(PartFuncWMBPContext &ctx, cand_pos_t i, cand_pos_t j, sparse_tree &tree, const RulesConfig &config);
void compute_WMBP_restricted_rules(PartFuncWMBPContext &ctx,
                                   cand_pos_t i,
                                   cand_pos_t j,
                                   const StructureView &view,
                                   sparse_tree &tree,
                                   const RulesConfig &config);
void compute_WMBW_restricted_rules(PartFuncWMBWContext &ctx, cand_pos_t i, cand_pos_t j, sparse_tree &tree, const RulesConfig &config);
void compute_WMBW_restricted_rules(PartFuncWMBWContext &ctx,
                                   cand_pos_t i,
                                   cand_pos_t j,
                                   const StructureView &view,
                                   const RulesConfig &config);
void compute_WMB_restricted_rules(PartFuncWMBContext &ctx, cand_pos_t i, cand_pos_t j, sparse_tree &tree, const RulesConfig &config);
void compute_WMB_restricted_rules(PartFuncWMBContext &ctx,
                                  cand_pos_t i,
                                  cand_pos_t j,
                                  const StructureView &view,
                                  sparse_tree &tree,
                                  const RulesConfig &config);
void compute_BE_restricted_rules(PartFuncBEContext &ctx,
                                 cand_pos_t i,
                                 cand_pos_t j,
                                 cand_pos_t ip,
                                 cand_pos_t jp,
                                 sparse_tree &tree,
                                 const RulesConfig &config);
void compute_BE_restricted_rules(PartFuncBEContext &ctx,
                                 cand_pos_t i,
                                 cand_pos_t j,
                                 cand_pos_t ip,
                                 cand_pos_t jp,
                                 const StructureView &view,
                                 sparse_tree &tree,
                                 const RulesConfig &config);

} // namespace scfg

#endif
