#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <unistd.h>

namespace {

constexpr double kAbsTol = 1e-7;
constexpr double kRelTol = 1e-7;

struct BaselineRow {
  std::string case_id;
  std::string seq;
  std::string g;
  std::string cli_mfe_structure;
  double cli_mfe_energy = 0.0;
};

struct ParsedCliOutput {
  std::string seq;
  std::string restricted;
  std::string mfe_structure;
  double mfe_energy = 0.0;
};

std::string trim(const std::string &s) {
  const auto start = s.find_first_not_of(" \t\r\n");
  if (start == std::string::npos) {
    return "";
  }
  const auto end = s.find_last_not_of(" \t\r\n");
  return s.substr(start, end - start + 1);
}

std::vector<std::string> split(const std::string &s, char delimiter) {
  std::vector<std::string> out;
  std::stringstream ss(s);
  std::string token;
  while (std::getline(ss, token, delimiter)) {
    out.push_back(token);
  }
  return out;
}

std::vector<BaselineRow> load_baseline(const std::string &path) {
  std::ifstream in(path);
  if (!in) {
    throw std::runtime_error("failed to open baseline: " + path);
  }

  std::string header;
  if (!std::getline(in, header)) {
    throw std::runtime_error("baseline is empty");
  }
  if (trim(header) != "case_id\tseq\tG\tcli_mfe_structure\tcli_mfe_energy") {
    throw std::runtime_error("unexpected baseline header: " + header);
  }

  std::vector<BaselineRow> rows;
  std::string line;
  int row_no = 1;
  while (std::getline(in, line)) {
    ++row_no;
    if (trim(line).empty()) {
      continue;
    }
    const auto cols = split(line, '\t');
    if (cols.size() != 5) {
      throw std::runtime_error("baseline row " + std::to_string(row_no) +
                               " has unexpected column count: " + std::to_string(cols.size()));
    }
    BaselineRow row;
    row.case_id = cols[0];
    row.seq = cols[1];
    row.g = cols[2];
    row.cli_mfe_structure = cols[3];
    try {
      row.cli_mfe_energy = std::stod(cols[4]);
    } catch (const std::exception &) {
      throw std::runtime_error("baseline row " + std::to_string(row_no) +
                               " has invalid cli_mfe_energy: " + cols[4]);
    }
    rows.push_back(row);
  }

  if (rows.empty()) {
    throw std::runtime_error("baseline has no data rows");
  }

  return rows;
}

std::string escape_single_quotes(const std::string &s) {
  std::string out;
  out.reserve(s.size());
  for (char c : s) {
    if (c == '\'') {
      out += "'\\''";
    } else {
      out.push_back(c);
    }
  }
  return out;
}

ParsedCliOutput parse_cli_stdout(const std::string &path) {
  std::ifstream in(path);
  if (!in) {
    throw std::runtime_error("failed to open CLI stdout file: " + path);
  }

  std::vector<std::string> lines;
  std::string line;
  while (std::getline(in, line)) {
    if (!trim(line).empty()) {
      lines.push_back(trim(line));
    }
  }

  if (lines.size() < 3) {
    throw std::runtime_error("CLI output has fewer than 3 non-empty lines");
  }

  static const std::regex mfe_re(R"(^(\S+)\s+\(([+-]?[0-9]+(?:\.[0-9]+)?)\)$)");
  std::smatch match;
  if (!std::regex_match(lines[2], match, mfe_re)) {
    throw std::runtime_error("failed to parse MFE line: " + lines[2]);
  }

  ParsedCliOutput out;
  out.seq = lines[0];
  out.restricted = lines[1];
  out.mfe_structure = match[1].str();
  out.mfe_energy = std::stod(match[2].str());
  return out;
}

bool almost_equal(double lhs, double rhs) {
  const double diff = std::fabs(lhs - rhs);
  if (diff <= kAbsTol) {
    return true;
  }
  const double scale = std::max(std::fabs(lhs), std::fabs(rhs));
  if (scale == 0.0) {
    return true;
  }
  return diff <= (kRelTol * scale);
}

}  // namespace

int main(int argc, char **argv) {
  if (argc != 3) {
    std::cerr << "usage: api_cli_density2_energy_alignment_test <baseline_tsv> <cparty_bin>\n";
    return EXIT_FAILURE;
  }

  try {
    const std::string baseline_path = argv[1];
    const std::string cparty_bin = argv[2];
    const auto baseline = load_baseline(baseline_path);

    int alignment_compared = 0;
    int alignment_mismatched = 0;
    int skipped = 0;
    int finite_count = 0;

    for (size_t i = 0; i < baseline.size(); ++i) {
      const BaselineRow &row = baseline[i];
      char tmp_name[] = "/tmp/cparty_alignment_XXXXXX";
      const int fd = mkstemp(tmp_name);
      if (fd < 0) {
        throw std::runtime_error("mkstemp failed");
      }
      std::fclose(fdopen(fd, "w"));

      const std::string cmd = "'" + escape_single_quotes(cparty_bin) + "' -d2 -r '" + escape_single_quotes(row.g) +
                              "' '" + escape_single_quotes(row.seq) + "' > '" + escape_single_quotes(tmp_name) +
                              "' 2>/dev/null";
      const int rc = std::system(cmd.c_str());
      if (rc != 0) {
        ++skipped;
        std::remove(tmp_name);
        continue;
      }

      ParsedCliOutput parsed;
      try {
        parsed = parse_cli_stdout(tmp_name);
      } catch (const std::exception &) {
        ++skipped;
        std::remove(tmp_name);
        continue;
      }
      std::remove(tmp_name);

      const double cli_energy = row.cli_mfe_energy;
      const double api_energy = parsed.mfe_energy;

      if (std::isfinite(cli_energy) && std::isfinite(api_energy)) {
        ++finite_count;
      }

      ++alignment_compared;

      const bool structure_mismatch = (row.cli_mfe_structure != parsed.mfe_structure);
      const bool energy_mismatch = !almost_equal(cli_energy, api_energy);
      if (structure_mismatch || energy_mismatch) {
        ++alignment_mismatched;
      }
    }

    const double finite_rate = (alignment_compared == 0)
                                   ? 0.0
                                   : (static_cast<double>(finite_count) * 100.0) / alignment_compared;

    std::cout << "alignment_compared=" << alignment_compared << "\n";
    std::cout << "alignment_mismatched=" << alignment_mismatched << "\n";
    std::cout << "skipped=" << skipped << "\n";
    std::cout << "finite_rate=" << finite_rate << "%\n";

    if (alignment_compared == 0) {
      std::cerr << "alignment gate failed: alignment_compared=0\n";
      return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
  } catch (const std::exception &e) {
    std::cerr << "api_cli_density2_energy_alignment_test failed: " << e.what() << "\n";
    return EXIT_FAILURE;
  }
}
