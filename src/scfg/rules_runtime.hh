#ifndef SCFG_RULES_RUNTIME_HH_
#define SCFG_RULES_RUNTIME_HH_

#include "scfg/rules_engine.hh"
#include "scfg/rules_part_func.hh"

namespace scfg {

void compute_W_restricted_rules(PartFuncWContext &ctx, sparse_tree &tree, const RulesConfig &config);
void compute_V_restricted_rules(PartFuncVContext &ctx, cand_pos_t i, cand_pos_t j, sparse_tree &tree, const RulesConfig &config);

} // namespace scfg

#endif
