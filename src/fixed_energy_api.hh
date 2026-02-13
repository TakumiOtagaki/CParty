#ifndef CPARTY_FIXED_ENERGY_API_HH
#define CPARTY_FIXED_ENERGY_API_HH

#include <string>
#include <vector>

namespace cparty {

// Evaluates a fixed-structure energy path. Story 011 exposes the callable API
// and strict validation contract; scoring internals are expanded in later stories.
double get_structure_energy(const std::string &seq, const std::string &db_full);

namespace internal {

struct RuleTraceStep {
  std::string state;
  int i = 0;
  int j = 0;
  std::string rule;
};

struct RolloutStatePlanEntry {
  std::string state;
  std::string story_id;
};

// Story 012 parser scaffold: deterministic ZW-only rule chain trace.
std::vector<RuleTraceStep> trace_rule_chain_zw_only(const std::string &seq,
                                                    const std::string &db_full);

// Story 014 slice-A shared parser trace: W/WI/V states on shared DP path.
std::vector<RuleTraceStep> trace_rule_chain_slice_a(const std::string &seq,
                                                    const std::string &db_full);

// Story 015 slice-B shared parser trace: VM/WM/WMv/WMp are traversed on pair paths.
std::vector<RuleTraceStep> trace_rule_chain_slice_b(const std::string &seq,
                                                    const std::string &db_full);

// Story 016 slice-C shared parser trace: WIP/VP/VPL/VPR are traversed on pair paths.
std::vector<RuleTraceStep> trace_rule_chain_slice_c(const std::string &seq,
                                                    const std::string &db_full);

// Story 013 registry: canonical fixed-energy target states and rollout mapping.
const std::vector<std::string> &fixed_energy_target_states();
const std::vector<RolloutStatePlanEntry> &fixed_energy_rollout_plan();

}  // namespace internal

}  // namespace cparty

#endif  // CPARTY_FIXED_ENERGY_API_HH
