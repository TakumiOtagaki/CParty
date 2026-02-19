#ifndef SCFG_RULES_CORE_HH_
#define SCFG_RULES_CORE_HH_

#include "scfg/rules_api.hh"

#include <cstddef>
#include <vector>

class sparse_tree;
class Node;
namespace scfg {

class StructureView;

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

enum class Endpoint : unsigned char {
    I,
    J,
    K,
    L,
    P,
    Q,
    IP,
    JP,
};

struct EndpointRef {
    Endpoint base = Endpoint::I;
    int offset = 0;
};

struct SpanSpec {
    EndpointRef left{};
    EndpointRef right{};
};

struct RuleChildSpec {
    NonTerminal nonterminal;
    SpanSpec span{};
    bool allow_empty = false;
};

struct RuleSpec {
    RuleId id;
    NonTerminal lhs;
    SplitSpec split;
    const RuleChildSpec *rhs = nullptr;
    size_t rhs_len = 0;
    enum class SplitGenKind : unsigned char { Custom, KRange, BandMinBpRange, BandMaxBpRange } split_gen =
        SplitGenKind::Custom;
    enum class SplitFilterKind : unsigned char { None, CanPairLeft, CanPairRight } split_filter =
        SplitFilterKind::None;
    enum class PredicateKind : unsigned char {
        None,
        VPairingState,
        UnpairedAtJ,
        UnpairedAtJMinus1,
        VpStackPairing,
        VpInternalLoopPairing,
        VprBasepair,
        VpWiCase1,
        VpWiCase2,
        VpWiCase3,
        BeStackPairing,
        WmbpJUnpaired,
        WmbpJUnpairedIpaired,
        WmbSplitBeWmbpWi
    } predicate = PredicateKind::None;
    struct SplitRangeSpec {
        EndpointRef start{};
        EndpointRef end{};
        bool end_inclusive = true;
        bool subtract_turn = false;
    } split_range{};
};

struct RuleSpanContext {
    cand_pos_t i = -1;
    cand_pos_t j = -1;
    RuleSplit split{};
    cand_pos_t ip = -1;
    cand_pos_t jp = -1;
};

struct ApplicableRule {
    RuleId rule;
    RuleSplit split;
};

const RuleSpec &rule_spec(RuleId rule);
const std::vector<RuleSpec> &rule_catalog();

cand_pos_t resolve_endpoint(EndpointRef endpoint, const RuleSpanContext &ctx);
std::vector<RuleChild> expand_rule_rhs(const RuleSpec &spec, const RuleSpanContext &ctx);
std::vector<RuleSplit> enumerate_splits_k_range(const RuleSpec &spec, const RuleSpanContext &ctx, cand_pos_t turn);

// Returns the full candidate rule list for a non-terminal, before applicability filtering.
const std::vector<RuleId> &rules_for(NonTerminal nonterminal);

// W-only rule helpers (initial step for rule-core migration).
std::vector<RuleSplit> enumerate_splits_w(RuleId rule,
                                          cand_pos_t i,
                                          cand_pos_t j,
                                          PartFuncWContext &ctx,
                                          sparse_tree &tree);
std::vector<ApplicableRule> applicable_rules_w(cand_pos_t i,
                                               cand_pos_t j,
                                               PartFuncWContext &ctx,
                                               sparse_tree &tree);
std::vector<RuleChild> expand_w(RuleId rule, cand_pos_t i, cand_pos_t j, const RuleSplit &split);
pf_t transition_weight_w(RuleId rule, cand_pos_t i, cand_pos_t j, const RuleSplit &split, PartFuncWContext &ctx);

std::vector<RuleSplit> enumerate_splits_v(RuleId rule,
                                          cand_pos_t i,
                                          cand_pos_t j,
                                          PartFuncVContext &ctx,
                                          sparse_tree &tree);
std::vector<ApplicableRule> applicable_rules_v(cand_pos_t i,
                                               cand_pos_t j,
                                               PartFuncVContext &ctx,
                                               sparse_tree &tree);
std::vector<RuleChild> expand_v(RuleId rule, cand_pos_t i, cand_pos_t j, const RuleSplit &split);
pf_t transition_weight_v(RuleId rule,
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
std::vector<ApplicableRule> applicable_rules_wi(cand_pos_t i,
                                                cand_pos_t j,
                                                PartFuncWIContext &ctx,
                                                sparse_tree &tree);
std::vector<RuleChild> expand_wi(RuleId rule, cand_pos_t i, cand_pos_t j, const RuleSplit &split);
pf_t transition_weight_wi(RuleId rule,
                          cand_pos_t i,
                          cand_pos_t j,
                          const RuleSplit &split,
                          PartFuncWIContext &ctx);

std::vector<RuleSplit> enumerate_splits_vm(RuleId rule,
                                           cand_pos_t i,
                                           cand_pos_t j,
                                           PartFuncVMContext &ctx,
                                           std::vector<int> &up);
std::vector<ApplicableRule> applicable_rules_vm(cand_pos_t i,
                                                cand_pos_t j,
                                                PartFuncVMContext &ctx,
                                                std::vector<int> &up);
std::vector<RuleChild> expand_vm(RuleId rule, cand_pos_t i, cand_pos_t j, const RuleSplit &split);
pf_t transition_weight_vm(RuleId rule,
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
std::vector<ApplicableRule> applicable_rules_wmv_wmp(cand_pos_t i,
                                                     cand_pos_t j,
                                                     PartFuncWMvWMpContext &ctx,
                                                     std::vector<Node> &tree);
std::vector<RuleChild> expand_wmv_wmp(RuleId rule, cand_pos_t i, cand_pos_t j, const RuleSplit &split);
pf_t transition_weight_wmv_wmp(RuleId rule,
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
std::vector<ApplicableRule> applicable_rules_wm(cand_pos_t i,
                                                cand_pos_t j,
                                                PartFuncWMContext &ctx,
                                                sparse_tree &tree);
std::vector<RuleChild> expand_wm(RuleId rule, cand_pos_t i, cand_pos_t j, const RuleSplit &split);
pf_t transition_weight_wm(RuleId rule,
                          cand_pos_t i,
                          cand_pos_t j,
                          const RuleSplit &split,
                          PartFuncWMContext &ctx);

std::vector<RuleSplit> enumerate_splits_wip(RuleId rule,
                                            cand_pos_t i,
                                            cand_pos_t j,
                                            PartFuncWIPContext &ctx,
                                            sparse_tree &tree);
std::vector<RuleSplit> enumerate_splits_wip(RuleId rule,
                                            cand_pos_t i,
                                            cand_pos_t j,
                                            PartFuncWIPContext &ctx,
                                            const StructureView &view);
std::vector<ApplicableRule> applicable_rules_wip(cand_pos_t i,
                                                 cand_pos_t j,
                                                 PartFuncWIPContext &ctx,
                                                 sparse_tree &tree);
std::vector<ApplicableRule> applicable_rules_wip(cand_pos_t i,
                                                 cand_pos_t j,
                                                 PartFuncWIPContext &ctx,
                                                 const StructureView &view);
std::vector<RuleChild> expand_wip(RuleId rule, cand_pos_t i, cand_pos_t j, const RuleSplit &split);
pf_t transition_weight_wip(RuleId rule,
                           cand_pos_t i,
                           cand_pos_t j,
                           const RuleSplit &split,
                           PartFuncWIPContext &ctx);

std::vector<RuleSplit> enumerate_splits_vpl(RuleId rule,
                                            cand_pos_t i,
                                            cand_pos_t j,
                                            PartFuncVPLContext &ctx,
                                            sparse_tree &tree);
std::vector<RuleSplit> enumerate_splits_vpl(RuleId rule,
                                            cand_pos_t i,
                                            cand_pos_t j,
                                            PartFuncVPLContext &ctx,
                                            const StructureView &view);
std::vector<ApplicableRule> applicable_rules_vpl(cand_pos_t i,
                                                 cand_pos_t j,
                                                 PartFuncVPLContext &ctx,
                                                 sparse_tree &tree);
std::vector<ApplicableRule> applicable_rules_vpl(cand_pos_t i,
                                                 cand_pos_t j,
                                                 PartFuncVPLContext &ctx,
                                                 const StructureView &view);
std::vector<RuleChild> expand_vpl(RuleId rule, cand_pos_t i, cand_pos_t j, const RuleSplit &split);
pf_t transition_weight_vpl(RuleId rule,
                           cand_pos_t i,
                           cand_pos_t j,
                           const RuleSplit &split,
                           PartFuncVPLContext &ctx);

std::vector<RuleSplit> enumerate_splits_vpr(RuleId rule,
                                            cand_pos_t i,
                                            cand_pos_t j,
                                            PartFuncVPRContext &ctx,
                                            sparse_tree &tree);
std::vector<RuleSplit> enumerate_splits_vpr(RuleId rule,
                                            cand_pos_t i,
                                            cand_pos_t j,
                                            PartFuncVPRContext &ctx,
                                            const StructureView &view);
std::vector<ApplicableRule> applicable_rules_vpr(cand_pos_t i,
                                                 cand_pos_t j,
                                                 PartFuncVPRContext &ctx,
                                                 sparse_tree &tree);
std::vector<ApplicableRule> applicable_rules_vpr(cand_pos_t i,
                                                 cand_pos_t j,
                                                 PartFuncVPRContext &ctx,
                                                 const StructureView &view);
std::vector<RuleChild> expand_vpr(RuleId rule, cand_pos_t i, cand_pos_t j, const RuleSplit &split);
pf_t transition_weight_vpr(RuleId rule,
                           cand_pos_t i,
                           cand_pos_t j,
                           const RuleSplit &split,
                           PartFuncVPRContext &ctx);

std::vector<RuleSplit> enumerate_splits_vp(RuleId rule,
                                           cand_pos_t i,
                                           cand_pos_t j,
                                           PartFuncVPContext &ctx,
                                           sparse_tree &tree);
std::vector<RuleSplit> enumerate_splits_vp(RuleId rule,
                                           cand_pos_t i,
                                           cand_pos_t j,
                                           PartFuncVPContext &ctx,
                                           const StructureView &view);
std::vector<ApplicableRule> applicable_rules_vp(cand_pos_t i,
                                                cand_pos_t j,
                                                PartFuncVPContext &ctx,
                                                sparse_tree &tree);
std::vector<ApplicableRule> applicable_rules_vp(cand_pos_t i,
                                                cand_pos_t j,
                                                PartFuncVPContext &ctx,
                                                const StructureView &view);
std::vector<RuleChild> expand_vp(RuleId rule, cand_pos_t i, cand_pos_t j, const RuleSplit &split);
pf_t transition_weight_vp(RuleId rule,
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
std::vector<RuleSplit> enumerate_splits_wmbw(RuleId rule,
                                             cand_pos_t i,
                                             cand_pos_t j,
                                             PartFuncWMBWContext &ctx,
                                             const StructureView &view);
std::vector<ApplicableRule> applicable_rules_wmbw(cand_pos_t i,
                                                  cand_pos_t j,
                                                  PartFuncWMBWContext &ctx,
                                                  sparse_tree &tree);
std::vector<ApplicableRule> applicable_rules_wmbw(cand_pos_t i,
                                                  cand_pos_t j,
                                                  PartFuncWMBWContext &ctx,
                                                  const StructureView &view);
std::vector<RuleChild> expand_wmbw(RuleId rule, cand_pos_t i, cand_pos_t j, const RuleSplit &split);
pf_t transition_weight_wmbw(RuleId rule, cand_pos_t i, cand_pos_t j, const RuleSplit &split, PartFuncWMBWContext &ctx);

std::vector<RuleSplit> enumerate_splits_wmbp(RuleId rule,
                                             cand_pos_t i,
                                             cand_pos_t j,
                                             PartFuncWMBPContext &ctx,
                                             sparse_tree &tree);
std::vector<RuleSplit> enumerate_splits_wmbp(RuleId rule,
                                             cand_pos_t i,
                                             cand_pos_t j,
                                             PartFuncWMBPContext &ctx,
                                             const StructureView &view,
                                             sparse_tree &tree);
std::vector<ApplicableRule> applicable_rules_wmbp(cand_pos_t i,
                                                  cand_pos_t j,
                                                  PartFuncWMBPContext &ctx,
                                                  sparse_tree &tree);
std::vector<ApplicableRule> applicable_rules_wmbp(cand_pos_t i,
                                                  cand_pos_t j,
                                                  PartFuncWMBPContext &ctx,
                                                  const StructureView &view,
                                                  sparse_tree &tree);
std::vector<RuleChild> expand_wmbp(RuleId rule, cand_pos_t i, cand_pos_t j, const RuleSplit &split);
pf_t transition_weight_wmbp(RuleId rule,
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
std::vector<RuleSplit> enumerate_splits_wmb(RuleId rule,
                                            cand_pos_t i,
                                            cand_pos_t j,
                                            PartFuncWMBContext &ctx,
                                            const StructureView &view);
std::vector<ApplicableRule> applicable_rules_wmb(cand_pos_t i,
                                                 cand_pos_t j,
                                                 PartFuncWMBContext &ctx,
                                                 sparse_tree &tree);
std::vector<ApplicableRule> applicable_rules_wmb(cand_pos_t i,
                                                 cand_pos_t j,
                                                 PartFuncWMBContext &ctx,
                                                 const StructureView &view);
std::vector<RuleChild> expand_wmb(RuleId rule, cand_pos_t i, cand_pos_t j, const RuleSplit &split);
pf_t transition_weight_wmb(RuleId rule,
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
std::vector<RuleSplit> enumerate_splits_be(RuleId rule,
                                           cand_pos_t i,
                                           cand_pos_t j,
                                           cand_pos_t ip,
                                           cand_pos_t jp,
                                           PartFuncBEContext &ctx,
                                           const StructureView &view);
std::vector<ApplicableRule> applicable_rules_be(cand_pos_t i,
                                                cand_pos_t j,
                                                cand_pos_t ip,
                                                cand_pos_t jp,
                                                PartFuncBEContext &ctx,
                                                sparse_tree &tree);
std::vector<ApplicableRule> applicable_rules_be(cand_pos_t i,
                                                cand_pos_t j,
                                                cand_pos_t ip,
                                                cand_pos_t jp,
                                                PartFuncBEContext &ctx,
                                                const StructureView &view);
std::vector<RuleChild> expand_be(RuleId rule,
                                 cand_pos_t i,
                                 cand_pos_t j,
                                 cand_pos_t ip,
                                 cand_pos_t jp,
                                 const RuleSplit &split);
pf_t transition_weight_be(RuleId rule,
                          cand_pos_t i,
                          cand_pos_t j,
                          cand_pos_t ip,
                          cand_pos_t jp,
                          const RuleSplit &split,
                          PartFuncBEContext &ctx,
                          sparse_tree &tree);

} // namespace scfg

#endif
