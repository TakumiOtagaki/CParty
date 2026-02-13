#ifndef SCFG_RULES_PART_FUNC_HH_
#define SCFG_RULES_PART_FUNC_HH_

#include "base_types.hh"
#include "sparse_tree.hh"

namespace scfg {

struct PartFuncModeConfig {
    pf_t exp_pb_penalty;
    cand_pos_t turn;
};

class PartFuncRuleHelpers {
  public:
    PartFuncRuleHelpers(sparse_tree &tree, const PartFuncModeConfig &config);

    cand_pos_t border_b(cand_pos_t i, cand_pos_t j);
    cand_pos_t border_bp(cand_pos_t i, cand_pos_t j);
    cand_pos_t border_B(cand_pos_t i, cand_pos_t j);
    cand_pos_t border_Bp(cand_pos_t i, cand_pos_t j);
    cand_pos_t pair_at(cand_pos_t pos) const;
    cand_pos_t parent_index(cand_pos_t pos) const;

    bool allow_exterior_split(cand_pos_t l, cand_pos_t j, cand_pos_t b_ij, int exterior_case) const;
    bool has_valid_band_borders(cand_pos_t i, cand_pos_t l, cand_pos_t j);
    bool parent_within_interval_and_turn(cand_pos_t i, cand_pos_t l, cand_pos_t j) const;
    bool has_valid_inner_arc_split(cand_pos_t i, cand_pos_t l, cand_pos_t j, cand_pos_t n_bound);

    pf_t apply_double_pb_penalty(pf_t value) const;

    template <typename Fn>
    void for_each_split(cand_pos_t i, cand_pos_t j, Fn fn) const {
        for (cand_pos_t l = i + 1; l < j; ++l) fn(l);
    }

    void on_traceback_hook(cand_pos_t i, cand_pos_t j) const;
    void on_fixed_parse_hook(cand_pos_t i, cand_pos_t j) const;

  private:
    sparse_tree &tree_;
    PartFuncModeConfig config_;
};

} // namespace scfg

#endif
