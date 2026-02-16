#include "scfg/rules_core.hh"

#include "scfg/rules_part_func.hh"
#include "sparse_tree.hh"

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

std::vector<RuleSplit> enumerate_splits_w(RuleId rule,
                                          cand_pos_t i,
                                          cand_pos_t j,
                                          PartFuncWContext &ctx,
                                          sparse_tree &tree) {
    std::vector<RuleSplit> splits;
    switch (rule) {
    case RuleId::W_EXTEND_UNPAIRED:
        if (tree.tree[j].pair < 0) {
            splits.push_back({});
        }
        break;
    case RuleId::W_SPLIT_V:
    case RuleId::W_SPLIT_WMB: {
        if (!tree.weakly_closed(1, j)) {
            break;
        }
        const cand_pos_t turn = ctx.turn();
        for (cand_pos_t k = i; k <= j - turn - 1; ++k) {
            if (!tree.weakly_closed(1, k - 1)) continue;
            if (rule == RuleId::W_SPLIT_WMB && !(k == i || tree.weakly_closed(k, j))) continue;
            splits.push_back({k, -1});
        }
    } break;
    default:
        break;
    }
    return splits;
}

std::vector<RuleChild> expand_w(RuleId rule, cand_pos_t i, cand_pos_t j, const RuleSplit &split) {
    std::vector<RuleChild> children;
    switch (rule) {
    case RuleId::W_EXTEND_UNPAIRED:
        children.push_back({NonTerminal::W, i, j - 1});
        break;
    case RuleId::W_SPLIT_V:
        if (split.k > i) {
            children.push_back({NonTerminal::W, i, split.k - 1});
        }
        children.push_back({NonTerminal::V, split.k, j});
        break;
    case RuleId::W_SPLIT_WMB:
        if (split.k > i) {
            children.push_back({NonTerminal::W, i, split.k - 1});
        }
        children.push_back({NonTerminal::WMB, split.k, j});
        break;
    default:
        break;
    }
    return children;
}

pf_t rule_score_w(RuleId rule, cand_pos_t i, cand_pos_t j, const RuleSplit &split, PartFuncWContext &ctx) {
    (void)i;
    switch (rule) {
    case RuleId::W_EXTEND_UNPAIRED:
        return ctx.scale1();
    case RuleId::W_SPLIT_V:
        return ctx.exp_Extloop(split.k, j);
    case RuleId::W_SPLIT_WMB:
        return ctx.expPS_penalty();
    default:
        return 0;
    }
}

std::vector<RuleSplit> enumerate_splits_v(RuleId rule,
                                          cand_pos_t i,
                                          cand_pos_t j,
                                          PartFuncVContext &ctx,
                                          sparse_tree &tree) {
    (void)ctx;
    std::vector<RuleSplit> splits;
    const bool unpaired = (tree.tree[i].pair < -1 && tree.tree[j].pair < -1);
    const bool paired = (tree.tree[i].pair == j && tree.tree[j].pair == i);
    if (!(paired || unpaired)) {
        return splits;
    }

    switch (rule) {
    case RuleId::V_HAIRPIN: {
        const bool canH = !(tree.up[j - 1] < (j - i - 1));
        if (canH) {
            splits.push_back({});
        }
    } break;
    case RuleId::V_INTERNAL:
    case RuleId::V_VM:
        splits.push_back({});
        break;
    default:
        break;
    }
    return splits;
}

std::vector<RuleChild> expand_v(RuleId rule, cand_pos_t i, cand_pos_t j, const RuleSplit &split) {
    (void)rule;
    (void)i;
    (void)j;
    (void)split;
    return {};
}

pf_t rule_score_v(RuleId rule,
                  cand_pos_t i,
                  cand_pos_t j,
                  const RuleSplit &split,
                  PartFuncVContext &ctx,
                  sparse_tree &tree) {
    (void)split;
    switch (rule) {
    case RuleId::V_HAIRPIN:
        return ctx.hairpin_energy(i, j);
    case RuleId::V_INTERNAL:
        return ctx.internal_energy(i, j, tree.up);
    case RuleId::V_VM:
        return ctx.vm_energy(i, j, tree.up);
    default:
        return 0;
    }
}

std::vector<RuleSplit> enumerate_splits_wi(RuleId rule,
                                           cand_pos_t i,
                                           cand_pos_t j,
                                           PartFuncWIContext &ctx,
                                           sparse_tree &tree) {
    std::vector<RuleSplit> splits;
    if (i == j) {
        if (rule == RuleId::WI_BASE_SINGLE) {
            splits.push_back({});
        }
        return splits;
    }
    switch (rule) {
    case RuleId::WI_SPLIT_V:
    case RuleId::WI_SPLIT_WMB: {
        const cand_pos_t turn = ctx.turn();
        for (cand_pos_t k = i; k <= j - turn - 1; ++k) {
            splits.push_back({k, -1});
        }
    } break;
    case RuleId::WI_EXTEND_UNPAIRED:
        if (tree.tree[j].pair < 0) {
            splits.push_back({});
        }
        break;
    default:
        break;
    }
    return splits;
}

std::vector<RuleChild> expand_wi(RuleId rule, cand_pos_t i, cand_pos_t j, const RuleSplit &split) {
    std::vector<RuleChild> children;
    switch (rule) {
    case RuleId::WI_BASE_SINGLE:
        break;
    case RuleId::WI_SPLIT_V:
        if (split.k > i) {
            children.push_back({NonTerminal::WI, i, split.k - 1});
        }
        children.push_back({NonTerminal::V, split.k, j});
        break;
    case RuleId::WI_SPLIT_WMB:
        if (split.k > i) {
            children.push_back({NonTerminal::WI, i, split.k - 1});
        }
        children.push_back({NonTerminal::WMB, split.k, j});
        break;
    case RuleId::WI_EXTEND_UNPAIRED:
        children.push_back({NonTerminal::WI, i, j - 1});
        break;
    default:
        break;
    }
    return children;
}

pf_t rule_score_wi(RuleId rule,
                   cand_pos_t i,
                   cand_pos_t j,
                   const RuleSplit &split,
                   PartFuncWIContext &ctx) {
    (void)i;
    (void)j;
    (void)split;
    switch (rule) {
    case RuleId::WI_BASE_SINGLE:
        return ctx.expPUP_pen1();
    case RuleId::WI_SPLIT_V:
        return ctx.expPPS_penalty();
    case RuleId::WI_SPLIT_WMB:
        return ctx.expPSP_penalty() * ctx.expPPS_penalty();
    case RuleId::WI_EXTEND_UNPAIRED:
        return ctx.expPUP_pen1();
    default:
        return 0;
    }
}

} // namespace scfg
