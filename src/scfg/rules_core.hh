#ifndef SCFG_RULES_CORE_HH_
#define SCFG_RULES_CORE_HH_

#include "scfg/rules_api.hh"

#include <vector>

namespace scfg {

struct RuleSpec {
    RuleId id;
    NonTerminal lhs;
    SplitSpec split;
};

// Returns the full candidate rule list for a non-terminal, before applicability filtering.
const std::vector<RuleId> &rules_for(NonTerminal nonterminal);

} // namespace scfg

#endif
