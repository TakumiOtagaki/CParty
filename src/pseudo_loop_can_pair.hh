#ifndef PSEUDO_LOOP_CAN_PAIR_HH_
#define PSEUDO_LOOP_CAN_PAIR_HH_

#include "base_types.hh"
#include <vector>

namespace cparty {
namespace pseudo_loop_can_pair {

bool can_use_left_unpaired_span(const std::vector<int> &up, cand_pos_t i, cand_pos_t k);
bool can_use_right_unpaired_span(const std::vector<int> &up, cand_pos_t k, cand_pos_t j);
energy_t cp_branch_penalty(cand_pos_t span);

} // namespace pseudo_loop_can_pair
} // namespace cparty

#endif
