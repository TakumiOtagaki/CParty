#ifndef SCFG_RULES_CORE_HH_
#define SCFG_RULES_CORE_HH_

#include "scfg/rules_api.hh"

#include <vector>

class sparse_tree;

namespace scfg {

class PartFuncWContext;

struct RuleSpec {
    RuleId id;
    NonTerminal lhs;
    SplitSpec split;
};

struct RuleSplit {
    cand_pos_t k = -1;
    cand_pos_t l = -1;
};

struct RuleChild {
    NonTerminal nonterminal;
    cand_pos_t i = -1;
    cand_pos_t j = -1;
};

// Returns the full candidate rule list for a non-terminal, before applicability filtering.
const std::vector<RuleId> &rules_for(NonTerminal nonterminal);

// W-only rule helpers (initial step for rule-core migration).
std::vector<RuleSplit> enumerate_splits_w(RuleId rule,
                                          cand_pos_t i,
                                          cand_pos_t j,
                                          PartFuncWContext &ctx,
                                          sparse_tree &tree);
std::vector<RuleChild> expand_w(RuleId rule, cand_pos_t i, cand_pos_t j, const RuleSplit &split);
pf_t rule_score_w(RuleId rule, cand_pos_t i, cand_pos_t j, const RuleSplit &split, PartFuncWContext &ctx);

} // namespace scfg

#endif
