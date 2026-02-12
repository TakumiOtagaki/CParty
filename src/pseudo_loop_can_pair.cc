#include "pseudo_loop_can_pair.hh"
#include "h_externs.hh"

namespace cparty {
namespace pseudo_loop_can_pair {

bool can_use_left_unpaired_span(const std::vector<int> &up, cand_pos_t i, cand_pos_t k) { return up[k - 1] >= (k - i); }

bool can_use_right_unpaired_span(const std::vector<int> &up, cand_pos_t k, cand_pos_t j) { return up[j - 1] >= (j - k); }

energy_t cp_branch_penalty(cand_pos_t span) { return static_cast<energy_t>(span * cp_penalty); }

} // namespace pseudo_loop_can_pair
} // namespace cparty
