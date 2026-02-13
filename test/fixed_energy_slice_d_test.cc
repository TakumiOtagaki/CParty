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

void expect_contains_state(const std::vector<cparty::internal::RuleTraceStep> &trace,
                           const std::string &state) {
  for (const auto &step : trace) {
    if (step.state == state) {
      return;
    }
  }
  expect(false, "trace missing state: " + state);
}

void expect_contains_rule(const std::vector<cparty::internal::RuleTraceStep> &trace,
                          const std::string &rule) {
  for (const auto &step : trace) {
    if (step.rule == rule) {
      return;
    }
  }
  expect(false, "trace missing rule: " + rule);
}

void expect_invalid(const std::string &seq, const std::string &db_full) {
  bool threw = false;
  try {
    (void)cparty::internal::trace_rule_chain_slice_d(seq, db_full);
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
  {
    const auto trace = cparty::internal::trace_rule_chain_slice_d("AUGCUA", "((..))");
    expect_contains_state(trace, "WMB");
    expect_contains_state(trace, "WMBP");
    expect_contains_state(trace, "WMBW");
    expect_contains_state(trace, "BE");
    expect_contains_rule(trace, "WMp_TO_WMB");
    expect_contains_rule(trace, "WMB_TO_WMBP");
    expect_contains_rule(trace, "WMBP_TO_WMBW");
    expect_contains_rule(trace, "WMBW_TO_BE");
    expect_contains_rule(trace, "BE_TO_WIP");
  }

  expect(cparty::get_structure_energy("AUGCUA", "((..))") == -2.0,
         "slice-d shared path energy must preserve pair-wrapped contribution");
  expect(cparty::get_structure_energy("AUGCUA", "......") == 0.0,
         "slice-d shared path energy must stay zero for fully-unpaired");

  // Balanced but not representable by wrapped/unpaired recursion.
  expect_invalid("AUGC", "()()");

  std::cout << "fixed_energy_slice_d=ok\n";
  return EXIT_SUCCESS;
}
