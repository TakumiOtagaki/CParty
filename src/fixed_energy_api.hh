#ifndef CPARTY_FIXED_ENERGY_API_HH
#define CPARTY_FIXED_ENERGY_API_HH

#include <cstddef>
#include <string>
#include <vector>

namespace cparty {

// Evaluates a fixed-structure energy path using deterministic SCFG parser rule selection.
double get_structure_energy(const std::string &seq, const std::string &db_full);

namespace fixed_energy_internal {

struct ParseTraceStep {
  std::string state;
  std::size_t i;
  std::size_t j;
  std::string rule;
};

std::vector<ParseTraceStep> trace_rule_chain(const std::string &seq, const std::string &db_full);

}  // namespace fixed_energy_internal

}  // namespace cparty

#endif  // CPARTY_FIXED_ENERGY_API_HH
