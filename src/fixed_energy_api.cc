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

enum class ZWRuleKind {
  kEmpty,
  kUnpaired,
  kPairWrapped,
};

struct ZWState {
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

std::vector<ZWRuleKind> rules_for_zw() {
  return {ZWRuleKind::kEmpty, ZWRuleKind::kUnpaired, ZWRuleKind::kPairWrapped};
}

bool rule_is_applicable(const ZWRuleKind rule,
                        const ZWState state,
                        const NormalizedInput &ctx) {
  if (rule == ZWRuleKind::kEmpty) {
    return state.i > state.j;
  }
  if (state.i > state.j) {
    return false;
  }

  const size_t left = static_cast<size_t>(state.i - 1);
  if (rule == ZWRuleKind::kUnpaired) {
    return ctx.db_full[left] == '.';
  }
  if (rule == ZWRuleKind::kPairWrapped) {
    if (ctx.db_full[left] != '(') {
      return false;
    }
    const int partner = ctx.pair_map[left];
    return partner == (state.j - 1);
  }
  return false;
}

std::vector<ZWRuleKind> applicable_rules(const ZWState state, const NormalizedInput &ctx) {
  const auto candidates = rules_for_zw();
  std::vector<ZWRuleKind> out;
  out.reserve(candidates.size());
  for (const ZWRuleKind rule : candidates) {
    if (rule_is_applicable(rule, state, ctx)) {
      out.push_back(rule);
    }
  }
  return out;
}

double rule_score(const ZWRuleKind rule) {
  if (rule == ZWRuleKind::kPairWrapped) {
    return -1.0;
  }
  return 0.0;
}

ZWState expand(const ZWRuleKind rule, const ZWState state) {
  if (rule == ZWRuleKind::kUnpaired) {
    return ZWState{state.i + 1, state.j};
  }
  if (rule == ZWRuleKind::kPairWrapped) {
    return ZWState{state.i + 1, state.j - 1};
  }
  return ZWState{1, 0};
}

std::string rule_name(const ZWRuleKind rule) {
  if (rule == ZWRuleKind::kEmpty) {
    return "ZW_EMPTY";
  }
  if (rule == ZWRuleKind::kUnpaired) {
    return "ZW_UNPAIRED";
  }
  return "ZW_PAIR_WRAPPED";
}

std::vector<internal::RuleTraceStep> trace_rule_chain_zw_only_from_normalized(const NormalizedInput &ctx) {
  std::vector<internal::RuleTraceStep> trace;
  ZWState state{1, static_cast<int>(ctx.db_full.size())};

  while (true) {
    const auto candidates = applicable_rules(state, ctx);
    if (candidates.size() != 1) {
      fail_invalid_input("deterministic ZW rule selection failed at state [" +
                         std::to_string(state.i) + "," + std::to_string(state.j) +
                         "] with candidates=" + std::to_string(candidates.size()));
    }

    const ZWRuleKind selected = candidates.front();
    trace.push_back(internal::RuleTraceStep{"ZW", state.i, state.j, rule_name(selected)});

    if (selected == ZWRuleKind::kEmpty) {
      break;
    }
    state = expand(selected, state);
  }
  return trace;
}

}  // namespace

double get_structure_energy(const std::string &seq, const std::string &db_full) {
  const NormalizedInput normalized = normalize_input(seq, db_full);
  const auto trace = trace_rule_chain_zw_only_from_normalized(normalized);

  double total = 0.0;
  for (const auto &step : trace) {
    if (step.rule == "ZW_PAIR_WRAPPED") {
      total += rule_score(ZWRuleKind::kPairWrapped);
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

}  // namespace internal

}  // namespace cparty
