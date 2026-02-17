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
  const auto rules_core_trace = cparty::internal::trace_rule_chain_slice_a_rules_core(seq, db_full);
  expect(trace.size() == expected_states.size(), "unexpected trace length");
  expect(trace.size() == expected_rules.size(), "unexpected expected rule length");
  expect(rules_core_trace.size() == trace.size(), "rules_core trace length mismatch");

  for (size_t i = 0; i < trace.size(); ++i) {
    expect(trace[i].state == expected_states[i],
           "state mismatch at step " + std::to_string(i) + ": expected " + expected_states[i] +
               ", got " + trace[i].state);
    expect(trace[i].rule == expected_rules[i],
           "rule mismatch at step " + std::to_string(i) + ": expected " + expected_rules[i] +
               ", got " + trace[i].rule);
    expect(rules_core_trace[i].state == trace[i].state,
           "rules_core state mismatch at step " + std::to_string(i));
    expect(rules_core_trace[i].rule == trace[i].rule,
           "rules_core rule mismatch at step " + std::to_string(i));
  }
}

}  // namespace

int main() {
  expect_trace_equals("AUGCUA", "((..))",
                      {"W", "V"},
                      {"W_SPLIT_V", "V_INTERNAL"});
  expect_trace_equals("AUGCUA", "[[..]]",
                      {"W", "V"},
                      {"W_SPLIT_V", "V_INTERNAL"});
  expect_trace_equals("AUGCUA", "......",
                      {"W", "W", "W", "W", "W", "W"},
                      {"W_EXTEND_UNPAIRED", "W_EXTEND_UNPAIRED", "W_EXTEND_UNPAIRED",
                       "W_EXTEND_UNPAIRED", "W_EXTEND_UNPAIRED", "W_EXTEND_UNPAIRED"});

  expect(cparty::get_structure_energy("AUGCUA", "((..))") == -2.0,
         "shared path energy must match pair-wrapped count");
  expect(cparty::get_structure_energy("AUGCUA", "......") == 0.0,
         "shared path energy must stay zero for fully-unpaired");

  std::cout << "fixed_energy_slice_a=ok\n";
  return EXIT_SUCCESS;
}
