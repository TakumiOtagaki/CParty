#ifndef SCFG_CORE_HH_
#define SCFG_CORE_HH_

#include "base_types.hh"
#include "sparse_tree.hh"

namespace scfg {

class Core {
  public:
    virtual ~Core();

    virtual cand_pos_t unpaired_prefix(const sparse_tree &tree, cand_pos_t index) const = 0;
    virtual bool can_pair_left_span(const sparse_tree &tree, cand_pos_t left, cand_pos_t split) const = 0;
    virtual bool can_pair_right_span(const sparse_tree &tree, cand_pos_t split, cand_pos_t right) const = 0;
};

} // namespace scfg

#endif
