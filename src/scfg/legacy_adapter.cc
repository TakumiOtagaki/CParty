#include "scfg/legacy_adapter.hh"

#include <algorithm>

namespace scfg {

cand_pos_t LegacyAdapter::unpaired_prefix(const sparse_tree &tree, cand_pos_t index) const {
    if (tree.up.empty() || index <= 0) return 0;
    const cand_pos_t capped = std::min<cand_pos_t>(index, static_cast<cand_pos_t>(tree.up.size() - 1));
    return static_cast<cand_pos_t>(tree.up[capped]);
}

bool LegacyAdapter::can_pair_left_span(const sparse_tree &tree, cand_pos_t left, cand_pos_t split) const {
    if (split <= left) return false;
    return unpaired_prefix(tree, split - 1) >= (split - left);
}

bool LegacyAdapter::can_pair_right_span(const sparse_tree &tree, cand_pos_t split, cand_pos_t right) const {
    if (right <= split) return false;
    return unpaired_prefix(tree, right - 1) >= (right - split);
}

bool can_pair_left_span(const sparse_tree &tree, cand_pos_t left, cand_pos_t split) {
    static const LegacyAdapter kLegacyAdapter;
    return kLegacyAdapter.can_pair_left_span(tree, left, split);
}

bool can_pair_right_span(const sparse_tree &tree, cand_pos_t split, cand_pos_t right) {
    static const LegacyAdapter kLegacyAdapter;
    return kLegacyAdapter.can_pair_right_span(tree, split, right);
}

} // namespace scfg
