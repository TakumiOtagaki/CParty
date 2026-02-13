#ifndef SCFG_PART_FUNC_ADAPTER_HH_
#define SCFG_PART_FUNC_ADAPTER_HH_

#include "base_types.hh"
#include "scfg/rules_part_func.hh"

class W_final_pf;
class sparse_tree;

namespace scfg {

struct PartFuncAdapterAccess;

void compute_W_restricted(W_final_pf &owner, sparse_tree &tree);
pf_t compute_VM_restricted(W_final_pf &owner, cand_pos_t i, cand_pos_t j, std::vector<int> &up);
void compute_V_restricted(W_final_pf &owner, cand_pos_t i, cand_pos_t j, sparse_tree &tree);
void compute_WI_restricted(W_final_pf &owner, cand_pos_t i, cand_pos_t j, sparse_tree &tree);

} // namespace scfg

#endif
