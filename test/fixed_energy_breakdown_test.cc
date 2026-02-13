#include "fixed_energy_api.hh"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>

namespace {

void expect(const bool cond, const std::string &message) {
  if (!cond) {
    std::cerr << "FAILED: " << message << "\n";
    std::exit(EXIT_FAILURE);
  }
}

void expect_close(const double actual, const double expected, const std::string &message) {
  if (std::fabs(actual - expected) > 1e-9) {
    std::cerr << "FAILED: " << message << " expected=" << expected << " actual=" << actual << "\n";
    std::exit(EXIT_FAILURE);
  }
}

void expect_breakdown_smoke(const std::string &seq,
                            const std::string &db_full,
                            const std::string &expected_family,
                            const double expected_energy) {
  const auto breakdown = cparty::internal::get_structure_energy_breakdown(seq, db_full);
  expect(breakdown.topology_family == expected_family, "unexpected topology family");
  expect_close(breakdown.total_energy, expected_energy, "unexpected total_energy");
  expect(breakdown.rule_evaluated_count > 0, "rule_evaluated_count must be populated");
  expect(breakdown.rule_evaluated_count ==
             breakdown.empty_rule_count + breakdown.unpaired_rule_count +
                 breakdown.pair_wrapped_rule_count + breakdown.transition_rule_count,
         "rule counts must cover all evaluated rules");
  expect(breakdown.pair_wrapped_rule_count >= 1, "pair_wrapped_rule_count should be populated");

  if (expected_family == "pk_free") {
    expect(breakdown.family_pk_free_rules == breakdown.rule_evaluated_count,
           "pk_free family counter should match rule count");
    expect(breakdown.family_h_type_rules == 0, "h_type family counter should be zero");
    expect(breakdown.family_k_type_rules == 0, "k_type family counter should be zero");
  } else if (expected_family == "h_type") {
    expect(breakdown.family_h_type_rules == breakdown.rule_evaluated_count,
           "h_type family counter should match rule count");
    expect(breakdown.family_pk_free_rules == 0, "pk_free family counter should be zero");
    expect(breakdown.family_k_type_rules == 0, "k_type family counter should be zero");
  } else if (expected_family == "k_type") {
    expect(breakdown.family_k_type_rules == breakdown.rule_evaluated_count,
           "k_type family counter should match rule count");
    expect(breakdown.family_pk_free_rules == 0, "pk_free family counter should be zero");
    expect(breakdown.family_h_type_rules == 0, "h_type family counter should be zero");
  }
}

}  // namespace

int main() {
  expect_breakdown_smoke("AUGCUA", "((..))", "pk_free", -2.0);
  expect_breakdown_smoke("AUGCUA", "[[..]]", "h_type", -2.0);
  expect_breakdown_smoke("AUGCUA", "([..])", "k_type", -2.0);

  expect_close(cparty::get_structure_energy("AUGCUA", "((..))"), -2.0,
               "api energy should remain aligned with shared route scoring");

  std::cout << "fixed_energy_breakdown=ok\n";
  return EXIT_SUCCESS;
}
