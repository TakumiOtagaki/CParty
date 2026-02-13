#include "fixed_energy_api.hh"

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

void expect_trace_equals(const std::string &seq,
                         const std::string &db_full,
                         const std::vector<std::string> &expected_rules) {
  const auto trace = cparty::internal::trace_rule_chain_zw_only(seq, db_full);
  if (trace.size() != expected_rules.size()) {
    std::cerr << "unexpected trace length for db_full=" << db_full
              << ": expected=" << expected_rules.size() << " actual=" << trace.size() << "\n";
    std::exit(EXIT_FAILURE);
  }

  for (size_t idx = 0; idx < expected_rules.size(); ++idx) {
    if (trace[idx].state != "ZW") {
      std::cerr << "unexpected state at step " << idx << ": " << trace[idx].state << "\n";
      std::exit(EXIT_FAILURE);
    }
    if (trace[idx].rule != expected_rules[idx]) {
      std::cerr << "unexpected rule at step " << idx
                << ": expected=" << expected_rules[idx]
                << " actual=" << trace[idx].rule << "\n";
      std::exit(EXIT_FAILURE);
    }
  }
}

void expect_invalid(const std::string &seq, const std::string &db_full) {
  bool threw = false;
  try {
    (void)cparty::internal::trace_rule_chain_zw_only(seq, db_full);
  } catch (const std::invalid_argument &) {
    threw = true;
  } catch (const std::exception &e) {
    std::cerr << "expected invalid_argument, got: " << e.what() << "\n";
    std::exit(EXIT_FAILURE);
  }

  if (!threw) {
    std::cerr << "expected invalid_argument for seq=" << seq << " db_full=" << db_full << "\n";
    std::exit(EXIT_FAILURE);
  }
}

}  // namespace

int main() {
  expect_trace_equals("AUGCUA", "((..))",
                      {"ZW_PAIR_WRAPPED", "ZW_PAIR_WRAPPED", "ZW_UNPAIRED",
                       "ZW_UNPAIRED", "ZW_EMPTY"});
  expect_trace_equals("AUGCUA", "......",
                      {"ZW_UNPAIRED", "ZW_UNPAIRED", "ZW_UNPAIRED",
                       "ZW_UNPAIRED", "ZW_UNPAIRED", "ZW_UNPAIRED", "ZW_EMPTY"});

  // This balanced structure is valid dot-bracket, but not representable by
  // the current ZW-only wrapped/unpaired scaffold and must fail deterministically.
  expect_invalid("AUGC", "()()");

  std::cout << "fixed_energy_internal=ok\n";
  return EXIT_SUCCESS;
}
