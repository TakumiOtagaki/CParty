#ifndef SCFG_RULES_CORE_HH_
#define SCFG_RULES_CORE_HH_

#include "scfg/rules_api.hh"

#include <vector>

class sparse_tree;
class Node;

namespace scfg {

class PartFuncWContext;
class PartFuncVContext;
class PartFuncWIContext;
class PartFuncVMContext;
class PartFuncWMvWMpContext;
class PartFuncWMContext;
class PartFuncWIPContext;
class PartFuncVPLContext;
class PartFuncVPRContext;
class PartFuncVPContext;
class PartFuncWMBWContext;
class PartFuncWMBPContext;
class PartFuncWMBContext;
class PartFuncBEContext;

struct RuleSpec {
    RuleId id;
    NonTerminal lhs;
    SplitSpec split;
};

struct RuleSplit {
    cand_pos_t k = -1;
    cand_pos_t l = -1;
    cand_pos_t p = -1;
    cand_pos_t q = -1;
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

std::vector<RuleSplit> enumerate_splits_v(RuleId rule,
                                          cand_pos_t i,
                                          cand_pos_t j,
                                          PartFuncVContext &ctx,
                                          sparse_tree &tree);
std::vector<RuleChild> expand_v(RuleId rule, cand_pos_t i, cand_pos_t j, const RuleSplit &split);
pf_t rule_score_v(RuleId rule,
                  cand_pos_t i,
                  cand_pos_t j,
                  const RuleSplit &split,
                  PartFuncVContext &ctx,
                  sparse_tree &tree);

std::vector<RuleSplit> enumerate_splits_wi(RuleId rule,
                                           cand_pos_t i,
                                           cand_pos_t j,
                                           PartFuncWIContext &ctx,
                                           sparse_tree &tree);
std::vector<RuleChild> expand_wi(RuleId rule, cand_pos_t i, cand_pos_t j, const RuleSplit &split);
pf_t rule_score_wi(RuleId rule,
                   cand_pos_t i,
                   cand_pos_t j,
                   const RuleSplit &split,
                   PartFuncWIContext &ctx);

std::vector<RuleSplit> enumerate_splits_vm(RuleId rule,
                                           cand_pos_t i,
                                           cand_pos_t j,
                                           PartFuncVMContext &ctx,
                                           std::vector<int> &up);
std::vector<RuleChild> expand_vm(RuleId rule, cand_pos_t i, cand_pos_t j, const RuleSplit &split);
pf_t rule_score_vm(RuleId rule,
                   cand_pos_t i,
                   cand_pos_t j,
                   const RuleSplit &split,
                   PartFuncVMContext &ctx,
                   std::vector<int> &up);

std::vector<RuleSplit> enumerate_splits_wmv_wmp(RuleId rule,
                                                cand_pos_t i,
                                                cand_pos_t j,
                                                PartFuncWMvWMpContext &ctx,
                                                std::vector<Node> &tree);
std::vector<RuleChild> expand_wmv_wmp(RuleId rule, cand_pos_t i, cand_pos_t j, const RuleSplit &split);
pf_t rule_score_wmv_wmp(RuleId rule,
                        cand_pos_t i,
                        cand_pos_t j,
                        const RuleSplit &split,
                        PartFuncWMvWMpContext &ctx,
                        std::vector<Node> &tree);

std::vector<RuleSplit> enumerate_splits_wm(RuleId rule,
                                           cand_pos_t i,
                                           cand_pos_t j,
                                           PartFuncWMContext &ctx,
                                           sparse_tree &tree);
std::vector<RuleChild> expand_wm(RuleId rule, cand_pos_t i, cand_pos_t j, const RuleSplit &split);
pf_t rule_score_wm(RuleId rule,
                   cand_pos_t i,
                   cand_pos_t j,
                   const RuleSplit &split,
                   PartFuncWMContext &ctx);

std::vector<RuleSplit> enumerate_splits_wip(RuleId rule,
                                            cand_pos_t i,
                                            cand_pos_t j,
                                            PartFuncWIPContext &ctx,
                                            sparse_tree &tree);
std::vector<RuleChild> expand_wip(RuleId rule, cand_pos_t i, cand_pos_t j, const RuleSplit &split);
pf_t rule_score_wip(RuleId rule,
                    cand_pos_t i,
                    cand_pos_t j,
                    const RuleSplit &split,
                    PartFuncWIPContext &ctx);

std::vector<RuleSplit> enumerate_splits_vpl(RuleId rule,
                                            cand_pos_t i,
                                            cand_pos_t j,
                                            PartFuncVPLContext &ctx,
                                            sparse_tree &tree);
std::vector<RuleChild> expand_vpl(RuleId rule, cand_pos_t i, cand_pos_t j, const RuleSplit &split);
pf_t rule_score_vpl(RuleId rule,
                    cand_pos_t i,
                    cand_pos_t j,
                    const RuleSplit &split,
                    PartFuncVPLContext &ctx);

std::vector<RuleSplit> enumerate_splits_vpr(RuleId rule,
                                            cand_pos_t i,
                                            cand_pos_t j,
                                            PartFuncVPRContext &ctx,
                                            sparse_tree &tree);
std::vector<RuleChild> expand_vpr(RuleId rule, cand_pos_t i, cand_pos_t j, const RuleSplit &split);
pf_t rule_score_vpr(RuleId rule,
                    cand_pos_t i,
                    cand_pos_t j,
                    const RuleSplit &split,
                    PartFuncVPRContext &ctx);

std::vector<RuleSplit> enumerate_splits_vp(RuleId rule,
                                           cand_pos_t i,
                                           cand_pos_t j,
                                           PartFuncVPContext &ctx,
                                           sparse_tree &tree);
std::vector<RuleChild> expand_vp(RuleId rule, cand_pos_t i, cand_pos_t j, const RuleSplit &split);
pf_t rule_score_vp(RuleId rule,
                   cand_pos_t i,
                   cand_pos_t j,
                   const RuleSplit &split,
                   PartFuncVPContext &ctx,
                   sparse_tree &tree);

std::vector<RuleSplit> enumerate_splits_wmbw(RuleId rule,
                                             cand_pos_t i,
                                             cand_pos_t j,
                                             PartFuncWMBWContext &ctx,
                                             sparse_tree &tree);
std::vector<RuleChild> expand_wmbw(RuleId rule, cand_pos_t i, cand_pos_t j, const RuleSplit &split);
pf_t rule_score_wmbw(RuleId rule, cand_pos_t i, cand_pos_t j, const RuleSplit &split, PartFuncWMBWContext &ctx);

std::vector<RuleSplit> enumerate_splits_wmbp(RuleId rule,
                                             cand_pos_t i,
                                             cand_pos_t j,
                                             PartFuncWMBPContext &ctx,
                                             sparse_tree &tree);
std::vector<RuleChild> expand_wmbp(RuleId rule, cand_pos_t i, cand_pos_t j, const RuleSplit &split);
pf_t rule_score_wmbp(RuleId rule,
                     cand_pos_t i,
                     cand_pos_t j,
                     const RuleSplit &split,
                     PartFuncWMBPContext &ctx,
                     sparse_tree &tree);

std::vector<RuleSplit> enumerate_splits_wmb(RuleId rule,
                                            cand_pos_t i,
                                            cand_pos_t j,
                                            PartFuncWMBContext &ctx,
                                            sparse_tree &tree);
std::vector<RuleChild> expand_wmb(RuleId rule, cand_pos_t i, cand_pos_t j, const RuleSplit &split);
pf_t rule_score_wmb(RuleId rule,
                    cand_pos_t i,
                    cand_pos_t j,
                    const RuleSplit &split,
                    PartFuncWMBContext &ctx,
                    sparse_tree &tree);

std::vector<RuleSplit> enumerate_splits_be(RuleId rule,
                                           cand_pos_t i,
                                           cand_pos_t j,
                                           cand_pos_t ip,
                                           cand_pos_t jp,
                                           PartFuncBEContext &ctx,
                                           sparse_tree &tree);
std::vector<RuleChild> expand_be(RuleId rule,
                                 cand_pos_t i,
                                 cand_pos_t j,
                                 cand_pos_t ip,
                                 cand_pos_t jp,
                                 const RuleSplit &split);
pf_t rule_score_be(RuleId rule,
                   cand_pos_t i,
                   cand_pos_t j,
                   cand_pos_t ip,
                   cand_pos_t jp,
                   const RuleSplit &split,
                   PartFuncBEContext &ctx,
                   sparse_tree &tree);

} // namespace scfg

#endif
