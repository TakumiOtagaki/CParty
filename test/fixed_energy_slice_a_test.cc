#include "fixed_energy_api.hh"

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

void expect(const bool cond, const std::string &message) {
  if (!cond) {
    std::cerr << "FAILED: " << message << "\n";
    std::exit(EXIT_FAILURE);
  }
}

void expect_trace_equals(const std::string &seq,
                         const std::string &db_full,
                         const std::vector<std::string> &expected_states,
                         const std::vector<std::string> &expected_rules) {
  const auto trace = cparty::internal::trace_rule_chain_slice_a(seq, db_full);
  expect(trace.size() == expected_states.size(), "unexpected trace length");
  expect(trace.size() == expected_rules.size(), "unexpected expected rule length");

  for (size_t i = 0; i < trace.size(); ++i) {
    expect(trace[i].state == expected_states[i],
           "state mismatch at step " + std::to_string(i) + ": expected " + expected_states[i] +
               ", got " + trace[i].state);
    expect(trace[i].rule == expected_rules[i],
           "rule mismatch at step " + std::to_string(i) + ": expected " + expected_rules[i] +
               ", got " + trace[i].rule);
  }
}

void expect_invalid(const std::string &seq, const std::string &db_full) {
  bool threw = false;
  try {
    (void)cparty::internal::trace_rule_chain_slice_a(seq, db_full);
  } catch (const std::invalid_argument &) {
    threw = true;
  } catch (const std::exception &e) {
    std::cerr << "expected invalid_argument, got: " << e.what() << "\n";
    std::exit(EXIT_FAILURE);
  }
  expect(threw, "expected invalid_argument");
}

}  // namespace

int main() {
  expect_trace_equals("AUGCUA", "((..))",
                      {"W", "WI", "V", "V", "V", "V", "V"},
                      {"W_TO_WI", "WI_TO_V", "V_PAIR_WRAPPED", "V_PAIR_WRAPPED",
                       "V_UNPAIRED", "V_UNPAIRED", "V_EMPTY"});
  expect_trace_equals("AUGCUA", "......",
                      {"W", "WI", "V", "V", "V", "V", "V", "V", "V"},
                      {"W_TO_WI", "WI_TO_V", "V_UNPAIRED", "V_UNPAIRED", "V_UNPAIRED",
                       "V_UNPAIRED", "V_UNPAIRED", "V_UNPAIRED", "V_EMPTY"});

  expect(cparty::get_structure_energy("AUGCUA", "((..))") == -2.0,
         "shared path energy must match pair-wrapped count");
  expect(cparty::get_structure_energy("AUGCUA", "......") == 0.0,
         "shared path energy must stay zero for fully-unpaired");

  // Balanced but not representable by V-only wrapped/unpaired recursion.
  expect_invalid("AUGC", "()()");

  std::cout << "fixed_energy_slice_a=ok\n";
  return EXIT_SUCCESS;
}
