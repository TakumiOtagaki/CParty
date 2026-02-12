#include "fixed_structure_energy_internal.hh"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

namespace {

constexpr double kAbsTol = 1e-6;
constexpr double kRelTol = 1e-6;

struct Fixture {
    std::string id;
    std::string family;
    std::string expected;
    std::string seq;
    std::string db_full;
    std::string invalid_reason;
};

bool approximately_equal(double a, double b) {
    const double scale = std::max(std::fabs(a), std::fabs(b));
    return std::fabs(a - b) <= std::max(kAbsTol, kRelTol * scale);
}

bool parse_fixture_line(const std::string &line, Fixture &fixture) {
    std::vector<std::string> cols;
    std::stringstream ss(line);
    std::string part;

    while (std::getline(ss, part, '\t')) {
        cols.push_back(part);
    }
    if (cols.size() < 6) {
        return false;
    }

    fixture.id = cols[0];
    fixture.family = cols[1];
    fixture.expected = cols[2];
    fixture.seq = cols[3];
    fixture.db_full = cols[4];
    fixture.invalid_reason = cols[5];
    return true;
}

} // namespace

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "usage: " << argv[0] << " <fixtures.tsv>" << std::endl;
        return 2;
    }

    std::ifstream input(argv[1]);
    if (!input.is_open()) {
        std::cerr << "failed to open fixture file: " << argv[1] << std::endl;
        return 2;
    }

    size_t line_no = 0;
    size_t checked = 0;
    size_t failed = 0;
    std::string line;

    while (std::getline(input, line)) {
        ++line_no;
        if (line.empty() || line[0] == '#') {
            continue;
        }

        Fixture fixture;
        if (!parse_fixture_line(line, fixture)) {
            std::cerr << "invalid fixture row at line " << line_no << std::endl;
            ++failed;
            continue;
        }

        const double energy = cparty::internal::evaluate_fixed_structure_energy_kcal(fixture.seq, fixture.db_full);
        const cparty::internal::EnergyBreakdown breakdown =
            cparty::internal::evaluate_fixed_structure_energy_breakdown_kcal(fixture.seq, fixture.db_full);

        const bool expect_valid = fixture.expected == "valid";
        if (expect_valid) {
            if (!std::isfinite(energy) || !std::isfinite(breakdown.total_kcal) || !std::isfinite(breakdown.pk_free_core_kcal) ||
                !std::isfinite(breakdown.pk_penalties_kcal) || !std::isfinite(breakdown.band_scaled_terms_kcal)) {
                std::cerr << fixture.id << ": expected finite breakdown values for valid fixture" << std::endl;
                ++failed;
                continue;
            }
            if (!approximately_equal(energy, breakdown.total_kcal)) {
                std::cerr << fixture.id << ": total mismatch energy=" << energy << " breakdown.total=" << breakdown.total_kcal
                          << std::endl;
                ++failed;
                continue;
            }

            const double recomposed =
                breakdown.pk_free_core_kcal + breakdown.pk_penalties_kcal + breakdown.band_scaled_terms_kcal;
            if (!approximately_equal(recomposed, breakdown.total_kcal)) {
                std::cerr << fixture.id << ": recomposed mismatch recomposed=" << recomposed
                          << " breakdown.total=" << breakdown.total_kcal << std::endl;
                ++failed;
                continue;
            }
        } else {
            if (!std::isnan(energy) || !std::isnan(breakdown.total_kcal) || !std::isnan(breakdown.pk_free_core_kcal) ||
                !std::isnan(breakdown.pk_penalties_kcal) || !std::isnan(breakdown.band_scaled_terms_kcal)) {
                std::cerr << fixture.id << ": expected NaN breakdown values for invalid fixture" << std::endl;
                ++failed;
                continue;
            }
        }

        ++checked;
    }

    if (failed != 0) {
        std::cerr << "fixed_energy_breakdown_test: failed=" << failed << " checked=" << checked << std::endl;
        return 1;
    }

    std::cout << "fixed_energy_breakdown_test: checked=" << checked << std::endl;
    return 0;
}
