#include "fixed_energy_api.hh"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

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
  if (argc != 2) {
    std::cerr << "usage: fixed_energy_cli_mfe_energy_alignment_test <baseline_tsv>\n";
    return EXIT_FAILURE;
  }

  try {
    const std::string baseline_path = argv[1];
    const auto baseline = load_baseline(baseline_path);

    int compared = 0;
    int mismatched = 0;
    int skipped = 0;
    int finite_count = 0;
    double max_abs_diff = 0.0;
    double max_rel_diff = 0.0;

    for (const auto &row : baseline) {
      double api_energy = 0.0;
      try {
        api_energy = cparty::get_structure_energy(row.seq, row.cli_mfe_structure);
      } catch (const std::invalid_argument &) {
        ++skipped;
        continue;
      }
      const double cli_energy = row.cli_mfe_energy;
      const double abs_diff = std::fabs(cli_energy - api_energy);
      const double scale = std::max(std::fabs(cli_energy), std::fabs(api_energy));
      const double rel_diff = (scale == 0.0) ? 0.0 : (abs_diff / scale);
      max_abs_diff = std::max(max_abs_diff, abs_diff);
      max_rel_diff = std::max(max_rel_diff, rel_diff);

      if (std::isfinite(cli_energy) && std::isfinite(api_energy)) {
        ++finite_count;
      }

      ++compared;
      if (!almost_equal(cli_energy, api_energy)) {
        ++mismatched;
      }
    }

    const double finite_rate = (compared == 0)
                                   ? 0.0
                                   : (static_cast<double>(finite_count) * 100.0) / compared;

    std::cout << "fixed_energy_compared=" << compared << "\n";
    std::cout << "fixed_energy_mismatched=" << mismatched << "\n";
    std::cout << "skipped=" << skipped << "\n";
    std::cout << "finite_rate=" << finite_rate << "%\n";
    std::cout << "max_abs_diff=" << max_abs_diff << "\n";
    std::cout << "max_rel_diff=" << max_rel_diff << "\n";

    if (compared < 100) {
      std::cerr << "fixed energy gate failed: compared=" << compared << " (<100)\n";
      return EXIT_FAILURE;
    }
    if (mismatched != 0) {
      std::cerr << "fixed energy gate failed: mismatched=" << mismatched << " (expected 0)\n";
      return EXIT_FAILURE;
    }
    if (finite_rate < 100.0) {
      std::cerr << "fixed energy gate failed: finite_rate=" << finite_rate << "% (expected 100%)\n";
      return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
  } catch (const std::exception &e) {
    std::cerr << "fixed_energy_cli_mfe_energy_alignment_test failed: " << e.what() << "\n";
    return EXIT_FAILURE;
  }
}
