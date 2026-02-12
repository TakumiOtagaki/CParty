#include "fixed_structure_energy_internal.hh"

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

        const double first = cparty::internal::evaluate_fixed_structure_energy_kcal(fixture.seq, fixture.db_full);
        const double second = cparty::internal::evaluate_fixed_structure_energy_kcal(fixture.seq, fixture.db_full);

        const bool expect_valid = fixture.expected == "valid";
        if (expect_valid) {
            if (!std::isfinite(first) || !std::isfinite(second)) {
                std::cerr << fixture.id << ": expected finite values, got " << first << " and " << second << std::endl;
                ++failed;
                continue;
            }
            if (!approximately_equal(first, second)) {
                std::cerr << fixture.id << ": expected deterministic values, got " << first << " and " << second << std::endl;
                ++failed;
                continue;
            }
        } else {
            if (!std::isnan(first) || !std::isnan(second)) {
                std::cerr << fixture.id << ": expected NaN values, got " << first << " and " << second << std::endl;
                ++failed;
                continue;
            }
        }

        ++checked;
    }

    if (failed != 0) {
        std::cerr << "fixed_energy_internal_test: failed=" << failed << " checked=" << checked << std::endl;
        return 1;
    }

    std::cout << "fixed_energy_internal_test: checked=" << checked << std::endl;
    return 0;
}
