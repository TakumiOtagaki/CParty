#include "CPartyAPI.hh"

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

struct EnergyCase {
    std::string id;
    std::string seq;
    std::string db_full;
};

} // namespace

int main() {
    size_t failed = 0;

    const std::vector<EnergyCase> valid_energy_cases = {
        {"pkfree_valid", "GCAUGC", "(....)"},
        {"h_type_valid", "GGGCAAAAGGCGAAAAGCCCAAAACGCC", "((((....[[[[....))))....]]]]"},
        {"k_type_valid", "GGCGGAAAAACCGGCAAAAACCGCCAAAAACGGGUAAUAAGCUGGAAAAAGCCCG",
         "(((((.....[[[[[.....))))).....(((((.....]]]]].....)))))"},
    };

    for (const EnergyCase &tc : valid_energy_cases) {
        const double first = get_structure_energy(tc.seq, tc.db_full);
        const double second = get_structure_energy(tc.seq, tc.db_full);
        if (!std::isfinite(first) || !std::isfinite(second)) {
            std::cerr << tc.id << ": expected finite outputs, got " << first << " and " << second << std::endl;
            ++failed;
            continue;
        }
        if (!approximately_equal(first, second)) {
            std::cerr << tc.id << ": expected deterministic outputs, got " << first << " and " << second << std::endl;
            ++failed;
        }
    }

    const std::string cond_seq = "GCAUGC";
    const std::string cond_db = "(....)";
    const double cond_before_first = get_cond_log_prob(cond_seq, cond_db);
    const double cond_before_second = get_cond_log_prob(cond_seq, cond_db);
    if (!std::isfinite(cond_before_first) || !std::isfinite(cond_before_second)) {
        std::cerr << "cond_log_prob_baseline: expected finite outputs, got " << cond_before_first << " and " << cond_before_second
                  << std::endl;
        ++failed;
    } else if (!approximately_equal(cond_before_first, cond_before_second)) {
        std::cerr << "cond_log_prob_baseline: expected deterministic outputs, got " << cond_before_first << " and "
                  << cond_before_second << std::endl;
        ++failed;
    }

    for (const EnergyCase &tc : valid_energy_cases) {
        (void)get_structure_energy(tc.seq, tc.db_full);
    }

    const double cond_after_first = get_cond_log_prob(cond_seq, cond_db);
    const double cond_after_second = get_cond_log_prob(cond_seq, cond_db);
    if (!std::isfinite(cond_after_first) || !std::isfinite(cond_after_second)) {
        std::cerr << "cond_log_prob_after_api_calls: expected finite outputs, got " << cond_after_first << " and " << cond_after_second
                  << std::endl;
        ++failed;
    } else {
        if (!approximately_equal(cond_after_first, cond_after_second)) {
            std::cerr << "cond_log_prob_after_api_calls: expected deterministic outputs, got " << cond_after_first << " and "
                      << cond_after_second << std::endl;
            ++failed;
        }
        if (!approximately_equal(cond_before_first, cond_after_first)) {
            std::cerr << "cond_log_prob_regression: expected unchanged value, before=" << cond_before_first
                      << " after=" << cond_after_first << std::endl;
            ++failed;
        }
    }

    if (failed != 0) {
        std::cerr << "api_structure_energy_compat_test: failed=" << failed << std::endl;
        return 1;
    }

    std::cout << "api_structure_energy_compat_test: checked=" << (valid_energy_cases.size() + 2) << std::endl;
    return 0;
}
