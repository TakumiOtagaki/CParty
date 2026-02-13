#include "fixed_energy_api.hh"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

void expect_invalid_argument(const std::string &seq, const std::string &db_full) {
  bool threw = false;
  try {
    (void)cparty::get_structure_energy(seq, db_full);
  } catch (const std::invalid_argument &) {
    threw = true;
  } catch (const std::exception &e) {
    std::cerr << "expected std::invalid_argument, got different exception: " << e.what() << "\n";
    std::exit(EXIT_FAILURE);
  }

  if (!threw) {
    std::cerr << "expected invalid_argument for seq='" << seq << "' db='" << db_full << "'\n";
    std::exit(EXIT_FAILURE);
  }
}

void expect_trace_equal(const std::vector<cparty::fixed_energy_internal::ParseTraceStep> &lhs,
                        const std::vector<cparty::fixed_energy_internal::ParseTraceStep> &rhs) {
  if (lhs.size() != rhs.size()) {
    std::cerr << "trace size mismatch: " << lhs.size() << " vs " << rhs.size() << "\n";
    std::exit(EXIT_FAILURE);
  }

  for (size_t i = 0; i < lhs.size(); ++i) {
    const auto &a = lhs[i];
    const auto &b = rhs[i];
    if (a.state != b.state || a.i != b.i || a.j != b.j || a.rule != b.rule) {
      std::cerr << "trace mismatch at step " << i << "\n";
      std::exit(EXIT_FAILURE);
    }
  }
}

}  // namespace

int main() {
  const std::string seq = "AUGCUA";
  const std::string db = "((..))";

  const auto trace1 = cparty::fixed_energy_internal::trace_rule_chain(seq, db);
  const auto trace2 = cparty::fixed_energy_internal::trace_rule_chain(seq, db);
  expect_trace_equal(trace1, trace2);

  if (trace1.empty()) {
    std::cerr << "expected non-empty parse trace\n";
    return EXIT_FAILURE;
  }

  size_t paired_count = 0;
  for (const auto &step : trace1) {
    if (step.state != "ZW") {
      std::cerr << "unexpected parser state in trace: " << step.state << "\n";
      return EXIT_FAILURE;
    }
    if (step.rule == "ZW -> '(' ZW ')' ZW") {
      ++paired_count;
    }
  }

  if (paired_count != 2) {
    std::cerr << "expected 2 paired rules for ((..)), got " << paired_count << "\n";
    return EXIT_FAILURE;
  }

  const double energy = cparty::get_structure_energy(seq, db);
  if (!std::isfinite(energy)) {
    std::cerr << "expected finite energy for valid input\n";
    return EXIT_FAILURE;
  }
  if (energy != -2.0) {
    std::cerr << "unexpected parser score for valid input: " << energy << "\n";
    return EXIT_FAILURE;
  }

  expect_invalid_argument("AUGCUA", "((..)");
  expect_invalid_argument("AUTCUA", "((..))");
  expect_invalid_argument("AUGCUA", "(()())..");

  std::cout << "fixed_energy_internal=ok\n";
  return EXIT_SUCCESS;
}
