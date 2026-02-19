#ifndef CPARTY_FIXED_ENERGY_SHARED_PARSE_HH
#define CPARTY_FIXED_ENERGY_SHARED_PARSE_HH

#include "fixed_energy_breakdown.hh"
#include "fixed_energy_input.hh"

#include <string>
#include <vector>

namespace cparty {

namespace internal {
struct RuleTraceStep;
}

enum class SharedStateKind {
  kW,
  kWI,
  kV,
  kVM,
  kWM,
  kWMv,
  kWMp,
  kWMB,
  kWMBP,
  kWMBW,
  kBE,
  kWIP,
  kVP,
  kVPL,
  kVPR,
};

enum class SharedRuleKind {
  kWToWI,
  kWIToV,
  kEmpty,
  kUnpaired,
  kPairWrapped,
  kVMToWM,
  kWMToWMv,
  kWMvToWMp,
  kWMpToV,
  kWMpToWMB,
  kWMBToWMBP,
  kWMBPToWMBW,
  kWMBWToBE,
  kBEToWIP,
  kWMpToWIP,
  kWIPToVP,
  kVPToVPL,
  kVPLToVPR,
  kVPRToV,
};

struct SharedParseMode {
  bool include_slice_b = false;
  bool include_slice_c = false;
  bool include_slice_d = false;
};

struct SharedState {
  SharedStateKind kind = SharedStateKind::kW;
  int i = 1;  // 1-based inclusive
  int j = 0;  // 1-based inclusive
};

struct SharedEvaluationResult {
  std::vector<internal::RuleTraceStep> trace;
  EnergyBreakdown breakdown;
};

double shared_rule_score(SharedRuleKind rule);

SharedEvaluationResult evaluate_shared_from_normalized(const NormalizedInput &ctx, SharedParseMode mode);

std::vector<internal::RuleTraceStep> trace_rule_chain_slice_a_shared_from_normalized(const NormalizedInput &ctx);
std::vector<internal::RuleTraceStep> trace_rule_chain_slice_b_shared_from_normalized(const NormalizedInput &ctx);
std::vector<internal::RuleTraceStep> trace_rule_chain_slice_c_shared_from_normalized(const NormalizedInput &ctx);
std::vector<internal::RuleTraceStep> trace_rule_chain_slice_d_shared_from_normalized(const NormalizedInput &ctx);
std::string topology_family_for_structure(const std::string &db_full);

}  // namespace cparty

#endif  // CPARTY_FIXED_ENERGY_SHARED_PARSE_HH
