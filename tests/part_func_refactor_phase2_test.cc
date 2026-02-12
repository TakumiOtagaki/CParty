#include "part_func_can_pair.hh"

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

    if (!cparty::part_func_can_pair::can_use_left_unpaired_span(up, 1, 3)) {
        std::cerr << "left-span can-pair predicate rejected covered span" << std::endl;
        return 1;
    }
    if (cparty::part_func_can_pair::can_use_left_unpaired_span(up, 1, 5)) {
        std::cerr << "left-span can-pair predicate accepted uncovered span" << std::endl;
        return 1;
    }

    if (!cparty::part_func_can_pair::can_use_right_unpaired_span(up, 2, 5)) {
        std::cerr << "right-span can-pair predicate rejected covered span" << std::endl;
        return 1;
    }
    if (cparty::part_func_can_pair::can_use_right_unpaired_span(up, 1, 5)) {
        std::cerr << "right-span can-pair predicate accepted uncovered span" << std::endl;
        return 1;
    }

    if (!cparty::part_func_can_pair::can_use_hairpin_unpaired_span(up, 1, 3)) {
        std::cerr << "hairpin-span predicate rejected covered interior span" << std::endl;
        return 1;
    }
    if (cparty::part_func_can_pair::can_use_hairpin_unpaired_span(up, 2, 6)) {
        std::cerr << "hairpin-span predicate accepted uncovered interior span" << std::endl;
        return 1;
    }

    if (!cparty::part_func_can_pair::can_use_internal_left_unpaired_span(up, 1, 3)) {
        std::cerr << "internal-left predicate rejected covered span" << std::endl;
        return 1;
    }
    if (cparty::part_func_can_pair::can_use_internal_left_unpaired_span(up, 1, 6)) {
        std::cerr << "internal-left predicate accepted uncovered span" << std::endl;
        return 1;
    }

    if (!cparty::part_func_can_pair::can_use_internal_right_unpaired_span(up, 2, 5)) {
        std::cerr << "internal-right predicate rejected covered span" << std::endl;
        return 1;
    }
    if (cparty::part_func_can_pair::can_use_internal_right_unpaired_span(up, 1, 6)) {
        std::cerr << "internal-right predicate accepted uncovered span" << std::endl;
        return 1;
    }

    return 0;
}
