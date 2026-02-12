#include "can_pair_policy.hh"

#include <iostream>
#include <vector>

int main() {
    const std::vector<std::pair<char, char>> allowed = {
        {'A', 'U'},
        {'U', 'A'},
        {'C', 'G'},
        {'G', 'C'},
        {'G', 'U'},
        {'U', 'G'},
        {'a', 'u'},
    };

    for (const auto &p : allowed) {
        if (!cparty::can_pair_policy::is_allowed_base_pair(p.first, p.second)) {
            std::cerr << "expected allowed pair rejected: " << p.first << "-" << p.second << std::endl;
            return 1;
        }
    }

    const std::vector<std::pair<char, char>> rejected = {
        {'A', 'A'},
        {'C', 'U'},
        {'N', 'G'},
        {'T', 'A'},
        {'X', 'U'},
    };

    for (const auto &p : rejected) {
        if (cparty::can_pair_policy::is_allowed_base_pair(p.first, p.second)) {
            std::cerr << "unexpected allowed pair accepted: " << p.first << "-" << p.second << std::endl;
            return 1;
        }
    }

    if (!cparty::can_pair_policy::is_tree_up_pairable(4, 4)) {
        std::cerr << "tree.up policy rejected exact-match span" << std::endl;
        return 1;
    }
    if (!cparty::can_pair_policy::is_tree_up_pairable(7, 3)) {
        std::cerr << "tree.up policy rejected covered span" << std::endl;
        return 1;
    }
    if (cparty::can_pair_policy::is_tree_up_pairable(2, 3)) {
        std::cerr << "tree.up policy accepted uncovered span" << std::endl;
        return 1;
    }

    return 0;
}
