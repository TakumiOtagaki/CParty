#include "CPartyAPI.hh"
#include "part_func_can_pair.hh"
#include "pseudo_loop_can_pair.hh"

#include <cmath>
#include <iostream>
#include <string>
#include <vector>

namespace {

struct Case {
    std::string id;
    std::string family;
    std::string seq;
    std::string db_full;
    int left;
    int right;
    bool expected_accept;
};

} // namespace

int main() {
    const std::vector<Case> matrix = {
        {"pk_free_accept", "pk_free", "GCAUGC", "(....)", 1, 6, true},
        {"pk_free_reject_can_pair", "pk_free", "AAAAAA", "(....)", 1, 6, false},
        {"h_type_accept", "h_type", "GGGCAAAAGGCGAAAAGCCCAAAACGCC", "((((....[[[[....))))....]]]]", 9, 28, true},
        {"h_type_reject_can_pair", "h_type", "AAAAAAAAAAAAAAAAAAAAAAAAAAAA", "((((....[[[[....))))....]]]]", 9, 28, false},
        {"k_type_accept", "k_type", "GGCGGAAAAACCGGCAAAAACCGCCAAAAACGGGUAAUAAGCUGGAAAAAGCCCG",
         "(((((.....[[[[[.....))))).....(((((.....]]]]].....)))))", 13, 43, true},
        {"k_type_reject_can_pair", "k_type", "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA",
         "(((((.....[[[[[.....))))).....(((((.....]]]]].....)))))", 13, 43, false},
        {"invalid_symbol_reject", "invalid", "XCAUGC", "(....)", 1, 6, false},
    };

    size_t failed = 0;
    for (const Case &tc : matrix) {
        const double api_energy = get_structure_energy(tc.seq, tc.db_full);
        const bool api_accept = std::isfinite(api_energy);
        const bool pseudo_accept = cparty::pseudo_loop_can_pair::can_form_allowed_pair(tc.seq, tc.left, tc.right);
        const bool part_func_accept = cparty::part_func_can_pair::can_form_allowed_pair(tc.seq, tc.left, tc.right);

        if (api_accept != tc.expected_accept) {
            std::cerr << tc.id << ": API mismatch (expected " << tc.expected_accept << ", got " << api_accept << ")"
                      << std::endl;
            ++failed;
        }
        if (pseudo_accept != tc.expected_accept) {
            std::cerr << tc.id << ": pseudo_loop mismatch (expected " << tc.expected_accept << ", got "
                      << pseudo_accept << ")" << std::endl;
            ++failed;
        }
        if (part_func_accept != tc.expected_accept) {
            std::cerr << tc.id << ": part_func mismatch (expected " << tc.expected_accept << ", got "
                      << part_func_accept << ")" << std::endl;
            ++failed;
        }
    }

    if (failed != 0) {
        std::cerr << "can_pair_cross_path_consistency_test: failed=" << failed << std::endl;
        return 1;
    }

    std::cout << "can_pair_cross_path_consistency_test: checked=" << matrix.size() << std::endl;
    return 0;
}
