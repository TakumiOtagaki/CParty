#include "scfg/rules_core.hh"

namespace scfg {
namespace {

const std::vector<RuleId> &empty_rules() {
    static const std::vector<RuleId> empty;
    return empty;
}

} // namespace

const std::vector<RuleId> &rules_for(NonTerminal nonterminal) {
    static const std::vector<RuleId> kV = {
        RuleId::V_HAIRPIN,
        RuleId::V_INTERNAL,
        RuleId::V_VM,
    };
    static const std::vector<RuleId> kWI = {
        RuleId::WI_BASE_SINGLE,
        RuleId::WI_SPLIT_V,
        RuleId::WI_SPLIT_WMB,
        RuleId::WI_EXTEND_UNPAIRED,
    };
    static const std::vector<RuleId> kW = {
        RuleId::W_EXTEND_UNPAIRED,
        RuleId::W_SPLIT_V,
        RuleId::W_SPLIT_WMB,
    };
    static const std::vector<RuleId> kVM = {
        RuleId::VM_SPLIT_WM_WMv,
        RuleId::VM_SPLIT_WM_WMp,
        RuleId::VM_SPLIT_WMp_BASE,
        RuleId::VM_SCALE2,
    };
    static const std::vector<RuleId> kWMv = {
        RuleId::WMv_STEM_V,
        RuleId::WMv_EXTEND_UNPAIRED,
    };
    static const std::vector<RuleId> kWMp = {
        RuleId::WMp_STEM_WMB,
        RuleId::WMp_EXTEND_UNPAIRED,
    };
    static const std::vector<RuleId> kWM = {
        RuleId::WM_START_V,
        RuleId::WM_START_WMB,
        RuleId::WM_SPLIT_V,
        RuleId::WM_SPLIT_WMB,
        RuleId::WM_EXTEND_UNPAIRED,
    };
    static const std::vector<RuleId> kWIP = {
        RuleId::WIP_BASE_V,
        RuleId::WIP_BASE_WMB,
        RuleId::WIP_SPLIT_V,
        RuleId::WIP_SPLIT_WMB,
        RuleId::WIP_BASEPAIR_V,
        RuleId::WIP_BASEPAIR_WMB,
        RuleId::WIP_EXTEND_UNPAIRED,
    };
    static const std::vector<RuleId> kVPL = {
        RuleId::VPL_SPLIT_VP,
    };
    static const std::vector<RuleId> kVPR = {
        RuleId::VPR_SPLIT_VP_WIP,
        RuleId::VPR_SPLIT_VP_BASEPAIR,
    };
    static const std::vector<RuleId> kVP = {
        RuleId::VP_WI_CASE1,
        RuleId::VP_WI_CASE2,
        RuleId::VP_WI_CASE3,
        RuleId::VP_STACK,
        RuleId::VP_INTERNAL_LOOP,
        RuleId::VP_WIP_VP_LEFT,
        RuleId::VP_VP_WIP_RIGHT,
        RuleId::VP_WIP_VPR,
        RuleId::VP_VPL_WIP,
    };
    static const std::vector<RuleId> kWMBW = {
        RuleId::WMBW_SPLIT_WMBP_WI,
    };
    static const std::vector<RuleId> kWMBP = {
        RuleId::WMBP_SPLIT_BE_WMBP_VP,
        RuleId::WMBP_SPLIT_BE_WMBW_VP,
        RuleId::WMBP_DIRECT_VP,
        RuleId::WMBP_SPLIT_BE_WI_VP,
    };
    static const std::vector<RuleId> kWMB = {
        RuleId::WMB_SPLIT_BE_WMBP_WI,
        RuleId::WMB_DIRECT_WMBP,
        RuleId::WMB_EMPTY,
    };
    static const std::vector<RuleId> kBE = {
        RuleId::BE_BASE_SAMEPAIR,
        RuleId::BE_STACK,
        RuleId::BE_INTERNAL_LOOP,
        RuleId::BE_WIP_WIP,
        RuleId::BE_WIP_BASEPAIR,
        RuleId::BE_BASEPAIR_WIP,
    };

    switch (nonterminal) {
    case NonTerminal::V:
        return kV;
    case NonTerminal::WI:
        return kWI;
    case NonTerminal::W:
        return kW;
    case NonTerminal::VM:
        return kVM;
    case NonTerminal::WMv:
        return kWMv;
    case NonTerminal::WMp:
        return kWMp;
    case NonTerminal::WM:
        return kWM;
    case NonTerminal::WIP:
        return kWIP;
    case NonTerminal::VPL:
        return kVPL;
    case NonTerminal::VPR:
        return kVPR;
    case NonTerminal::VP:
        return kVP;
    case NonTerminal::WMBW:
        return kWMBW;
    case NonTerminal::WMBP:
        return kWMBP;
    case NonTerminal::WMB:
        return kWMB;
    case NonTerminal::BE:
        return kBE;
    }

    return empty_rules();
}

} // namespace scfg
