#include "pseudo_loop_can_pair.hh"
#include "can_pair_policy.hh"
#include "h_externs.hh"

namespace cparty {
namespace pseudo_loop_can_pair {

bool can_use_left_unpaired_span(const std::vector<int> &up, cand_pos_t i, cand_pos_t k) {
    return cparty::can_pair_policy::is_tree_up_pairable(up[k - 1], k - i);
}

bool can_use_right_unpaired_span(const std::vector<int> &up, cand_pos_t k, cand_pos_t j) {
    return cparty::can_pair_policy::is_tree_up_pairable(up[j - 1], j - k);
}

bool can_use_internal_left_unpaired_span(const std::vector<int> &up, cand_pos_t i, cand_pos_t k) {
    return cparty::can_pair_policy::is_tree_up_pairable(up[k - 1], k - i - 1);
}

bool can_use_internal_right_unpaired_span(const std::vector<int> &up, cand_pos_t l, cand_pos_t j) {
    return cparty::can_pair_policy::is_tree_up_pairable(up[j - 1], j - l - 1);
}

bool can_form_allowed_pair(const std::string &seq, cand_pos_t left, cand_pos_t right) {
    if (left < 1 || right < 1) {
        return false;
    }
    if (left > static_cast<cand_pos_t>(seq.size()) || right > static_cast<cand_pos_t>(seq.size())) {
        return false;
    }
    return cparty::can_pair_policy::is_allowed_base_pair(seq[left - 1], seq[right - 1]);
}

energy_t cp_branch_penalty(cand_pos_t span) { return static_cast<energy_t>(span * cp_penalty); }

} // namespace pseudo_loop_can_pair
} // namespace cparty
