#ifndef CPARTY_FIXED_ENERGY_BREAKDOWN_HH
#define CPARTY_FIXED_ENERGY_BREAKDOWN_HH

#include <string>

namespace cparty {

struct EnergyBreakdown {
  std::string topology_family = "unknown";
  int rule_evaluated_count = 0;
  int empty_rule_count = 0;
  int unpaired_rule_count = 0;
  int pair_wrapped_rule_count = 0;
  int transition_rule_count = 0;
  int family_pk_free_rules = 0;
  int family_h_type_rules = 0;
  int family_k_type_rules = 0;
  double total_energy = 0.0;
};

}  // namespace cparty

#endif  // CPARTY_FIXED_ENERGY_BREAKDOWN_HH
