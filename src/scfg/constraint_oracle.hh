#ifndef SCFG_CONSTRAINT_ORACLE_HH_
#define SCFG_CONSTRAINT_ORACLE_HH_

#include "base_types.hh"
#include "sparse_tree.hh"

namespace scfg {

bool is_unpaired_position(const sparse_tree &tree, cand_pos_t pos);
bool is_empty_region(const sparse_tree &tree, cand_pos_t left, cand_pos_t right);
bool is_pair_type_allowed(pair_type ptype);

} // namespace scfg

#endif
