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

constexpr EndpointRef ep(Endpoint base, int offset = 0) {
    return EndpointRef{base, offset};
}

constexpr SpanSpec span(Endpoint base_left, int offset_left, Endpoint base_right, int offset_right) {
    return SpanSpec{ep(base_left, offset_left), ep(base_right, offset_right)};
}

constexpr RuleChildSpec child(NonTerminal nonterminal,
                              Endpoint base_left,
                              int offset_left,
                              Endpoint base_right,
                              int offset_right,
                              bool allow_empty = false) {
    return RuleChildSpec{nonterminal, span(base_left, offset_left, base_right, offset_right), allow_empty};
}

constexpr RuleSpec rule_spec(RuleId id,
                             NonTerminal lhs,
                             SplitSpec split,
                             const RuleChildSpec *rhs = nullptr,
                             size_t rhs_len = 0,
                             RuleSpec::SplitGenKind split_gen = RuleSpec::SplitGenKind::Custom,
                             RuleSpec::SplitRangeSpec split_range = {}) {
    return RuleSpec{id, lhs, split, rhs, rhs_len, split_gen, split_range};
}

constexpr RuleSpec::SplitRangeSpec split_range(Endpoint start_base,
                                               int start_offset,
                                               Endpoint end_base,
                                               int end_offset,
                                               bool end_inclusive,
                                               bool subtract_turn) {
    return RuleSpec::SplitRangeSpec{ep(start_base, start_offset), ep(end_base, end_offset), end_inclusive, subtract_turn};
}

// RHS: WI
constexpr RuleChildSpec kWiSplitVRhs[] = {
    child(NonTerminal::WI, Endpoint::I, 0, Endpoint::K, -1),
    child(NonTerminal::V, Endpoint::K, 0, Endpoint::J, 0),
};
constexpr RuleChildSpec kWiSplitWmbRhs[] = {
    child(NonTerminal::WI, Endpoint::I, 0, Endpoint::K, -1),
    child(NonTerminal::WMB, Endpoint::K, 0, Endpoint::J, 0),
};
constexpr RuleChildSpec kWiExtendUnpairedRhs[] = {
    child(NonTerminal::WI, Endpoint::I, 0, Endpoint::J, -1),
};

// RHS: W
constexpr RuleChildSpec kWExtendUnpairedRhs[] = {
    child(NonTerminal::W, Endpoint::I, 0, Endpoint::J, -1),
};
constexpr RuleChildSpec kWSplitVRhs[] = {
    child(NonTerminal::W, Endpoint::I, 0, Endpoint::K, -1),
    child(NonTerminal::V, Endpoint::K, 0, Endpoint::J, 0),
};
constexpr RuleChildSpec kWSplitWmbRhs[] = {
    child(NonTerminal::W, Endpoint::I, 0, Endpoint::K, -1),
    child(NonTerminal::WMB, Endpoint::K, 0, Endpoint::J, 0),
};

// RHS: VM
constexpr RuleChildSpec kVmSplitWmWmvRhs[] = {
    // WM(i+1..k-1) can be empty when k=i+1.
    child(NonTerminal::WM, Endpoint::I, 1, Endpoint::K, -1, true),
    child(NonTerminal::WMv, Endpoint::K, 0, Endpoint::J, -1),
};
constexpr RuleChildSpec kVmSplitWmWmpRhs[] = {
    // WM(i+1..k-1) can be empty when k=i+1.
    child(NonTerminal::WM, Endpoint::I, 1, Endpoint::K, -1, true),
    child(NonTerminal::WMp, Endpoint::K, 0, Endpoint::J, -1),
};
constexpr RuleChildSpec kVmSplitWmpBaseRhs[] = {
    child(NonTerminal::WMp, Endpoint::K, 0, Endpoint::J, -1),
};

// RHS: WIP
constexpr RuleChildSpec kWipBaseVRhs[] = {
    child(NonTerminal::V, Endpoint::I, 0, Endpoint::J, 0),
};
constexpr RuleChildSpec kWipBaseWmbRhs[] = {
    child(NonTerminal::WMB, Endpoint::I, 0, Endpoint::J, 0),
};
constexpr RuleChildSpec kWipSplitVRhs[] = {
    child(NonTerminal::WIP, Endpoint::I, 0, Endpoint::K, -1),
    child(NonTerminal::V, Endpoint::K, 0, Endpoint::J, 0),
};
constexpr RuleChildSpec kWipSplitWmbRhs[] = {
    child(NonTerminal::WIP, Endpoint::I, 0, Endpoint::K, -1),
    child(NonTerminal::WMB, Endpoint::K, 0, Endpoint::J, 0),
};
constexpr RuleChildSpec kWipBasepairVRhs[] = {
    child(NonTerminal::V, Endpoint::K, 0, Endpoint::J, 0),
};
constexpr RuleChildSpec kWipBasepairWmbRhs[] = {
    child(NonTerminal::WMB, Endpoint::K, 0, Endpoint::J, 0),
};
constexpr RuleChildSpec kWipExtendUnpairedRhs[] = {
    // WIP(i..j-1) can be empty when i==j.
    child(NonTerminal::WIP, Endpoint::I, 0, Endpoint::J, -1, true),
};

// RHS: VPL
constexpr RuleChildSpec kVplSplitVpRhs[] = {
    child(NonTerminal::VP, Endpoint::K, 0, Endpoint::J, 0),
};

// RHS: VPR
constexpr RuleChildSpec kVprSplitVpWipRhs[] = {
    child(NonTerminal::VP, Endpoint::I, 0, Endpoint::K, 0),
    child(NonTerminal::WIP, Endpoint::K, 1, Endpoint::J, 0),
};
constexpr RuleChildSpec kVprSplitVpBasepairRhs[] = {
    child(NonTerminal::VP, Endpoint::I, 0, Endpoint::K, 0),
};

// RHS: VP
constexpr RuleChildSpec kVpWiCase1Rhs[] = {
    // WI segments can be empty when p/q are adjacent to i/j.
    child(NonTerminal::WI, Endpoint::I, 1, Endpoint::P, -1, true),
    child(NonTerminal::WI, Endpoint::Q, 1, Endpoint::J, -1, true),
};
constexpr RuleChildSpec kVpWiCase2Rhs[] = {
    // WI segments can be empty when p/q are adjacent to i/j.
    child(NonTerminal::WI, Endpoint::I, 1, Endpoint::P, -1, true),
    child(NonTerminal::WI, Endpoint::Q, 1, Endpoint::J, -1, true),
};
constexpr RuleChildSpec kVpWiCase3Rhs[] = {
    // WI segments can be empty when borders are adjacent.
    child(NonTerminal::WI, Endpoint::I, 1, Endpoint::P, -1, true),
    child(NonTerminal::WI, Endpoint::Q, 1, Endpoint::K, -1, true),
    child(NonTerminal::WI, Endpoint::L, 1, Endpoint::J, -1, true),
};
constexpr RuleChildSpec kVpStackRhs[] = {
    child(NonTerminal::VP, Endpoint::I, 1, Endpoint::J, -1),
};
constexpr RuleChildSpec kVpInternalLoopRhs[] = {
    child(NonTerminal::VP, Endpoint::K, 0, Endpoint::L, 0),
};
constexpr RuleChildSpec kVpWipVpLeftRhs[] = {
    // WIP(i+1..k-1) can be empty when k=i+1.
    child(NonTerminal::WIP, Endpoint::I, 1, Endpoint::K, -1, true),
    child(NonTerminal::VP, Endpoint::K, 0, Endpoint::J, -1),
};
constexpr RuleChildSpec kVpVpWipRightRhs[] = {
    child(NonTerminal::VP, Endpoint::I, 1, Endpoint::K, 0),
    // WIP(k+1..j-1) can be empty when k=j-1.
    child(NonTerminal::WIP, Endpoint::K, 1, Endpoint::J, -1, true),
};
constexpr RuleChildSpec kVpWipVprRhs[] = {
    // WIP(i+1..k-1) can be empty when k=i+1.
    child(NonTerminal::WIP, Endpoint::I, 1, Endpoint::K, -1, true),
    child(NonTerminal::VPR, Endpoint::K, 0, Endpoint::J, -1),
};
constexpr RuleChildSpec kVpVplWipRhs[] = {
    child(NonTerminal::VPL, Endpoint::I, 1, Endpoint::K, 0),
    // WIP(k+1..j-1) can be empty when k=j-1.
    child(NonTerminal::WIP, Endpoint::K, 1, Endpoint::J, -1, true),
};

// RHS: WMBW
constexpr RuleChildSpec kWmbwSplitWmbpWiRhs[] = {
    child(NonTerminal::WMBP, Endpoint::I, 0, Endpoint::K, 0),
    child(NonTerminal::WI, Endpoint::K, 1, Endpoint::J, 0),
};

// RHS: WMBP
constexpr RuleChildSpec kWmbpSplitBeWmbpVpRhs[] = {
    child(NonTerminal::WMBP, Endpoint::I, 0, Endpoint::K, -1),
    child(NonTerminal::VP, Endpoint::K, 0, Endpoint::J, 0),
};
constexpr RuleChildSpec kWmbpSplitBeWmbwVpRhs[] = {
    child(NonTerminal::WMBW, Endpoint::I, 0, Endpoint::K, -1),
    child(NonTerminal::VP, Endpoint::K, 0, Endpoint::J, 0),
};
constexpr RuleChildSpec kWmbpDirectVpRhs[] = {
    child(NonTerminal::VP, Endpoint::I, 0, Endpoint::J, 0),
};
constexpr RuleChildSpec kWmbpSplitBeWiVpRhs[] = {
    // WI(p+1..k-1) can be empty when k=p+1.
    child(NonTerminal::WI, Endpoint::P, 1, Endpoint::K, -1, true),
    child(NonTerminal::VP, Endpoint::K, 0, Endpoint::J, 0),
};

// RHS: WMB
constexpr RuleChildSpec kWmbDirectWmbpRhs[] = {
    child(NonTerminal::WMBP, Endpoint::I, 0, Endpoint::J, 0),
};
constexpr RuleChildSpec kWmbSplitBeWmbpWiRhs[] = {
    child(NonTerminal::WMBP, Endpoint::I, 0, Endpoint::K, 0),
    // WI(k+1..q-1) can be empty when k+1==q.
    child(NonTerminal::WI, Endpoint::K, 1, Endpoint::Q, -1, true),
};

// RHS: BE
constexpr RuleChildSpec kBeStackRhs[] = {
    child(NonTerminal::BE, Endpoint::I, 1, Endpoint::J, -1),
};
constexpr RuleChildSpec kBeInternalLoopRhs[] = {
    child(NonTerminal::BE, Endpoint::K, 0, Endpoint::L, 0),
};
constexpr RuleChildSpec kBeWipWipRhs[] = {
    // WIP ranges can be empty when borders touch.
    child(NonTerminal::WIP, Endpoint::I, 1, Endpoint::K, -1, true),
    child(NonTerminal::BE, Endpoint::K, 0, Endpoint::L, 0),
    child(NonTerminal::WIP, Endpoint::L, 1, Endpoint::J, -1, true),
};
constexpr RuleChildSpec kBeWipBasepairRhs[] = {
    // WIP(i+1..k-1) can be empty when k=i+1.
    child(NonTerminal::WIP, Endpoint::I, 1, Endpoint::K, -1, true),
    child(NonTerminal::BE, Endpoint::K, 0, Endpoint::L, 0),
};
constexpr RuleChildSpec kBeBasepairWipRhs[] = {
    child(NonTerminal::BE, Endpoint::K, 0, Endpoint::L, 0),
    // WIP(l+1..j-1) can be empty when l=j-1.
    child(NonTerminal::WIP, Endpoint::L, 1, Endpoint::J, -1, true),
};

const RuleSpec kRuleSpecs[] = {
    // V
    rule_spec(RuleId::V_HAIRPIN, NonTerminal::V, split_spec(SplitKind::None)),
    rule_spec(RuleId::V_INTERNAL, NonTerminal::V, split_spec(SplitKind::KL)),
    rule_spec(RuleId::V_VM, NonTerminal::V, split_spec(SplitKind::None)),

    // WI
    rule_spec(RuleId::WI_BASE_SINGLE, NonTerminal::WI, split_spec(SplitKind::None)),
    rule_spec(RuleId::WI_SPLIT_V,
              NonTerminal::WI,
              split_spec(SplitKind::K),
              kWiSplitVRhs,
              2,
              RuleSpec::SplitGenKind::KRange,
              split_range(Endpoint::I, 0, Endpoint::J, -1, true, true)),
    rule_spec(RuleId::WI_SPLIT_WMB,
              NonTerminal::WI,
              split_spec(SplitKind::K),
              kWiSplitWmbRhs,
              2,
              RuleSpec::SplitGenKind::KRange,
              split_range(Endpoint::I, 0, Endpoint::J, -1, true, true)),
    rule_spec(RuleId::WI_EXTEND_UNPAIRED, NonTerminal::WI, split_spec(SplitKind::None), kWiExtendUnpairedRhs, 1),

    // W
    rule_spec(RuleId::W_EXTEND_UNPAIRED, NonTerminal::W, split_spec(SplitKind::None), kWExtendUnpairedRhs, 1),
    rule_spec(RuleId::W_SPLIT_V, NonTerminal::W, split_spec(SplitKind::K), kWSplitVRhs, 2),
    rule_spec(RuleId::W_SPLIT_WMB, NonTerminal::W, split_spec(SplitKind::K), kWSplitWmbRhs, 2),

    // VM
    rule_spec(RuleId::VM_SPLIT_WM_WMv,
              NonTerminal::VM,
              split_spec(SplitKind::K),
              kVmSplitWmWmvRhs,
              2,
              RuleSpec::SplitGenKind::KRange,
              split_range(Endpoint::I, 1, Endpoint::J, -1, true, true)),
    rule_spec(RuleId::VM_SPLIT_WM_WMp,
              NonTerminal::VM,
              split_spec(SplitKind::K),
              kVmSplitWmWmpRhs,
              2,
              RuleSpec::SplitGenKind::KRange,
              split_range(Endpoint::I, 1, Endpoint::J, -1, true, true)),
    rule_spec(RuleId::VM_SPLIT_WMp_BASE, NonTerminal::VM, split_spec(SplitKind::K), kVmSplitWmpBaseRhs, 1),
    rule_spec(RuleId::VM_SCALE2, NonTerminal::VM, split_spec(SplitKind::None)),

    // WMv / WMp
    rule_spec(RuleId::WMv_STEM_V, NonTerminal::WMv, split_spec(SplitKind::None)),
    rule_spec(RuleId::WMv_EXTEND_UNPAIRED, NonTerminal::WMv, split_spec(SplitKind::None)),
    rule_spec(RuleId::WMp_STEM_WMB, NonTerminal::WMp, split_spec(SplitKind::None)),
    rule_spec(RuleId::WMp_EXTEND_UNPAIRED, NonTerminal::WMp, split_spec(SplitKind::None)),

    // WM
    rule_spec(RuleId::WM_START_V, NonTerminal::WM, split_spec(SplitKind::K)),
    rule_spec(RuleId::WM_START_WMB, NonTerminal::WM, split_spec(SplitKind::K)),
    rule_spec(RuleId::WM_SPLIT_V, NonTerminal::WM, split_spec(SplitKind::K)),
    rule_spec(RuleId::WM_SPLIT_WMB, NonTerminal::WM, split_spec(SplitKind::K)),
    rule_spec(RuleId::WM_EXTEND_UNPAIRED, NonTerminal::WM, split_spec(SplitKind::None)),

    // WIP
    rule_spec(RuleId::WIP_BASE_V, NonTerminal::WIP, split_spec(SplitKind::None), kWipBaseVRhs, 1),
    rule_spec(RuleId::WIP_BASE_WMB, NonTerminal::WIP, split_spec(SplitKind::None), kWipBaseWmbRhs, 1),
    rule_spec(RuleId::WIP_SPLIT_V,
              NonTerminal::WIP,
              split_spec(SplitKind::K),
              kWipSplitVRhs,
              2,
              RuleSpec::SplitGenKind::KRange,
              split_range(Endpoint::I, 1, Endpoint::J, -1, false, true)),
    rule_spec(RuleId::WIP_SPLIT_WMB,
              NonTerminal::WIP,
              split_spec(SplitKind::K),
              kWipSplitWmbRhs,
              2,
              RuleSpec::SplitGenKind::KRange,
              split_range(Endpoint::I, 1, Endpoint::J, -1, false, true)),
    rule_spec(RuleId::WIP_BASEPAIR_V, NonTerminal::WIP, split_spec(SplitKind::K), kWipBasepairVRhs, 1),
    rule_spec(RuleId::WIP_BASEPAIR_WMB, NonTerminal::WIP, split_spec(SplitKind::K), kWipBasepairWmbRhs, 1),
    rule_spec(RuleId::WIP_EXTEND_UNPAIRED, NonTerminal::WIP, split_spec(SplitKind::None), kWipExtendUnpairedRhs, 1),

    // VPL
    rule_spec(RuleId::VPL_SPLIT_VP, NonTerminal::VPL, split_spec(SplitKind::K), kVplSplitVpRhs, 1),

    // VPR
    rule_spec(RuleId::VPR_SPLIT_VP_WIP, NonTerminal::VPR, split_spec(SplitKind::K), kVprSplitVpWipRhs, 2),
    rule_spec(RuleId::VPR_SPLIT_VP_BASEPAIR, NonTerminal::VPR, split_spec(SplitKind::K), kVprSplitVpBasepairRhs, 1),

    // VP
    rule_spec(RuleId::VP_WI_CASE1, NonTerminal::VP, split_spec(SplitKind::None, true, true), kVpWiCase1Rhs, 2),
    rule_spec(RuleId::VP_WI_CASE2, NonTerminal::VP, split_spec(SplitKind::None, true, true), kVpWiCase2Rhs, 2),
    rule_spec(RuleId::VP_WI_CASE3, NonTerminal::VP, split_spec(SplitKind::None, true, true), kVpWiCase3Rhs, 3),
    rule_spec(RuleId::VP_STACK, NonTerminal::VP, split_spec(SplitKind::None), kVpStackRhs, 1),
    rule_spec(RuleId::VP_INTERNAL_LOOP, NonTerminal::VP, split_spec(SplitKind::KL), kVpInternalLoopRhs, 1),
    rule_spec(RuleId::VP_WIP_VP_LEFT, NonTerminal::VP, split_spec(SplitKind::K), kVpWipVpLeftRhs, 2),
    rule_spec(RuleId::VP_VP_WIP_RIGHT, NonTerminal::VP, split_spec(SplitKind::K), kVpVpWipRightRhs, 2),
    rule_spec(RuleId::VP_WIP_VPR, NonTerminal::VP, split_spec(SplitKind::K), kVpWipVprRhs, 2),
    rule_spec(RuleId::VP_VPL_WIP, NonTerminal::VP, split_spec(SplitKind::K), kVpVplWipRhs, 2),

    // WMBW
    rule_spec(RuleId::WMBW_SPLIT_WMBP_WI, NonTerminal::WMBW, split_spec(SplitKind::K), kWmbwSplitWmbpWiRhs, 2),

    // WMBP
    rule_spec(RuleId::WMBP_SPLIT_BE_WMBP_VP, NonTerminal::WMBP, split_spec(SplitKind::K, true, true),
              kWmbpSplitBeWmbpVpRhs, 2),
    rule_spec(RuleId::WMBP_SPLIT_BE_WMBW_VP, NonTerminal::WMBP, split_spec(SplitKind::K, true, true),
              kWmbpSplitBeWmbwVpRhs, 2),
    rule_spec(RuleId::WMBP_DIRECT_VP, NonTerminal::WMBP, split_spec(SplitKind::None), kWmbpDirectVpRhs, 1),
    rule_spec(RuleId::WMBP_SPLIT_BE_WI_VP, NonTerminal::WMBP, split_spec(SplitKind::K, true, false),
              kWmbpSplitBeWiVpRhs, 2),

    // WMB
    rule_spec(RuleId::WMB_SPLIT_BE_WMBP_WI, NonTerminal::WMB, split_spec(SplitKind::K, true, true),
              kWmbSplitBeWmbpWiRhs, 2),
    rule_spec(RuleId::WMB_DIRECT_WMBP, NonTerminal::WMB, split_spec(SplitKind::None), kWmbDirectWmbpRhs, 1),
    rule_spec(RuleId::WMB_EMPTY, NonTerminal::WMB, split_spec(SplitKind::None)),

    // BE
    rule_spec(RuleId::BE_BASE_SAMEPAIR, NonTerminal::BE, split_spec(SplitKind::None)),
    rule_spec(RuleId::BE_STACK, NonTerminal::BE, split_spec(SplitKind::None), kBeStackRhs, 1),
    rule_spec(RuleId::BE_INTERNAL_LOOP, NonTerminal::BE, split_spec(SplitKind::KL), kBeInternalLoopRhs, 1),
    rule_spec(RuleId::BE_WIP_WIP, NonTerminal::BE, split_spec(SplitKind::K), kBeWipWipRhs, 3),
    rule_spec(RuleId::BE_WIP_BASEPAIR, NonTerminal::BE, split_spec(SplitKind::K), kBeWipBasepairRhs, 2),
    rule_spec(RuleId::BE_BASEPAIR_WIP, NonTerminal::BE, split_spec(SplitKind::K), kBeBasepairWipRhs, 2),
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

const std::vector<RuleSpec> &rule_catalog() {
    static const std::vector<RuleSpec> catalog = []() {
        std::vector<RuleSpec> out;
        out.reserve(kRuleIdCount);
        for (const auto &spec : kRuleSpecs) {
            out.push_back(spec);
        }
        return out;
    }();
    return catalog;
}

} // namespace scfg
