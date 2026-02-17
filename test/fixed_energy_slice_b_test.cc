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

void expect_missing_state(const std::vector<cparty::internal::RuleTraceStep> &trace,
                          const std::string &state) {
  for (const auto &step : trace) {
    if (step.state == state) {
      expect(false, "trace unexpectedly contains state: " + state);
    }
  }
}

void expect_rules_present(const std::vector<cparty::internal::RuleTraceStep> &trace) {
  for (size_t i = 0; i < trace.size(); ++i) {
    if (trace[i].rule.empty()) {
      expect(false, "trace rule missing at step " + std::to_string(i));
    }
  }
}

void expect_invalid(const std::string &seq, const std::string &db_full) {
  bool threw = false;
  try {
    (void)cparty::internal::trace_rule_chain_slice_b(seq, db_full);
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
    const auto trace = cparty::internal::trace_rule_chain_slice_b("AUGCUA", "((..))");
    const auto rules_core_trace =
        cparty::internal::trace_rule_chain_slice_b_rules_core("AUGCUA", "((..))");
    expect_contains_state(trace, "VM");
    expect_contains_state(trace, "WM");
    expect_contains_state(trace, "WMv");
    expect_contains_state(trace, "WMp");
    expect_missing_state(trace, "WIP");
    expect_rules_present(trace);
    expect(rules_core_trace.size() == trace.size(), "rules_core trace length mismatch");
    for (size_t i = 0; i < rules_core_trace.size(); ++i) {
      expect(rules_core_trace[i].state == trace[i].state,
             "rules_core state mismatch at step " + std::to_string(i));
      expect(!rules_core_trace[i].rule.empty(),
             "rules_core rule missing at step " + std::to_string(i));
    }
  }
  {
    const auto trace = cparty::internal::trace_rule_chain_slice_b("AUGCUA", "[[..]]");
    const auto rules_core_trace =
        cparty::internal::trace_rule_chain_slice_b_rules_core("AUGCUA", "[[..]]");
    expect_contains_state(trace, "WM");
    expect_contains_state(trace, "WMv");
    expect_contains_state(trace, "WMp");
    expect_rules_present(trace);
    expect(rules_core_trace.size() == trace.size(), "rules_core trace length mismatch");
    for (size_t i = 0; i < rules_core_trace.size(); ++i) {
      expect(rules_core_trace[i].state == trace[i].state,
             "rules_core state mismatch at step " + std::to_string(i));
      expect(!rules_core_trace[i].rule.empty(),
             "rules_core rule missing at step " + std::to_string(i));
    }
  }

  expect(cparty::get_structure_energy("AUGCUA", "((..))") == -2.0,
         "slice-b shared path energy must preserve pair-wrapped contribution");
  expect(cparty::get_structure_energy("AUGCUA", "......") == 0.0,
         "slice-b shared path energy must stay zero for fully-unpaired");

  // Balanced but not representable by wrapped/unpaired recursion.
  expect_invalid("AUGC", "()()");

  std::cout << "fixed_energy_slice_b=ok\n";
  return EXIT_SUCCESS;
}
