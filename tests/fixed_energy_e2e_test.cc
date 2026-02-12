#include "CPartyAPI.hh"

#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

namespace {

struct FixtureCase {
    std::string id;
    std::string family;
    std::string expected;
    std::string reason;
    std::string seq;
    std::string db_full;
};

struct CaseResult {
    FixtureCase fixture;
    double energy;
    bool finite;
    bool valid_contract;
};

std::string trim(const std::string& input) {
    const std::string spaces = " \t\r\n";
    const std::size_t start = input.find_first_not_of(spaces);
    if (start == std::string::npos) {
        return "";
    }
    const std::size_t end = input.find_last_not_of(spaces);
    return input.substr(start, end - start + 1);
}

bool parse_header(const std::string& header, FixtureCase& fixture, std::string& error) {
    std::size_t cursor = 0;
    std::vector<std::string> tokens;
    while (true) {
        const std::size_t sep = header.find('|', cursor);
        tokens.push_back(trim(header.substr(cursor, sep == std::string::npos ? std::string::npos : sep - cursor)));
        if (sep == std::string::npos) {
            break;
        }
        cursor = sep + 1;
    }

    if (tokens.empty() || tokens[0].empty()) {
        error = "missing fixture id in FASTA header";
        return false;
    }
    fixture.id = tokens[0];

    for (std::size_t i = 1; i < tokens.size(); ++i) {
        const std::string token = tokens[i];
        const std::size_t eq = token.find('=');
        if (eq == std::string::npos) {
            continue;
        }
        const std::string key = trim(token.substr(0, eq));
        const std::string value = trim(token.substr(eq + 1));
        if (key == "family") {
            fixture.family = value;
        } else if (key == "expected") {
            fixture.expected = value;
        } else if (key == "reason") {
            fixture.reason = value;
        }
    }

    if (fixture.expected.empty()) {
        error = "missing expected field for fixture '" + fixture.id + "'";
        return false;
    }

    return true;
}

bool load_fixtures(const std::string& path, std::vector<FixtureCase>& fixtures, std::string& error) {
    std::ifstream input(path);
    if (!input.is_open()) {
        error = "unable to open fixture file: " + path;
        return false;
    }

    FixtureCase current;
    bool have_current = false;
    std::string line;
    int line_no = 0;
    while (std::getline(input, line)) {
        ++line_no;
        const std::string trimmed = trim(line);
        if (trimmed.empty()) {
            continue;
        }
        if (trimmed[0] == '>') {
            if (have_current) {
                if (current.seq.empty() || current.db_full.empty()) {
                    error = "incomplete fixture before header at line " + std::to_string(line_no);
                    return false;
                }
                fixtures.push_back(current);
            }
            current = FixtureCase{};
            have_current = true;
            if (!parse_header(trimmed.substr(1), current, error)) {
                error += " (line " + std::to_string(line_no) + ")";
                return false;
            }
            continue;
        }

        if (!have_current) {
            error = "encountered sequence/structure before first header at line " + std::to_string(line_no);
            return false;
        }

        if (current.seq.empty()) {
            current.seq = trimmed;
        } else if (current.db_full.empty()) {
            current.db_full = trimmed;
        } else {
            error = "fixture '" + current.id + "' has more than two payload lines";
            return false;
        }
    }

    if (have_current) {
        if (current.seq.empty() || current.db_full.empty()) {
            error = "incomplete final fixture record";
            return false;
        }
        fixtures.push_back(current);
    }

    if (fixtures.empty()) {
        error = "fixture file contains no records: " + path;
        return false;
    }

    return true;
}

bool write_report(const std::string& path, const std::vector<CaseResult>& results, std::string& error) {
    std::ofstream out(path);
    if (!out.is_open()) {
        error = "unable to write report file: " + path;
        return false;
    }

    out << "# Fixed Energy End-to-End Report\n\n";
    out << "Source fixture: `tests/cparty_fixed_structure_energy/test.multi.fa`\n\n";
    out << "## Observed Energies\n\n";
    out << "| id | family | expected | observed_energy_kcal_mol |\n";
    out << "| --- | --- | --- | --- |\n";
    for (const CaseResult& result : results) {
        out << "| " << result.fixture.id << " | " << result.fixture.family << " | " << result.fixture.expected << " | ";
        if (result.finite) {
            out << result.energy;
        } else {
            out << "NaN";
        }
        out << " |\n";
    }

    out << "\n## Unsupported/Invalid Cases\n\n";
    out << "| id | reason |\n";
    out << "| --- | --- |\n";
    for (const CaseResult& result : results) {
        const bool is_invalid_expected = (result.fixture.expected == "invalid");
        const bool unexpected_nan = (result.fixture.expected == "valid" && !result.finite);
        if (!is_invalid_expected && !unexpected_nan) {
            continue;
        }

        std::string reason = trim(result.fixture.reason);
        if (is_invalid_expected && reason.empty()) {
            reason = "invalid_expected_without_reason";
        }
        if (unexpected_nan) {
            reason = "unexpected_nan_for_valid_case";
        }
        out << "| " << result.fixture.id << " | " << reason << " |\n";
    }

    return true;
}

}  // namespace

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "usage: " << argv[0] << " <fixture_multi_fa> <output_report_md>" << std::endl;
        return 1;
    }

    std::vector<FixtureCase> fixtures;
    std::string error;
    if (!load_fixtures(argv[1], fixtures, error)) {
        std::cerr << error << std::endl;
        return 1;
    }

    bool ok = true;
    std::vector<CaseResult> results;
    results.reserve(fixtures.size());
    for (const FixtureCase& fixture : fixtures) {
        const double energy = get_structure_energy(fixture.seq, fixture.db_full);
        const bool finite = std::isfinite(energy);
        const bool valid = (fixture.expected == "valid");
        const bool invalid = (fixture.expected == "invalid");
        const bool contract_ok = (valid && finite) || (invalid && std::isnan(energy));
        if (!contract_ok) {
            ok = false;
            std::cerr << "contract violation for case '" << fixture.id << "' (expected=" << fixture.expected
                      << ", observed=" << (finite ? "finite" : "NaN") << ")" << std::endl;
        }

        results.push_back(CaseResult{fixture, energy, finite, contract_ok});
    }

    if (!write_report(argv[2], results, error)) {
        std::cerr << error << std::endl;
        return 1;
    }

    for (const CaseResult& result : results) {
        if (!result.valid_contract) {
            ok = false;
        }
    }

    return ok ? 0 : 1;
}
