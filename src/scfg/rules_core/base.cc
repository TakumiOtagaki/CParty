#include "scfg/rules_core.hh"

#include <array>

namespace scfg {
namespace {

// ルール定義の「唯一の正本」。
// - RuleId と NonTerminal / SplitSpec の対応表
// - NonTerminal ごとのルール一覧生成
// - rule_spec() によるメタ情報参照
// 実際の分解ロジックは rules_core/basic_*.cc と rules_core/band_*.cc に分割している。

constexpr size_t kNonTerminalCount = 15;

constexpr SplitSpec split_spec(SplitKind kind, bool uses_p = false, bool uses_q = false) {
    return SplitSpec{kind, uses_p, uses_q};
}

const RuleSpec kRuleSpecs[] = {
    // V
    {RuleId::V_HAIRPIN, NonTerminal::V, split_spec(SplitKind::None)},
    {RuleId::V_INTERNAL, NonTerminal::V, split_spec(SplitKind::KL)},
    {RuleId::V_VM, NonTerminal::V, split_spec(SplitKind::None)},

    // WI
    {RuleId::WI_BASE_SINGLE, NonTerminal::WI, split_spec(SplitKind::None)},
    {RuleId::WI_SPLIT_V, NonTerminal::WI, split_spec(SplitKind::K)},
    {RuleId::WI_SPLIT_WMB, NonTerminal::WI, split_spec(SplitKind::K)},
    {RuleId::WI_EXTEND_UNPAIRED, NonTerminal::WI, split_spec(SplitKind::None)},

    // W
    {RuleId::W_EXTEND_UNPAIRED, NonTerminal::W, split_spec(SplitKind::None)},
    {RuleId::W_SPLIT_V, NonTerminal::W, split_spec(SplitKind::K)},
    {RuleId::W_SPLIT_WMB, NonTerminal::W, split_spec(SplitKind::K)},

    // VM
    {RuleId::VM_SPLIT_WM_WMv, NonTerminal::VM, split_spec(SplitKind::K)},
    {RuleId::VM_SPLIT_WM_WMp, NonTerminal::VM, split_spec(SplitKind::K)},
    {RuleId::VM_SPLIT_WMp_BASE, NonTerminal::VM, split_spec(SplitKind::K)},
    {RuleId::VM_SCALE2, NonTerminal::VM, split_spec(SplitKind::None)},

    // WMv / WMp
    {RuleId::WMv_STEM_V, NonTerminal::WMv, split_spec(SplitKind::None)},
    {RuleId::WMv_EXTEND_UNPAIRED, NonTerminal::WMv, split_spec(SplitKind::None)},
    {RuleId::WMp_STEM_WMB, NonTerminal::WMp, split_spec(SplitKind::None)},
    {RuleId::WMp_EXTEND_UNPAIRED, NonTerminal::WMp, split_spec(SplitKind::None)},

    // WM
    {RuleId::WM_START_V, NonTerminal::WM, split_spec(SplitKind::K)},
    {RuleId::WM_START_WMB, NonTerminal::WM, split_spec(SplitKind::K)},
    {RuleId::WM_SPLIT_V, NonTerminal::WM, split_spec(SplitKind::K)},
    {RuleId::WM_SPLIT_WMB, NonTerminal::WM, split_spec(SplitKind::K)},
    {RuleId::WM_EXTEND_UNPAIRED, NonTerminal::WM, split_spec(SplitKind::None)},

    // WIP
    {RuleId::WIP_BASE_V, NonTerminal::WIP, split_spec(SplitKind::None)},
    {RuleId::WIP_BASE_WMB, NonTerminal::WIP, split_spec(SplitKind::None)},
    {RuleId::WIP_SPLIT_V, NonTerminal::WIP, split_spec(SplitKind::K)},
    {RuleId::WIP_SPLIT_WMB, NonTerminal::WIP, split_spec(SplitKind::K)},
    {RuleId::WIP_BASEPAIR_V, NonTerminal::WIP, split_spec(SplitKind::K)},
    {RuleId::WIP_BASEPAIR_WMB, NonTerminal::WIP, split_spec(SplitKind::K)},
    {RuleId::WIP_EXTEND_UNPAIRED, NonTerminal::WIP, split_spec(SplitKind::None)},

    // VPL
    {RuleId::VPL_SPLIT_VP, NonTerminal::VPL, split_spec(SplitKind::K)},

    // VPR
    {RuleId::VPR_SPLIT_VP_WIP, NonTerminal::VPR, split_spec(SplitKind::K)},
    {RuleId::VPR_SPLIT_VP_BASEPAIR, NonTerminal::VPR, split_spec(SplitKind::K)},

    // VP
    {RuleId::VP_WI_CASE1, NonTerminal::VP, split_spec(SplitKind::None, true, true)},
    {RuleId::VP_WI_CASE2, NonTerminal::VP, split_spec(SplitKind::None, true, true)},
    {RuleId::VP_WI_CASE3, NonTerminal::VP, split_spec(SplitKind::None, true, true)},
    {RuleId::VP_STACK, NonTerminal::VP, split_spec(SplitKind::None)},
    {RuleId::VP_INTERNAL_LOOP, NonTerminal::VP, split_spec(SplitKind::KL)},
    {RuleId::VP_WIP_VP_LEFT, NonTerminal::VP, split_spec(SplitKind::K)},
    {RuleId::VP_VP_WIP_RIGHT, NonTerminal::VP, split_spec(SplitKind::K)},
    {RuleId::VP_WIP_VPR, NonTerminal::VP, split_spec(SplitKind::K)},
    {RuleId::VP_VPL_WIP, NonTerminal::VP, split_spec(SplitKind::K)},

    // WMBW
    {RuleId::WMBW_SPLIT_WMBP_WI, NonTerminal::WMBW, split_spec(SplitKind::K)},

    // WMBP
    {RuleId::WMBP_SPLIT_BE_WMBP_VP, NonTerminal::WMBP, split_spec(SplitKind::K, true, true)},
    {RuleId::WMBP_SPLIT_BE_WMBW_VP, NonTerminal::WMBP, split_spec(SplitKind::K, true, true)},
    {RuleId::WMBP_DIRECT_VP, NonTerminal::WMBP, split_spec(SplitKind::None)},
    {RuleId::WMBP_SPLIT_BE_WI_VP, NonTerminal::WMBP, split_spec(SplitKind::K, true, false)},

    // WMB
    {RuleId::WMB_SPLIT_BE_WMBP_WI, NonTerminal::WMB, split_spec(SplitKind::K, true, true)},
    {RuleId::WMB_DIRECT_WMBP, NonTerminal::WMB, split_spec(SplitKind::None)},
    {RuleId::WMB_EMPTY, NonTerminal::WMB, split_spec(SplitKind::None)},

    // BE
    {RuleId::BE_BASE_SAMEPAIR, NonTerminal::BE, split_spec(SplitKind::None)},
    {RuleId::BE_STACK, NonTerminal::BE, split_spec(SplitKind::None)},
    {RuleId::BE_INTERNAL_LOOP, NonTerminal::BE, split_spec(SplitKind::KL)},
    {RuleId::BE_WIP_WIP, NonTerminal::BE, split_spec(SplitKind::K)},
    {RuleId::BE_WIP_BASEPAIR, NonTerminal::BE, split_spec(SplitKind::K)},
    {RuleId::BE_BASEPAIR_WIP, NonTerminal::BE, split_spec(SplitKind::K)},
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

const RuleSpec &rule_spec(RuleId rule) {
    static_assert(sizeof(kRuleSpecs) / sizeof(kRuleSpecs[0]) == kRuleIdCount,
                  "RuleSpec count mismatch");
    static const std::array<RuleSpec, kRuleIdCount> specs = []() {
        std::array<RuleSpec, kRuleIdCount> out{};
        for (size_t i = 0; i < kRuleIdCount; ++i) {
            out[i] = kRuleSpecs[i];
        }
        return out;
    }();
    return specs[static_cast<size_t>(rule)];
}

} // namespace scfg
