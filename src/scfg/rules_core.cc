#include "scfg/rules_core.hh"

#include <array>

namespace scfg {
namespace {

constexpr size_t kNonTerminalCount = 15;

const RuleSpec kRuleSpecs[] = {
    // V
    {RuleId::V_HAIRPIN, NonTerminal::V, {SplitKind::None}},
    {RuleId::V_INTERNAL, NonTerminal::V, {SplitKind::KL}},
    {RuleId::V_VM, NonTerminal::V, {SplitKind::None}},

    // WI
    {RuleId::WI_BASE_SINGLE, NonTerminal::WI, {SplitKind::None}},
    {RuleId::WI_SPLIT_V, NonTerminal::WI, {SplitKind::K}},
    {RuleId::WI_SPLIT_WMB, NonTerminal::WI, {SplitKind::K}},
    {RuleId::WI_EXTEND_UNPAIRED, NonTerminal::WI, {SplitKind::None}},

    // W
    {RuleId::W_EXTEND_UNPAIRED, NonTerminal::W, {SplitKind::None}},
    {RuleId::W_SPLIT_V, NonTerminal::W, {SplitKind::K}},
    {RuleId::W_SPLIT_WMB, NonTerminal::W, {SplitKind::K}},

    // VM
    {RuleId::VM_SPLIT_WM_WMv, NonTerminal::VM, {SplitKind::K}},
    {RuleId::VM_SPLIT_WM_WMp, NonTerminal::VM, {SplitKind::K}},
    {RuleId::VM_SPLIT_WMp_BASE, NonTerminal::VM, {SplitKind::K}},
    {RuleId::VM_SCALE2, NonTerminal::VM, {SplitKind::None}},

    // WMv / WMp
    {RuleId::WMv_STEM_V, NonTerminal::WMv, {SplitKind::None}},
    {RuleId::WMv_EXTEND_UNPAIRED, NonTerminal::WMv, {SplitKind::None}},
    {RuleId::WMp_STEM_WMB, NonTerminal::WMp, {SplitKind::None}},
    {RuleId::WMp_EXTEND_UNPAIRED, NonTerminal::WMp, {SplitKind::None}},

    // WM
    {RuleId::WM_START_V, NonTerminal::WM, {SplitKind::K}},
    {RuleId::WM_START_WMB, NonTerminal::WM, {SplitKind::K}},
    {RuleId::WM_SPLIT_V, NonTerminal::WM, {SplitKind::K}},
    {RuleId::WM_SPLIT_WMB, NonTerminal::WM, {SplitKind::K}},
    {RuleId::WM_EXTEND_UNPAIRED, NonTerminal::WM, {SplitKind::None}},

    // WIP
    {RuleId::WIP_BASE_V, NonTerminal::WIP, {SplitKind::None}},
    {RuleId::WIP_BASE_WMB, NonTerminal::WIP, {SplitKind::None}},
    {RuleId::WIP_SPLIT_V, NonTerminal::WIP, {SplitKind::K}},
    {RuleId::WIP_SPLIT_WMB, NonTerminal::WIP, {SplitKind::K}},
    {RuleId::WIP_BASEPAIR_V, NonTerminal::WIP, {SplitKind::K}},
    {RuleId::WIP_BASEPAIR_WMB, NonTerminal::WIP, {SplitKind::K}},
    {RuleId::WIP_EXTEND_UNPAIRED, NonTerminal::WIP, {SplitKind::None}},

    // VPL
    {RuleId::VPL_SPLIT_VP, NonTerminal::VPL, {SplitKind::K}},

    // VPR
    {RuleId::VPR_SPLIT_VP_WIP, NonTerminal::VPR, {SplitKind::K}},
    {RuleId::VPR_SPLIT_VP_BASEPAIR, NonTerminal::VPR, {SplitKind::K}},

    // VP
    {RuleId::VP_WI_CASE1, NonTerminal::VP, {SplitKind::None}},
    {RuleId::VP_WI_CASE2, NonTerminal::VP, {SplitKind::None}},
    {RuleId::VP_WI_CASE3, NonTerminal::VP, {SplitKind::None}},
    {RuleId::VP_STACK, NonTerminal::VP, {SplitKind::None}},
    {RuleId::VP_INTERNAL_LOOP, NonTerminal::VP, {SplitKind::KL}},
    {RuleId::VP_WIP_VP_LEFT, NonTerminal::VP, {SplitKind::K}},
    {RuleId::VP_VP_WIP_RIGHT, NonTerminal::VP, {SplitKind::K}},
    {RuleId::VP_WIP_VPR, NonTerminal::VP, {SplitKind::K}},
    {RuleId::VP_VPL_WIP, NonTerminal::VP, {SplitKind::K}},

    // WMBW
    {RuleId::WMBW_SPLIT_WMBP_WI, NonTerminal::WMBW, {SplitKind::K}},

    // WMBP
    {RuleId::WMBP_SPLIT_BE_WMBP_VP, NonTerminal::WMBP, {SplitKind::K}},
    {RuleId::WMBP_SPLIT_BE_WMBW_VP, NonTerminal::WMBP, {SplitKind::K}},
    {RuleId::WMBP_DIRECT_VP, NonTerminal::WMBP, {SplitKind::None}},
    {RuleId::WMBP_SPLIT_BE_WI_VP, NonTerminal::WMBP, {SplitKind::K}},

    // WMB
    {RuleId::WMB_SPLIT_BE_WMBP_WI, NonTerminal::WMB, {SplitKind::K}},
    {RuleId::WMB_DIRECT_WMBP, NonTerminal::WMB, {SplitKind::None}},
    {RuleId::WMB_EMPTY, NonTerminal::WMB, {SplitKind::None}},

    // BE
    {RuleId::BE_BASE_SAMEPAIR, NonTerminal::BE, {SplitKind::None}},
    {RuleId::BE_STACK, NonTerminal::BE, {SplitKind::None}},
    {RuleId::BE_INTERNAL_LOOP, NonTerminal::BE, {SplitKind::KL}},
    {RuleId::BE_WIP_WIP, NonTerminal::BE, {SplitKind::K}},
    {RuleId::BE_WIP_BASEPAIR, NonTerminal::BE, {SplitKind::K}},
    {RuleId::BE_BASEPAIR_WIP, NonTerminal::BE, {SplitKind::K}},
};

std::array<std::vector<RuleId>, kNonTerminalCount> build_rules_for() {
    std::array<std::vector<RuleId>, kNonTerminalCount> rules;
    for (const auto &spec : kRuleSpecs) {
        rules[static_cast<size_t>(spec.lhs)].push_back(spec.id);
    }
    return rules;
}

} // namespace

const std::vector<RuleId> &rules_for(NonTerminal nonterminal) {
    static_assert(static_cast<size_t>(NonTerminal::BE) + 1 == kNonTerminalCount,
                  "NonTerminal count mismatch");
    static const std::array<std::vector<RuleId>, kNonTerminalCount> rules = build_rules_for();
    return rules[static_cast<size_t>(nonterminal)];
}

} // namespace scfg
