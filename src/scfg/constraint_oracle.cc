#include "scfg/constraint_oracle.hh"

namespace scfg {

bool is_unpaired_position(const sparse_tree &tree, cand_pos_t pos) {
    if (pos <= 0 || pos >= static_cast<cand_pos_t>(tree.tree.size())) return false;
    return tree.tree[pos].pair < -1;
}

bool is_empty_region(const sparse_tree &tree, cand_pos_t left, cand_pos_t right) {
    if (right <= left) return false;
    if (right == left + 1) return true;
    if (tree.up.empty()) return false;
    const cand_pos_t up_index = right - 1;
    if (up_index <= 0 || up_index >= static_cast<cand_pos_t>(tree.up.size())) return false;
    return static_cast<cand_pos_t>(tree.up[up_index]) >= (right - left - 1);
}

bool is_pair_type_allowed(pair_type ptype) { return ptype > 0; }

} // namespace scfg
