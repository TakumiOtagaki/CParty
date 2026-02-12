#include "CPartyAPI.hh"

#include <cmath>
#include <iostream>
#include <string>
#include <vector>

namespace {

struct Case {
    std::string id;
    std::string seq;
    std::string db_full;
    bool expect_nan;
};

} // namespace

int main() {
    const std::vector<Case> cases = {
        {"valid_pkfree", "GCAUGC", "(....)", false},
        {"valid_t_normalization", "GCATGC", "(....)", false},
        {"invalid_length", "GCAAA", "(....)", true},
        {"invalid_symbol", "GCXAC", "(..).", true},
        {"invalid_unbalanced", "GCAAC", "((..)", true},
        {"invalid_mixed_pk", "AGGGGAAAAAGGAGGAAAUGGGGAAAAAAACCCCUAAAACCUCCAAAGGCGGAAAAACCCCGAAAAACCGCC",
         "(((((.....{{{{{...[[[[[.......)))))....}}}}}...(((((.....]]]]].....)))))", true},
        {"invalid_structure_char", "GCAUGC", "(..a.)", true},
    };

    size_t failed = 0;
    for (const Case &tc : cases) {
        const double energy = get_structure_energy(tc.seq, tc.db_full);
        const bool is_nan = std::isnan(energy);
        if (tc.expect_nan != is_nan) {
            std::cerr << tc.id << ": expected NaN=" << tc.expect_nan << " got " << energy << std::endl;
            ++failed;
        }
    }

    if (failed != 0) {
        std::cerr << "api_structure_energy_validation_test: failed=" << failed << std::endl;
        return 1;
    }

    std::cout << "api_structure_energy_validation_test: checked=" << cases.size() << std::endl;
    return 0;
}
