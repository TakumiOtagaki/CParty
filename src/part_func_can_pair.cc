#include "part_func_can_pair.hh"

namespace cparty {
namespace part_func_can_pair {

bool can_use_left_unpaired_span(const std::vector<int> &up, cand_pos_t i, cand_pos_t k) { return up[k - 1] >= (k - i); }

bool can_use_right_unpaired_span(const std::vector<int> &up, cand_pos_t k, cand_pos_t j) { return up[j - 1] >= (j - k); }

bool can_use_hairpin_unpaired_span(const std::vector<int> &up, cand_pos_t i, cand_pos_t j) { return up[j - 1] >= (j - i - 1); }

bool can_use_internal_left_unpaired_span(const std::vector<int> &up, cand_pos_t i, cand_pos_t k) { return up[k - 1] >= (k - i - 1); }

bool can_use_internal_right_unpaired_span(const std::vector<int> &up, cand_pos_t l, cand_pos_t j) { return up[j - 1] >= (j - l - 1); }

} // namespace part_func_can_pair
} // namespace cparty
