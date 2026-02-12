#ifndef PART_FUNC_CAN_PAIR_HH_
#define PART_FUNC_CAN_PAIR_HH_

#include "base_types.hh"
#include <string>
#include <vector>

namespace cparty {
namespace part_func_can_pair {

bool can_use_left_unpaired_span(const std::vector<int> &up, cand_pos_t i, cand_pos_t k);
bool can_use_right_unpaired_span(const std::vector<int> &up, cand_pos_t k, cand_pos_t j);
bool can_use_hairpin_unpaired_span(const std::vector<int> &up, cand_pos_t i, cand_pos_t j);
bool can_use_internal_left_unpaired_span(const std::vector<int> &up, cand_pos_t i, cand_pos_t k);
bool can_use_internal_right_unpaired_span(const std::vector<int> &up, cand_pos_t l, cand_pos_t j);
bool can_form_allowed_pair(const std::string &seq, cand_pos_t left, cand_pos_t right);

} // namespace part_func_can_pair
} // namespace cparty

#endif
