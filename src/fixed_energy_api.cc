#include "fixed_energy_api.hh"

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

namespace cparty {
namespace {

struct NormalizedInput {
  std::string seq;
  std::string db_full;
  std::vector<int> pair_map;
};

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

[[noreturn]] void fail_invalid_input(const std::string &reason) {
  throw std::invalid_argument("invalid fixed-structure input: " + reason);
}

void validate_sequence(const std::string &seq) {
  if (seq.empty()) {
    fail_invalid_input("sequence is empty");
  }

  for (size_t i = 0; i < seq.size(); ++i) {
    const char c = seq[i];
    if (c == 'A' || c == 'U' || c == 'G' || c == 'C') {
      continue;
    }
    if (c == 'T') {
      fail_invalid_input("sequence contains T at position " + std::to_string(i + 1));
    }
    fail_invalid_input("sequence contains non-AUGC base at position " + std::to_string(i + 1));
  }
}

void validate_structure(const std::string &db_full, const size_t expected_length) {
  if (db_full.empty()) {
    fail_invalid_input("structure is empty");
  }
  if (db_full.size() != expected_length) {
    fail_invalid_input("sequence/structure length mismatch");
  }

  std::vector<size_t> round_stack;
  std::vector<size_t> square_stack;
  round_stack.reserve(db_full.size());
  square_stack.reserve(db_full.size());

  for (size_t i = 0; i < db_full.size(); ++i) {
    const char c = db_full[i];
    if (c == '.') {
      continue;
    }
    if (c == '(') {
      round_stack.push_back(i);
      continue;
    }
    if (c == '[') {
      square_stack.push_back(i);
      continue;
    }
    if (c == ')') {
      if (round_stack.empty()) {
        fail_invalid_input("unbalanced structure: closing bracket without opener");
      }
      round_stack.pop_back();
      continue;
    }
    if (c == ']') {
      if (square_stack.empty()) {
        fail_invalid_input("unbalanced structure: closing bracket without opener");
      }
      square_stack.pop_back();
      continue;
    }
    fail_invalid_input("structure contains unsupported symbol at position " + std::to_string(i + 1));
  }

  if (!round_stack.empty() || !square_stack.empty()) {
    fail_invalid_input("unbalanced structure: missing closing bracket");
  }
}

NormalizedInput normalize_input(const std::string &seq, const std::string &db_full) {
  validate_sequence(seq);
  validate_structure(db_full, seq.size());

  NormalizedInput out;
  out.seq = seq;
  out.db_full = db_full;
  out.pair_map.assign(static_cast<size_t>(db_full.size()), -1);

  std::vector<int> round_stack;
  std::vector<int> square_stack;
  round_stack.reserve(db_full.size());
  square_stack.reserve(db_full.size());
  for (size_t idx = 0; idx < db_full.size(); ++idx) {
    const char c = db_full[idx];
    if (c == '(') {
      round_stack.push_back(static_cast<int>(idx));
      continue;
    }
    if (c == '[') {
      square_stack.push_back(static_cast<int>(idx));
      continue;
    }
    if (c == ')') {
      const int left = round_stack.back();
      round_stack.pop_back();
      out.pair_map[static_cast<size_t>(left)] = static_cast<int>(idx);
      out.pair_map[idx] = left;
      continue;
    }
    if (c == ']') {
      const int left = square_stack.back();
      square_stack.pop_back();
      out.pair_map[static_cast<size_t>(left)] = static_cast<int>(idx);
      out.pair_map[idx] = left;
    }
  }
  return out;
}

std::vector<SharedRuleKind> rules_for(const SharedStateKind state_kind, const SharedParseMode mode) {
  if (state_kind == SharedStateKind::kW) {
    return {SharedRuleKind::kWToWI};
  }
  if (state_kind == SharedStateKind::kWI) {
    return {SharedRuleKind::kWIToV};
  }
  if (state_kind == SharedStateKind::kV) {
    if (mode.include_slice_b) {
      return {SharedRuleKind::kEmpty, SharedRuleKind::kUnpaired, SharedRuleKind::kPairWrapped};
    }
    return {SharedRuleKind::kEmpty, SharedRuleKind::kUnpaired, SharedRuleKind::kPairWrapped};
  }
  if (state_kind == SharedStateKind::kVM) {
    return {SharedRuleKind::kVMToWM};
  }
  if (state_kind == SharedStateKind::kWM) {
    return {SharedRuleKind::kWMToWMv};
  }
  if (state_kind == SharedStateKind::kWMv) {
    return {SharedRuleKind::kWMvToWMp};
  }
  if (state_kind == SharedStateKind::kWMp) {
    if (mode.include_slice_d) {
      return {SharedRuleKind::kWMpToWMB};
    }
    if (mode.include_slice_c) {
      return {SharedRuleKind::kWMpToWIP};
    }
    return {};
  }
  if (state_kind == SharedStateKind::kWMB) {
    return {SharedRuleKind::kWMBToWMBP};
  }
  if (state_kind == SharedStateKind::kWMBP) {
    return {SharedRuleKind::kWMBPToWMBW};
  }
  if (state_kind == SharedStateKind::kWMBW) {
    return {SharedRuleKind::kWMBWToBE};
  }
  if (state_kind == SharedStateKind::kBE) {
    return {SharedRuleKind::kBEToWIP};
  }
  if (state_kind == SharedStateKind::kWIP) {
    return {SharedRuleKind::kWIPToVP};
  }
  if (state_kind == SharedStateKind::kVP) {
    return {SharedRuleKind::kVPToVPL};
  }
  if (state_kind == SharedStateKind::kVPL) {
    return {SharedRuleKind::kVPLToVPR};
  }
  if (state_kind == SharedStateKind::kVPR) {
    return {SharedRuleKind::kVPRToV};
  }
  return {};
}

bool rule_is_applicable(const SharedRuleKind rule,
                        const SharedState state,
                        const NormalizedInput &ctx,
                        const SharedParseMode mode) {
  if (rule == SharedRuleKind::kWToWI || rule == SharedRuleKind::kWIToV) {
    return true;
  }

  if (rule == SharedRuleKind::kVMToWM || rule == SharedRuleKind::kWMToWMv || rule == SharedRuleKind::kWMvToWMp) {
    return mode.include_slice_b;
  }

  if (rule == SharedRuleKind::kWMpToWIP || rule == SharedRuleKind::kWIPToVP ||
      rule == SharedRuleKind::kVPToVPL || rule == SharedRuleKind::kVPLToVPR ||
      rule == SharedRuleKind::kVPRToV) {
    return mode.include_slice_c;
  }

  if (rule == SharedRuleKind::kWMpToWMB || rule == SharedRuleKind::kWMBToWMBP ||
      rule == SharedRuleKind::kWMBPToWMBW || rule == SharedRuleKind::kWMBWToBE ||
      rule == SharedRuleKind::kBEToWIP) {
    return mode.include_slice_d;
  }

  if (rule == SharedRuleKind::kEmpty) {
    return state.i > state.j;
  }
  if (state.i > state.j) {
    return false;
  }

  const size_t left = static_cast<size_t>(state.i - 1);
  if (rule == SharedRuleKind::kUnpaired) {
    return ctx.db_full[left] == '.';
  }
  if (rule == SharedRuleKind::kPairWrapped) {
    const char left_bracket = ctx.db_full[left];
    if (left_bracket != '(' && left_bracket != '[') {
      return false;
    }
    const int partner = ctx.pair_map[left];
    return partner == (state.j - 1);
  }
  return false;
}

std::vector<SharedRuleKind> applicable_rules(const SharedState state,
                                             const NormalizedInput &ctx,
                                             const SharedParseMode mode) {
  const auto candidates = rules_for(state.kind, mode);
  std::vector<SharedRuleKind> out;
  out.reserve(candidates.size());
  for (const SharedRuleKind rule : candidates) {
    if (rule_is_applicable(rule, state, ctx, mode)) {
      out.push_back(rule);
    }
  }
  return out;
}

double rule_score(const SharedRuleKind rule) {
  if (rule == SharedRuleKind::kPairWrapped) {
    return -1.0;
  }
  return 0.0;
}

std::string topology_family_for_structure(const std::string &db_full) {
  bool has_round = false;
  bool has_square = false;
  for (const char c : db_full) {
    has_round = has_round || c == '(' || c == ')';
    has_square = has_square || c == '[' || c == ']';
  }
  if (has_round && has_square) {
    return "k_type";
  }
  if (has_square) {
    return "h_type";
  }
  return "pk_free";
}

void accumulate_breakdown(const SharedRuleKind rule, EnergyBreakdown &breakdown) {
  ++breakdown.rule_evaluated_count;
  if (rule == SharedRuleKind::kEmpty) {
    ++breakdown.empty_rule_count;
  } else if (rule == SharedRuleKind::kUnpaired) {
    ++breakdown.unpaired_rule_count;
  } else if (rule == SharedRuleKind::kPairWrapped) {
    ++breakdown.pair_wrapped_rule_count;
  } else {
    ++breakdown.transition_rule_count;
  }

  breakdown.total_energy += rule_score(rule);
  if (breakdown.topology_family == "pk_free") {
    ++breakdown.family_pk_free_rules;
  } else if (breakdown.topology_family == "h_type") {
    ++breakdown.family_h_type_rules;
  } else if (breakdown.topology_family == "k_type") {
    ++breakdown.family_k_type_rules;
  }
}

std::vector<SharedState> expand(const SharedRuleKind rule,
                                const SharedState state,
                                const SharedParseMode mode) {
  if (rule == SharedRuleKind::kWToWI) {
    return {SharedState{SharedStateKind::kWI, state.i, state.j}};
  }
  if (rule == SharedRuleKind::kWIToV) {
    return {SharedState{SharedStateKind::kV, state.i, state.j}};
  }
  if (rule == SharedRuleKind::kUnpaired) {
    return {SharedState{SharedStateKind::kV, state.i + 1, state.j}};
  }
  if (rule == SharedRuleKind::kPairWrapped) {
    if (mode.include_slice_b) {
      return {SharedState{SharedStateKind::kVM, state.i + 1, state.j - 1}};
    }
    return {SharedState{SharedStateKind::kV, state.i + 1, state.j - 1}};
  }
  if (rule == SharedRuleKind::kVMToWM) {
    return {SharedState{SharedStateKind::kWM, state.i, state.j}};
  }
  if (rule == SharedRuleKind::kWMToWMv) {
    return {SharedState{SharedStateKind::kWMv, state.i, state.j}};
  }
  if (rule == SharedRuleKind::kWMvToWMp) {
    return {SharedState{SharedStateKind::kWMp, state.i, state.j}};
  }
  if (rule == SharedRuleKind::kWMpToWMB) {
    return {SharedState{SharedStateKind::kWMB, state.i, state.j}};
  }
  if (rule == SharedRuleKind::kWMBToWMBP) {
    return {SharedState{SharedStateKind::kWMBP, state.i, state.j}};
  }
  if (rule == SharedRuleKind::kWMBPToWMBW) {
    return {SharedState{SharedStateKind::kWMBW, state.i, state.j}};
  }
  if (rule == SharedRuleKind::kWMBWToBE) {
    return {SharedState{SharedStateKind::kBE, state.i, state.j}};
  }
  if (rule == SharedRuleKind::kBEToWIP) {
    return {SharedState{SharedStateKind::kWIP, state.i, state.j}};
  }
  if (rule == SharedRuleKind::kWMpToWIP) {
    return {SharedState{SharedStateKind::kWIP, state.i, state.j}};
  }
  if (rule == SharedRuleKind::kWIPToVP) {
    return {SharedState{SharedStateKind::kVP, state.i, state.j}};
  }
  if (rule == SharedRuleKind::kVPToVPL) {
    return {SharedState{SharedStateKind::kVPL, state.i, state.j}};
  }
  if (rule == SharedRuleKind::kVPLToVPR) {
    return {SharedState{SharedStateKind::kVPR, state.i, state.j}};
  }
  if (rule == SharedRuleKind::kVPRToV) {
    return {SharedState{SharedStateKind::kV, state.i, state.j}};
  }
  return {};
}

std::string state_name(const SharedStateKind state_kind) {
  if (state_kind == SharedStateKind::kW) {
    return "W";
  }
  if (state_kind == SharedStateKind::kWI) {
    return "WI";
  }
  if (state_kind == SharedStateKind::kV) {
    return "V";
  }
  if (state_kind == SharedStateKind::kVM) {
    return "VM";
  }
  if (state_kind == SharedStateKind::kWM) {
    return "WM";
  }
  if (state_kind == SharedStateKind::kWMv) {
    return "WMv";
  }
  if (state_kind == SharedStateKind::kWMp) {
    return "WMp";
  }
  if (state_kind == SharedStateKind::kWMB) {
    return "WMB";
  }
  if (state_kind == SharedStateKind::kWMBP) {
    return "WMBP";
  }
  if (state_kind == SharedStateKind::kWMBW) {
    return "WMBW";
  }
  if (state_kind == SharedStateKind::kBE) {
    return "BE";
  }
  if (state_kind == SharedStateKind::kWIP) {
    return "WIP";
  }
  if (state_kind == SharedStateKind::kVP) {
    return "VP";
  }
  if (state_kind == SharedStateKind::kVPL) {
    return "VPL";
  }
  return "VPR";
}

std::string rule_name(const SharedRuleKind rule) {
  if (rule == SharedRuleKind::kWToWI) {
    return "W_TO_WI";
  }
  if (rule == SharedRuleKind::kWIToV) {
    return "WI_TO_V";
  }
  if (rule == SharedRuleKind::kEmpty) {
    return "V_EMPTY";
  }
  if (rule == SharedRuleKind::kUnpaired) {
    return "V_UNPAIRED";
  }
  if (rule == SharedRuleKind::kPairWrapped) {
    return "V_PAIR_WRAPPED";
  }
  if (rule == SharedRuleKind::kVMToWM) {
    return "VM_TO_WM";
  }
  if (rule == SharedRuleKind::kWMToWMv) {
    return "WM_TO_WMv";
  }
  if (rule == SharedRuleKind::kWMvToWMp) {
    return "WMv_TO_WMp";
  }
  if (rule == SharedRuleKind::kWMpToWMB) {
    return "WMp_TO_WMB";
  }
  if (rule == SharedRuleKind::kWMBToWMBP) {
    return "WMB_TO_WMBP";
  }
  if (rule == SharedRuleKind::kWMBPToWMBW) {
    return "WMBP_TO_WMBW";
  }
  if (rule == SharedRuleKind::kWMBWToBE) {
    return "WMBW_TO_BE";
  }
  if (rule == SharedRuleKind::kBEToWIP) {
    return "BE_TO_WIP";
  }
  if (rule == SharedRuleKind::kWMpToWIP) {
    return "WMp_TO_WIP";
  }
  if (rule == SharedRuleKind::kWIPToVP) {
    return "WIP_TO_VP";
  }
  if (rule == SharedRuleKind::kVPToVPL) {
    return "VP_TO_VPL";
  }
  if (rule == SharedRuleKind::kVPLToVPR) {
    return "VPL_TO_VPR";
  }
  return "VPR_TO_V";
}

SharedEvaluationResult evaluate_shared_from_normalized(const NormalizedInput &ctx,
                                                       const SharedParseMode mode) {
  SharedEvaluationResult out;
  out.breakdown.topology_family = topology_family_for_structure(ctx.db_full);
  std::vector<SharedState> stack;
  stack.push_back(SharedState{SharedStateKind::kW, 1, static_cast<int>(ctx.db_full.size())});

  while (!stack.empty()) {
    const SharedState state = stack.back();
    stack.pop_back();
    const auto candidates = applicable_rules(state, ctx, mode);
    if (candidates.size() != 1) {
      fail_invalid_input("deterministic shared rule selection failed at state " +
                         state_name(state.kind) + "[" + std::to_string(state.i) + "," +
                         std::to_string(state.j) +
                         "] with candidates=" + std::to_string(candidates.size()));
    }

    const SharedRuleKind selected = candidates.front();
    out.trace.push_back(internal::RuleTraceStep{state_name(state.kind), state.i, state.j, rule_name(selected)});
    accumulate_breakdown(selected, out.breakdown);

    const auto children = expand(selected, state, mode);
    for (auto it = children.rbegin(); it != children.rend(); ++it) {
      stack.push_back(*it);
    }
  }

  return out;
}

std::vector<internal::RuleTraceStep> trace_rule_chain_slice_a_from_normalized(const NormalizedInput &ctx) {
  return evaluate_shared_from_normalized(ctx, SharedParseMode{false, false, false}).trace;
}

std::vector<internal::RuleTraceStep> trace_rule_chain_slice_b_from_normalized(const NormalizedInput &ctx) {
  return evaluate_shared_from_normalized(ctx, SharedParseMode{true, false, false}).trace;
}

std::vector<internal::RuleTraceStep> trace_rule_chain_slice_c_from_normalized(const NormalizedInput &ctx) {
  return evaluate_shared_from_normalized(ctx, SharedParseMode{true, true, false}).trace;
}

std::vector<internal::RuleTraceStep> trace_rule_chain_slice_d_from_normalized(const NormalizedInput &ctx) {
  return evaluate_shared_from_normalized(ctx, SharedParseMode{true, true, true}).trace;
}

EnergyBreakdown structure_energy_breakdown_from_normalized(const NormalizedInput &ctx) {
  return evaluate_shared_from_normalized(ctx, SharedParseMode{true, true, true}).breakdown;
}

std::vector<internal::RuleTraceStep> trace_rule_chain_zw_only_from_normalized(const NormalizedInput &ctx) {
  std::vector<internal::RuleTraceStep> out;
  const auto slice_a_trace = trace_rule_chain_slice_a_from_normalized(ctx);
  out.reserve(slice_a_trace.size());
  for (const auto &step : slice_a_trace) {
    if (step.state == "V") {
      internal::RuleTraceStep mapped = step;
      mapped.state = "ZW";
      if (step.rule == "V_EMPTY") {
        mapped.rule = "ZW_EMPTY";
      } else if (step.rule == "V_UNPAIRED") {
        mapped.rule = "ZW_UNPAIRED";
      } else if (step.rule == "V_PAIR_WRAPPED") {
        mapped.rule = "ZW_PAIR_WRAPPED";
      } else {
        continue;
      }
      out.push_back(mapped);
    }
  }
  return out;
}

}  // namespace

double get_structure_energy(const std::string &seq, const std::string &db_full) {
  const NormalizedInput normalized = normalize_input(seq, db_full);
  return structure_energy_breakdown_from_normalized(normalized).total_energy;
}

namespace internal {

std::vector<RuleTraceStep> trace_rule_chain_zw_only(const std::string &seq,
                                                    const std::string &db_full) {
  return trace_rule_chain_zw_only_from_normalized(normalize_input(seq, db_full));
}

std::vector<RuleTraceStep> trace_rule_chain_slice_a(const std::string &seq,
                                                    const std::string &db_full) {
  return trace_rule_chain_slice_a_from_normalized(normalize_input(seq, db_full));
}

std::vector<RuleTraceStep> trace_rule_chain_slice_b(const std::string &seq,
                                                    const std::string &db_full) {
  return trace_rule_chain_slice_b_from_normalized(normalize_input(seq, db_full));
}

std::vector<RuleTraceStep> trace_rule_chain_slice_c(const std::string &seq,
                                                    const std::string &db_full) {
  return trace_rule_chain_slice_c_from_normalized(normalize_input(seq, db_full));
}

std::vector<RuleTraceStep> trace_rule_chain_slice_d(const std::string &seq,
                                                    const std::string &db_full) {
  return trace_rule_chain_slice_d_from_normalized(normalize_input(seq, db_full));
}

EnergyBreakdown get_structure_energy_breakdown(const std::string &seq,
                                               const std::string &db_full) {
  return structure_energy_breakdown_from_normalized(normalize_input(seq, db_full));
}

const std::vector<std::string> &fixed_energy_target_states() {
  static const std::vector<std::string> kStates = {
      "W", "WI", "V", "VM", "WM", "WMv", "WMp", "WIP",
      "VP", "VPL", "VPR", "WMB", "WMBP", "WMBW", "BE", "ZW",
  };
  return kStates;
}

const std::vector<RolloutStatePlanEntry> &fixed_energy_rollout_plan() {
  static const std::vector<RolloutStatePlanEntry> kPlan = {
      {"W", "014"},   {"WI", "014"},  {"V", "014"},    {"VM", "015"},
      {"WM", "015"},  {"WMv", "015"}, {"WMp", "015"},  {"WIP", "016"},
      {"VP", "016"},  {"VPL", "016"}, {"VPR", "016"},  {"WMB", "017"},
      {"WMBP", "017"},{"WMBW", "017"},{"BE", "017"},   {"ZW", "013"},
  };
  return kPlan;
}

}  // namespace internal

}  // namespace cparty
