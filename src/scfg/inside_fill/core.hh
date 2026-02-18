#ifndef SCFG_INSIDE_FILL_CORE_HH_
#define SCFG_INSIDE_FILL_CORE_HH_

#include "scfg/inside_fill.hh"

namespace scfg {

void compute_W_restricted_core(PartFuncWContext &ctx,
                               PartFuncAllContext &all,
                               sparse_tree &tree,
                               const RulesConfig &config);
void compute_V_restricted_core(PartFuncVContext &ctx,
                               PartFuncAllContext &all,
                               cand_pos_t i,
                               cand_pos_t j,
                               sparse_tree &tree,
                               const RulesConfig &config);
pf_t compute_VM_restricted_core(PartFuncVMContext &ctx,
                                PartFuncAllContext &all,
                                cand_pos_t i,
                                cand_pos_t j,
                                std::vector<int> &up,
                                const RulesConfig &config);
void compute_WI_restricted_core(PartFuncWIContext &ctx,
                                PartFuncAllContext &all,
                                cand_pos_t i,
                                cand_pos_t j,
                                sparse_tree &tree,
                                const RulesConfig &config);
void compute_WMv_WMp_restricted_core(PartFuncWMvWMpContext &ctx,
                                     PartFuncAllContext &all,
                                     cand_pos_t i,
                                     cand_pos_t j,
                                     std::vector<Node> &tree,
                                     const RulesConfig &config);
void compute_WM_restricted_core(PartFuncWMContext &ctx,
                                PartFuncAllContext &all,
                                cand_pos_t i,
                                cand_pos_t j,
                                sparse_tree &tree,
                                const RulesConfig &config);
void compute_WIP_restricted_core(PartFuncWIPContext &ctx,
                                 PartFuncAllContext &all,
                                 cand_pos_t i,
                                 cand_pos_t j,
                                 const StructureView &view,
                                 const RulesConfig &config);
void compute_VPL_restricted_core(PartFuncVPLContext &ctx,
                                 PartFuncAllContext &all,
                                 cand_pos_t i,
                                 cand_pos_t j,
                                 const StructureView &view,
                                 const RulesConfig &config);
void compute_VPR_restricted_core(PartFuncVPRContext &ctx,
                                 PartFuncAllContext &all,
                                 cand_pos_t i,
                                 cand_pos_t j,
                                 const StructureView &view,
                                 const RulesConfig &config);
void compute_VP_restricted_core(PartFuncVPContext &ctx,
                                PartFuncAllContext &all,
                                cand_pos_t i,
                                cand_pos_t j,
                                const StructureView &view,
                                sparse_tree &tree,
                                const RulesConfig &config);
void compute_WMBW_restricted_core(PartFuncWMBWContext &ctx,
                                  PartFuncAllContext &all,
                                  cand_pos_t i,
                                  cand_pos_t j,
                                  const StructureView &view,
                                  const RulesConfig &config);
void compute_WMBP_restricted_core(PartFuncWMBPContext &ctx,
                                  PartFuncAllContext &all,
                                  cand_pos_t i,
                                  cand_pos_t j,
                                  const StructureView &view,
                                  sparse_tree &tree,
                                  const RulesConfig &config);
void compute_WMB_restricted_core(PartFuncWMBContext &ctx,
                                 PartFuncAllContext &all,
                                 cand_pos_t i,
                                 cand_pos_t j,
                                 const StructureView &view,
                                 sparse_tree &tree,
                                 const RulesConfig &config);
void compute_BE_restricted_core(PartFuncBEContext &ctx,
                                PartFuncAllContext &all,
                                cand_pos_t i,
                                cand_pos_t j,
                                cand_pos_t ip,
                                cand_pos_t jp,
                                const StructureView &view,
                                sparse_tree &tree,
                                const RulesConfig &config);

} // namespace scfg

#endif
