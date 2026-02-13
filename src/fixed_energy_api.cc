#include "fixed_energy_api.hh"

#include <cstddef>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace cparty {
namespace {

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

struct NormalizedInput {
  std::string seq;
  std::string db_full;
  std::vector<int> pair_index;
};

NormalizedInput normalize_input(const std::string &seq, const std::string &db_full) {
  validate_sequence(seq);
  validate_structure(db_full, seq.size());

  std::vector<int> pair_index(db_full.size(), -1);
  std::vector<size_t> stack;
  stack.reserve(db_full.size());

  for (size_t i = 0; i < db_full.size(); ++i) {
    const char c = db_full[i];
    if (c == '(') {
      stack.push_back(i);
      continue;
    }
    if (c == ')') {
      if (stack.empty()) {
        fail_invalid_input("unbalanced structure while building pair map");
      }
      const size_t open = stack.back();
      stack.pop_back();
      pair_index[open] = static_cast<int>(i);
      pair_index[i] = static_cast<int>(open);
    }
  }

  if (!stack.empty()) {
    fail_invalid_input("unbalanced structure while building pair map");
  }

  return NormalizedInput{seq, db_full, std::move(pair_index)};
}

enum class ParserState {
  ZW,
};

enum class ParserRule {
  ZW_Epsilon,
  ZW_Unpaired,
  ZW_Paired,
};

struct ParserItem {
  ParserState state;
  size_t i;
  size_t j;
};

const char *state_name(const ParserState state) {
  switch (state) {
    case ParserState::ZW:
      return "ZW";
  }
  return "unknown";
}

const char *rule_name(const ParserRule rule) {
  switch (rule) {
    case ParserRule::ZW_Epsilon:
      return "ZW -> epsilon";
    case ParserRule::ZW_Unpaired:
      return "ZW -> '.' ZW";
    case ParserRule::ZW_Paired:
      return "ZW -> '(' ZW ')' ZW";
  }
  return "unknown";
}

class FixedStructureParser {
 public:
  explicit FixedStructureParser(const NormalizedInput &input) : input_(input) {}

  std::vector<ParserRule> rules_for(const ParserState state) const {
    switch (state) {
      case ParserState::ZW:
        return {ParserRule::ZW_Epsilon, ParserRule::ZW_Unpaired, ParserRule::ZW_Paired};
    }
    return {};
  }

  std::vector<ParserRule> applicable_rules(const std::vector<ParserRule> &rules,
                                           const ParserItem &item) const {
    std::vector<ParserRule> applicable;
    applicable.reserve(rules.size());

    for (const ParserRule rule : rules) {
      if (is_rule_applicable(rule, item)) {
        applicable.push_back(rule);
      }
    }

    return applicable;
  }

  double rule_score(const ParserRule rule, const ParserItem &) const {
    if (rule == ParserRule::ZW_Paired) {
      return -1.0;
    }
    return 0.0;
  }

  std::vector<ParserItem> expand(const ParserRule rule, const ParserItem &item) const {
    if (rule == ParserRule::ZW_Epsilon) {
      return {};
    }

    if (rule == ParserRule::ZW_Unpaired) {
      return {{ParserState::ZW, item.i + 1, item.j}};
    }

    const size_t pair = static_cast<size_t>(input_.pair_index[item.i]);
    return {
        {ParserState::ZW, item.i + 1, pair},
        {ParserState::ZW, pair + 1, item.j},
    };
  }

  struct ParseResult {
    double total = 0.0;
    std::vector<fixed_energy_internal::ParseTraceStep> trace;
  };

  ParseResult run(const bool capture_trace) const {
    ParseResult result;
    std::vector<ParserItem> stack;
    stack.push_back({ParserState::ZW, 0, input_.db_full.size()});

    while (!stack.empty()) {
      const ParserItem item = stack.back();
      stack.pop_back();

      const std::vector<ParserRule> rules = rules_for(item.state);
      const std::vector<ParserRule> candidates = applicable_rules(rules, item);

      if (candidates.size() != 1) {
        fail_invalid_input(std::string("non-unique or missing applicable rule at state=") +
                           state_name(item.state) + " interval=[" +
                           std::to_string(item.i + 1) + "," + std::to_string(item.j) +
                           ") candidates=" + std::to_string(candidates.size()));
      }

      const ParserRule rule = candidates.front();
      result.total += rule_score(rule, item);

      if (capture_trace) {
        result.trace.push_back({state_name(item.state), item.i + 1, item.j, rule_name(rule)});
      }

      const auto children = expand(rule, item);
      for (auto it = children.rbegin(); it != children.rend(); ++it) {
        stack.push_back(*it);
      }
    }

    return result;
  }

 private:
  bool is_rule_applicable(const ParserRule rule, const ParserItem &item) const {
    if (item.state != ParserState::ZW) {
      return false;
    }

    switch (rule) {
      case ParserRule::ZW_Epsilon:
        return item.i == item.j;
      case ParserRule::ZW_Unpaired:
        return item.i < item.j && input_.db_full[item.i] == '.';
      case ParserRule::ZW_Paired: {
        if (item.i >= item.j || input_.db_full[item.i] != '(') {
          return false;
        }
        const int pair_index = input_.pair_index[item.i];
        return pair_index >= 0 && static_cast<size_t>(pair_index) < item.j;
      }
    }

    return false;
  }

  const NormalizedInput &input_;
};

FixedStructureParser::ParseResult parse_fixed_structure(const std::string &seq,
                                                        const std::string &db_full,
                                                        const bool capture_trace) {
  const NormalizedInput normalized = normalize_input(seq, db_full);
  const FixedStructureParser parser(normalized);
  return parser.run(capture_trace);
}

}  // namespace

double get_structure_energy(const std::string &seq, const std::string &db_full) {
  return parse_fixed_structure(seq, db_full, false).total;
}

namespace fixed_energy_internal {

std::vector<ParseTraceStep> trace_rule_chain(const std::string &seq, const std::string &db_full) {
  return parse_fixed_structure(seq, db_full, true).trace;
}

}  // namespace fixed_energy_internal

}  // namespace cparty
