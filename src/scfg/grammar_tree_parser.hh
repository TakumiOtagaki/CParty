#ifndef CPARTY_SCFG_GRAMMAR_TREE_PARSER_HH
#define CPARTY_SCFG_GRAMMAR_TREE_PARSER_HH

#include <string>
#include <vector>

namespace cparty::scfg {

struct ParseOptions {
  bool return_trace = true;
  bool require_rules_core = true;
};

struct RuleTraceStep {
  std::string state;
  int i = 0;
  int j = 0;
  std::string rule;
};

struct ParseResult {
  double total_energy = 0.0;
  std::vector<RuleTraceStep> trace;
  bool ok = true;
  std::string error;
};

ParseResult parse_fixed_energy(const std::string &seq,
                               const std::string &structure_g,
                               const std::string &structure_gprime,
                               const ParseOptions &options = {});

}  // namespace cparty::scfg

#endif  // CPARTY_SCFG_GRAMMAR_TREE_PARSER_HH
