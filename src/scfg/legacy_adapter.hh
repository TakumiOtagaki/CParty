#ifndef SCFG_LEGACY_ADAPTER_HH_
#define SCFG_LEGACY_ADAPTER_HH_

#include "scfg/core.hh"

namespace scfg {

class LegacyAdapter : public Core {
  public:
    cand_pos_t unpaired_prefix(const sparse_tree &tree, cand_pos_t index) const override;
    bool can_pair_left_span(const sparse_tree &tree, cand_pos_t left, cand_pos_t split) const override;
    bool can_pair_right_span(const sparse_tree &tree, cand_pos_t split, cand_pos_t right) const override;
};

bool can_pair_left_span(const sparse_tree &tree, cand_pos_t left, cand_pos_t split);
bool can_pair_right_span(const sparse_tree &tree, cand_pos_t split, cand_pos_t right);

} // namespace scfg

#endif
