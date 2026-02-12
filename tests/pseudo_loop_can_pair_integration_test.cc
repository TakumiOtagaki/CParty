#include "pseudo_loop_can_pair.hh"
#include "h_externs.hh"

#include <iostream>
#include <vector>

int main() {
    const std::vector<int> up = {
        0, // unused 0-index slot for 1-based DP indexing
        0,
        2,
        0,
        3,
        0,
    };

    if (!cparty::pseudo_loop_can_pair::can_use_internal_left_unpaired_span(up, 1, 3)) {
        std::cerr << "internal-left span predicate rejected covered span" << std::endl;
        return 1;
    }
    if (cparty::pseudo_loop_can_pair::can_use_internal_left_unpaired_span(up, 1, 6)) {
        std::cerr << "internal-left span predicate accepted uncovered span" << std::endl;
        return 1;
    }

    if (!cparty::pseudo_loop_can_pair::can_use_internal_right_unpaired_span(up, 2, 5)) {
        std::cerr << "internal-right span predicate rejected covered span" << std::endl;
        return 1;
    }
    if (cparty::pseudo_loop_can_pair::can_use_internal_right_unpaired_span(up, 1, 6)) {
        std::cerr << "internal-right span predicate accepted uncovered span" << std::endl;
        return 1;
    }

    const std::string sequence = "AUGC";
    if (!cparty::pseudo_loop_can_pair::can_form_allowed_pair(sequence, 1, 2)) {
        std::cerr << "allowed pair A-U was rejected by shared can-pair policy" << std::endl;
        return 1;
    }
    if (cparty::pseudo_loop_can_pair::can_form_allowed_pair(sequence, 1, 4)) {
        std::cerr << "disallowed pair A-C was accepted by shared can-pair policy" << std::endl;
        return 1;
    }
    if (cparty::pseudo_loop_can_pair::can_form_allowed_pair(sequence, 0, 2)) {
        std::cerr << "out-of-range left index accepted in can-pair policy helper" << std::endl;
        return 1;
    }
    if (cparty::pseudo_loop_can_pair::can_form_allowed_pair(sequence, 2, 6)) {
        std::cerr << "out-of-range right index accepted in can-pair policy helper" << std::endl;
        return 1;
    }

    if (cparty::pseudo_loop_can_pair::cp_branch_penalty(2) != 2 * cp_penalty) {
        std::cerr << "cp-penalty helper mismatched expected scaled span penalty" << std::endl;
        return 1;
    }

    return 0;
}
