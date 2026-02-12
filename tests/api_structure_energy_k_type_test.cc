#include "CPartyAPI.hh"
#include "fixed_structure_energy_internal.hh"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

namespace {

constexpr double kAbsTol = 1e-6;
constexpr double kRelTol = 1e-6;

bool approximately_equal(double a, double b) {
    const double scale = std::max(std::fabs(a), std::fabs(b));
    return std::fabs(a - b) <= std::max(kAbsTol, kRelTol * scale);
}

struct Case {
    std::string id;
    std::string seq;
    std::string db_full;
    bool expect_nan;
    bool expect_equal_to_internal;
};

} // namespace

int main() {
    const std::vector<Case> cases = {
        {"k_type_valid", "GGCGGAAAAACCGGCAAAAACCGCCAAAAACGGGUAAUAAGCUGGAAAAAGCCCG",
         "(((((.....[[[[[.....))))).....(((((.....]]]]].....)))))", false, true},
        {"k_type_valid_t_normalized", "GGCGGAAAAACCGGCAAAAACCGCCAAAAACGGGTAAUAAGCTGGAAAAAGCCCG",
         "(((((.....[[[[[.....))))).....(((((.....]]]]].....)))))", false, true},
        {"k_type_invalid_can_pair", "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA",
         "(((((.....[[[[[.....))))).....(((((.....]]]]].....)))))", true, false},
        {"unsupported_noncrossing_square", "GGGCAAAAGGCGAAAAGCCCAAAACGCC", "((((....[[[[....]]]]....))))", true, false},
    };

    size_t failed = 0;
    for (const Case &tc : cases) {
        const double first = get_structure_energy(tc.seq, tc.db_full);
        const double second = get_structure_energy(tc.seq, tc.db_full);

        if (tc.expect_nan) {
            if (!std::isnan(first) || !std::isnan(second)) {
                std::cerr << tc.id << ": expected NaN, got " << first << " and " << second << std::endl;
                ++failed;
            }
            continue;
        }

        if (!std::isfinite(first) || !std::isfinite(second)) {
            std::cerr << tc.id << ": expected finite values, got " << first << " and " << second << std::endl;
            ++failed;
            continue;
        }
        if (!approximately_equal(first, second)) {
            std::cerr << tc.id << ": expected deterministic values, got " << first << " and " << second << std::endl;
            ++failed;
            continue;
        }
        if (tc.expect_equal_to_internal) {
            const double internal = cparty::internal::evaluate_fixed_structure_energy_kcal(tc.seq, tc.db_full);
            if (!std::isfinite(internal) || !approximately_equal(first, internal)) {
                std::cerr << tc.id << ": expected API/internal match, api=" << first << " internal=" << internal << std::endl;
                ++failed;
            }
        }
    }

    if (failed != 0) {
        std::cerr << "api_structure_energy_k_type_test: failed=" << failed << std::endl;
        return 1;
    }

    std::cout << "api_structure_energy_k_type_test: checked=" << cases.size() << std::endl;
    return 0;
}
