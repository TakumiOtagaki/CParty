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
};

struct SharedState {
  SharedStateKind kind = SharedStateKind::kW;
  int i = 1;  // 1-based inclusive
  int j = 0;  // 1-based inclusive
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

  std::vector<size_t> stack;
  stack.reserve(db_full.size());

  for (size_t i = 0; i < db_full.size(); ++i) {
    const char c = db_full[i];
    if (c == '.') {
      continue;
    }
    if (c == '(') {
      stack.push_back(i);
      continue;
    }
    if (c == ')') {
      if (stack.empty()) {
        fail_invalid_input("unbalanced structure: closing bracket without opener");
      }
      stack.pop_back();
      continue;
    }
    fail_invalid_input("structure contains unsupported symbol at position " + std::to_string(i + 1));
  }

  if (!stack.empty()) {
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

  std::vector<int> stack;
  stack.reserve(db_full.size());
  for (size_t idx = 0; idx < db_full.size(); ++idx) {
    const char c = db_full[idx];
    if (c == '(') {
      stack.push_back(static_cast<int>(idx));
      continue;
    }
    if (c == ')') {
      const int left = stack.back();
      stack.pop_back();
      out.pair_map[static_cast<size_t>(left)] = static_cast<int>(idx);
      out.pair_map[idx] = left;
    }
  }
  return out;
}

std::vector<SharedRuleKind> rules_for(const SharedStateKind state_kind, const bool include_slice_b_states) {
  if (state_kind == SharedStateKind::kW) {
    return {SharedRuleKind::kWToWI};
  }
  if (state_kind == SharedStateKind::kWI) {
    return {SharedRuleKind::kWIToV};
  }
  if (state_kind == SharedStateKind::kV) {
    if (include_slice_b_states) {
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
  return {SharedRuleKind::kWMpToV};
}

bool rule_is_applicable(const SharedRuleKind rule,
                        const SharedState state,
                        const NormalizedInput &ctx,
                        const bool include_slice_b_states) {
  if (rule == SharedRuleKind::kWToWI || rule == SharedRuleKind::kWIToV) {
    return true;
  }

  if (rule == SharedRuleKind::kVMToWM || rule == SharedRuleKind::kWMToWMv ||
      rule == SharedRuleKind::kWMvToWMp || rule == SharedRuleKind::kWMpToV) {
    return include_slice_b_states;
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
    if (ctx.db_full[left] != '(') {
      return false;
    }
    const int partner = ctx.pair_map[left];
    return partner == (state.j - 1);
  }
  return false;
}

std::vector<SharedRuleKind> applicable_rules(const SharedState state,
                                             const NormalizedInput &ctx,
                                             const bool include_slice_b_states) {
  const auto candidates = rules_for(state.kind, include_slice_b_states);
  std::vector<SharedRuleKind> out;
  out.reserve(candidates.size());
  for (const SharedRuleKind rule : candidates) {
    if (rule_is_applicable(rule, state, ctx, include_slice_b_states)) {
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

std::vector<SharedState> expand(const SharedRuleKind rule,
                                const SharedState state,
                                const bool include_slice_b_states) {
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
    if (include_slice_b_states) {
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
  if (rule == SharedRuleKind::kWMpToV) {
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
  return "WMp";
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
  return "WMp_TO_V";
}

std::vector<internal::RuleTraceStep> trace_rule_chain_shared_from_normalized(
    const NormalizedInput &ctx,
    const bool include_slice_b_states) {
  std::vector<internal::RuleTraceStep> trace;
  std::vector<SharedState> stack;
  stack.push_back(SharedState{SharedStateKind::kW, 1, static_cast<int>(ctx.db_full.size())});

  while (!stack.empty()) {
    const SharedState state = stack.back();
    stack.pop_back();
    const auto candidates = applicable_rules(state, ctx, include_slice_b_states);
    if (candidates.size() != 1) {
      fail_invalid_input("deterministic shared rule selection failed at state " +
                         state_name(state.kind) + "[" + std::to_string(state.i) + "," +
                         std::to_string(state.j) +
                         "] with candidates=" + std::to_string(candidates.size()));
    }

    const SharedRuleKind selected = candidates.front();
    trace.push_back(internal::RuleTraceStep{state_name(state.kind), state.i, state.j, rule_name(selected)});

    const auto children = expand(selected, state, include_slice_b_states);
    for (auto it = children.rbegin(); it != children.rend(); ++it) {
      stack.push_back(*it);
    }
  }

  return trace;
}

std::vector<internal::RuleTraceStep> trace_rule_chain_slice_a_from_normalized(const NormalizedInput &ctx) {
  return trace_rule_chain_shared_from_normalized(ctx, false);
}

std::vector<internal::RuleTraceStep> trace_rule_chain_slice_b_from_normalized(const NormalizedInput &ctx) {
  return trace_rule_chain_shared_from_normalized(ctx, true);
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
  const auto trace = trace_rule_chain_slice_b_from_normalized(normalized);

  double total = 0.0;
  for (const auto &step : trace) {
    if (step.rule == "V_PAIR_WRAPPED") {
      total += rule_score(SharedRuleKind::kPairWrapped);
    } else {
      total += 0.0;
    }
  }
  return total;
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
