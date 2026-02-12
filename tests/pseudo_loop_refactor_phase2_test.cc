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

    if (!cparty::pseudo_loop_can_pair::can_use_left_unpaired_span(up, 1, 3)) {
        std::cerr << "left-span can-pair predicate rejected covered span" << std::endl;
        return 1;
    }
    if (cparty::pseudo_loop_can_pair::can_use_left_unpaired_span(up, 1, 5)) {
        std::cerr << "left-span can-pair predicate accepted uncovered span" << std::endl;
        return 1;
    }

    if (!cparty::pseudo_loop_can_pair::can_use_right_unpaired_span(up, 2, 5)) {
        std::cerr << "right-span can-pair predicate rejected covered span" << std::endl;
        return 1;
    }
    if (cparty::pseudo_loop_can_pair::can_use_right_unpaired_span(up, 1, 5)) {
        std::cerr << "right-span can-pair predicate accepted uncovered span" << std::endl;
        return 1;
    }

    if (cparty::pseudo_loop_can_pair::cp_branch_penalty(0) != 0) {
        std::cerr << "cp-penalty helper mismatched zero span" << std::endl;
        return 1;
    }
    if (cparty::pseudo_loop_can_pair::cp_branch_penalty(3) != 3 * cp_penalty) {
        std::cerr << "cp-penalty helper mismatched positive span" << std::endl;
        return 1;
    }

    return 0;
}
