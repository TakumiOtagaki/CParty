#include "CPartyAPI.hh"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace {

constexpr double kAbsTol = 1e-6;
constexpr double kRelTol = 1e-6;

struct FixtureRecord {
    std::string id;
    std::string seq;
};

struct BaselineRow {
    std::string fixture_id;
    std::string flag_id;
    std::string flags;
    std::string constraint_mode;
    int exit_code;
    std::string status;
    std::string mfe_structure;
    double mfe_energy;
};

struct ComparisonResult {
    std::string fixture_id;
    std::string flag_id;
    std::string flags;
    double cli_energy;
    double api_energy;
    bool within_tolerance;
};

struct SkippedRow {
    std::string fixture_id;
    std::string flag_id;
    std::string flags;
    int exit_code;
    std::string reason;
};

bool has_flag(const std::string &flags, const std::string &flag) {
    if (flags.empty()) {
        return false;
    }
    std::istringstream iss(flags);
    std::string token;
    while (iss >> token) {
        if (token == flag) {
            return true;
        }
    }
    return false;
}

cparty::EnergyEvalOptions options_from_flags(const std::string &flags) {
    cparty::EnergyEvalOptions options;
    options.pk_free = has_flag(flags, "-p");
    options.pk_only = has_flag(flags, "-k");
    options.dangles = has_flag(flags, "-d0") ? 0 : 2;
    return options;
}

std::string trim(const std::string &input) {
    const std::string spaces = " \t\r\n";
    const std::size_t start = input.find_first_not_of(spaces);
    if (start == std::string::npos) {
        return "";
    }
    const std::size_t end = input.find_last_not_of(spaces);
    return input.substr(start, end - start + 1);
}

bool approximately_equal(double a, double b) {
    const double scale = std::max(std::fabs(a), std::fabs(b));
    return std::fabs(a - b) <= std::max(kAbsTol, kRelTol * scale);
}

bool parse_header_id(const std::string &header, std::string &id) {
    std::size_t pipe = header.find('|');
    const std::string leading = (pipe == std::string::npos) ? header : header.substr(0, pipe);
    id = trim(leading);
    return !id.empty();
}

bool load_fixture_sequences(const std::string &path, std::unordered_map<std::string, std::string> &seq_by_id, std::string &error) {
    std::ifstream input(path);
    if (!input.is_open()) {
        error = "unable to open fixture file: " + path;
        return false;
    }

    FixtureRecord current;
    bool have_current = false;
    std::string line;
    int line_no = 0;
    while (std::getline(input, line)) {
        ++line_no;
        const std::string t = trim(line);
        if (t.empty()) {
            continue;
        }

        if (t[0] == '>') {
            if (have_current && !current.seq.empty()) {
                seq_by_id[current.id] = current.seq;
            }
            current = FixtureRecord{};
            have_current = true;
            if (!parse_header_id(t.substr(1), current.id)) {
                error = "invalid header without fixture id at line " + std::to_string(line_no);
                return false;
            }
            continue;
        }

        if (!have_current) {
            error = "encountered record payload before header at line " + std::to_string(line_no);
            return false;
        }

        if (current.seq.empty()) {
            current.seq = t;
        }
    }

    if (have_current && !current.seq.empty()) {
        seq_by_id[current.id] = current.seq;
    }

    if (seq_by_id.empty()) {
        error = "fixture file contains no readable sequence entries: " + path;
        return false;
    }

    return true;
}

bool parse_baseline_row(const std::string &line, BaselineRow &row) {
    std::vector<std::string> cols;
    std::size_t start = 0;
    while (true) {
        const std::size_t sep = line.find('\t', start);
        if (sep == std::string::npos) {
            cols.push_back(line.substr(start));
            break;
        }
        cols.push_back(line.substr(start, sep - start));
        start = sep + 1;
    }
    if (cols.size() != 8) {
        return false;
    }

    row.fixture_id = cols[0];
    row.flag_id = cols[1];
    row.flags = cols[2];
    row.constraint_mode = cols[3];
    try {
        row.exit_code = std::stoi(cols[4]);
    } catch (...) {
        return false;
    }
    row.status = cols[5];
    row.mfe_structure = cols[6];
    if (row.status == "ok") {
        try {
            row.mfe_energy = std::stod(cols[7]);
        } catch (...) {
            return false;
        }
    } else {
        row.mfe_energy = std::numeric_limits<double>::quiet_NaN();
    }

    return true;
}

bool load_baseline(const std::string &path, std::vector<BaselineRow> &rows, std::string &error) {
    std::ifstream input(path);
    if (!input.is_open()) {
        error = "unable to open baseline TSV: " + path;
        return false;
    }

    std::string line;
    int line_no = 0;
    bool saw_header = false;
    while (std::getline(input, line)) {
        ++line_no;
        if (line.empty()) {
            continue;
        }
        if (!saw_header) {
            saw_header = true;
            continue;
        }
        BaselineRow row{};
        if (!parse_baseline_row(line, row)) {
            error = "invalid baseline row at line " + std::to_string(line_no);
            return false;
        }
        rows.push_back(row);
    }

    if (!saw_header || rows.empty()) {
        error = "baseline TSV has no data rows: " + path;
        return false;
    }

    return true;
}

bool write_report(const std::string &path,
                  const std::vector<ComparisonResult> &comparisons,
                  const std::vector<SkippedRow> &skipped,
                  std::size_t mismatched,
                  std::string &error) {
    std::ofstream out(path);
    if (!out.is_open()) {
        error = "unable to write report file: " + path;
        return false;
    }

    out << "# Fixed Energy CLI Alignment Report\n\n";
    out << "Baseline source: `tests/baselines/fixed_energy_cli/density2_mfe.tsv`\n\n";
    out << "## Summary\n\n";
    out << "- compared: " << comparisons.size() << "\n";
    out << "- skipped: " << skipped.size() << "\n";
    out << "- mismatched: " << mismatched << "\n";
    out << "- abs_tol: " << kAbsTol << "\n";
    out << "- rel_tol: " << kRelTol << "\n\n";
    out << "## Compared Rows (`status=ok` and `exit_code=0`)\n\n";
    out << "| fixture_id | flag_id | flags | cli_mfe_energy | api_structure_energy | within_tolerance |\n";
    out << "| --- | --- | --- | --- | --- | --- |\n";
    for (const ComparisonResult &result : comparisons) {
        out << "| " << result.fixture_id << " | " << result.flag_id << " | " << result.flags << " | " << result.cli_energy << " | "
            << result.api_energy << " | " << (result.within_tolerance ? "yes" : "no") << " |\n";
    }

    out << "\n## Skipped Rows (nonzero CLI exit)\n\n";
    out << "| fixture_id | flag_id | flags | exit_code | reason |\n";
    out << "| --- | --- | --- | --- | --- |\n";
    for (const SkippedRow &row : skipped) {
        out << "| " << row.fixture_id << " | " << row.flag_id << " | " << row.flags << " | " << row.exit_code << " | " << row.reason
            << " |\n";
    }

    return true;
}

} // namespace

int main(int argc, char **argv) {
    if (argc != 4) {
        std::cerr << "usage: " << argv[0] << " <density2_mfe.tsv> <fixture_multi_fa> <report_md>" << std::endl;
        return 2;
    }

    std::unordered_map<std::string, std::string> seq_by_id;
    std::vector<BaselineRow> rows;
    std::string error;

    if (!load_fixture_sequences(argv[2], seq_by_id, error)) {
        std::cerr << error << std::endl;
        return 1;
    }
    if (!load_baseline(argv[1], rows, error)) {
        std::cerr << error << std::endl;
        return 1;
    }

    std::vector<ComparisonResult> comparisons;
    std::vector<SkippedRow> skipped;
    comparisons.reserve(rows.size());
    skipped.reserve(rows.size());

    std::size_t failed = 0;
    for (const BaselineRow &row : rows) {
        if (row.exit_code != 0) {
            skipped.push_back(
                SkippedRow{row.fixture_id, row.flag_id, row.flags, row.exit_code, "cli_exit_nonzero_status_" + row.status});
            continue;
        }

        if (row.status != "ok") {
            std::cerr << row.fixture_id << "/" << row.flag_id
                      << ": expected status=ok when exit_code=0, got status=" << row.status << std::endl;
            ++failed;
            continue;
        }

        const auto seq_it = seq_by_id.find(row.fixture_id);
        if (seq_it == seq_by_id.end()) {
            std::cerr << row.fixture_id << "/" << row.flag_id << ": fixture_id missing from fixture source file" << std::endl;
            ++failed;
            continue;
        }
        if (row.mfe_structure.empty()) {
            std::cerr << row.fixture_id << "/" << row.flag_id << ": missing mfe_structure for status=ok row" << std::endl;
            ++failed;
            continue;
        }

        const cparty::EnergyEvalOptions options = options_from_flags(row.flags);
        const double api_energy = get_structure_energy(seq_it->second, row.mfe_structure, options);
        if (!std::isfinite(api_energy)) {
            std::cerr << row.fixture_id << "/" << row.flag_id
                      << ": expected finite get_structure_energy for status=ok row, got " << api_energy << std::endl;
            ++failed;
            continue;
        }

        const bool within = approximately_equal(api_energy, row.mfe_energy);
        comparisons.push_back(
            ComparisonResult{row.fixture_id, row.flag_id, row.flags, row.mfe_energy, api_energy, within});
        if (!within) {
            std::cerr << row.fixture_id << "/" << row.flag_id << ": energy mismatch cli=" << row.mfe_energy << " api=" << api_energy
                      << std::endl;
        }
    }

    if (comparisons.empty()) {
        std::cerr << "api_cli_density2_energy_alignment_test: no comparable rows found" << std::endl;
        return 1;
    }

    if (failed != 0) {
        std::cerr << "api_cli_density2_energy_alignment_test: failed=" << failed << " compared=" << comparisons.size()
                  << " skipped=" << skipped.size() << std::endl;
        return 1;
    }

    std::size_t mismatched = 0;
    for (const ComparisonResult &result : comparisons) {
        if (!result.within_tolerance) {
            ++mismatched;
        }
    }

    if (!write_report(argv[3], comparisons, skipped, mismatched, error)) {
        std::cerr << error << std::endl;
        return 1;
    }

    if (mismatched != 0) {
        std::cerr << "api_cli_density2_energy_alignment_test: compared=" << comparisons.size()
                  << " skipped=" << skipped.size() << " mismatched=" << mismatched << " abs_tol=" << kAbsTol
                  << " rel_tol=" << kRelTol << std::endl;
        return 1;
    }

    std::cout << "api_cli_density2_energy_alignment_test: compared=" << comparisons.size() << " skipped=" << skipped.size()
              << " mismatched=" << mismatched << " abs_tol=" << kAbsTol << " rel_tol=" << kRelTol << std::endl;
    return 0;
}
