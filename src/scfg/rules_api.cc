#include "scfg/rules_api.hh"

#include <string_view>

namespace scfg {

const char *rule_id_name(RuleId id) {
    switch (id) {
    case RuleId::V_HAIRPIN:
        return "V_HAIRPIN";
    case RuleId::V_INTERNAL:
        return "V_INTERNAL";
    case RuleId::V_VM:
        return "V_VM";
    case RuleId::WI_BASE_SINGLE:
        return "WI_BASE_SINGLE";
    case RuleId::WI_SPLIT_V:
        return "WI_SPLIT_V";
    case RuleId::WI_SPLIT_WMB:
        return "WI_SPLIT_WMB";
    case RuleId::WI_EXTEND_UNPAIRED:
        return "WI_EXTEND_UNPAIRED";
    case RuleId::W_EXTEND_UNPAIRED:
        return "W_EXTEND_UNPAIRED";
    case RuleId::W_SPLIT_V:
        return "W_SPLIT_V";
    case RuleId::W_SPLIT_WMB:
        return "W_SPLIT_WMB";
    case RuleId::VM_SPLIT_WM_WMv:
        return "VM_SPLIT_WM_WMv";
    case RuleId::VM_SPLIT_WM_WMp:
        return "VM_SPLIT_WM_WMp";
    case RuleId::VM_SPLIT_WMp_BASE:
        return "VM_SPLIT_WMp_BASE";
    case RuleId::VM_SCALE2:
        return "VM_SCALE2";
    case RuleId::WMv_STEM_V:
        return "WMv_STEM_V";
    case RuleId::WMp_STEM_WMB:
        return "WMp_STEM_WMB";
    case RuleId::WMv_EXTEND_UNPAIRED:
        return "WMv_EXTEND_UNPAIRED";
    case RuleId::WMp_EXTEND_UNPAIRED:
        return "WMp_EXTEND_UNPAIRED";
    case RuleId::WM_START_V:
        return "WM_START_V";
    case RuleId::WM_START_WMB:
        return "WM_START_WMB";
    case RuleId::WM_SPLIT_V:
        return "WM_SPLIT_V";
    case RuleId::WM_SPLIT_WMB:
        return "WM_SPLIT_WMB";
    case RuleId::WM_EXTEND_UNPAIRED:
        return "WM_EXTEND_UNPAIRED";
    case RuleId::WIP_BASE_V:
        return "WIP_BASE_V";
    case RuleId::WIP_BASE_WMB:
        return "WIP_BASE_WMB";
    case RuleId::WIP_SPLIT_V:
        return "WIP_SPLIT_V";
    case RuleId::WIP_SPLIT_WMB:
        return "WIP_SPLIT_WMB";
    case RuleId::WIP_BASEPAIR_V:
        return "WIP_BASEPAIR_V";
    case RuleId::WIP_BASEPAIR_WMB:
        return "WIP_BASEPAIR_WMB";
    case RuleId::WIP_EXTEND_UNPAIRED:
        return "WIP_EXTEND_UNPAIRED";
    case RuleId::VPL_SPLIT_VP:
        return "VPL_SPLIT_VP";
    case RuleId::VPR_SPLIT_VP_WIP:
        return "VPR_SPLIT_VP_WIP";
    case RuleId::VPR_SPLIT_VP_BASEPAIR:
        return "VPR_SPLIT_VP_BASEPAIR";
    case RuleId::VP_WI_CASE1:
        return "VP_WI_CASE1";
    case RuleId::VP_WI_CASE2:
        return "VP_WI_CASE2";
    case RuleId::VP_WI_CASE3:
        return "VP_WI_CASE3";
    case RuleId::VP_STACK:
        return "VP_STACK";
    case RuleId::VP_INTERNAL_LOOP:
        return "VP_INTERNAL_LOOP";
    case RuleId::VP_WIP_VP_LEFT:
        return "VP_WIP_VP_LEFT";
    case RuleId::VP_VP_WIP_RIGHT:
        return "VP_VP_WIP_RIGHT";
    case RuleId::VP_WIP_VPR:
        return "VP_WIP_VPR";
    case RuleId::VP_VPL_WIP:
        return "VP_VPL_WIP";
    case RuleId::WMBW_SPLIT_WMBP_WI:
        return "WMBW_SPLIT_WMBP_WI";
    case RuleId::WMBP_SPLIT_BE_WMBP_VP:
        return "WMBP_SPLIT_BE_WMBP_VP";
    case RuleId::WMBP_SPLIT_BE_WMBW_VP:
        return "WMBP_SPLIT_BE_WMBW_VP";
    case RuleId::WMBP_DIRECT_VP:
        return "WMBP_DIRECT_VP";
    case RuleId::WMBP_SPLIT_BE_WI_VP:
        return "WMBP_SPLIT_BE_WI_VP";
    case RuleId::WMB_SPLIT_BE_WMBP_WI:
        return "WMB_SPLIT_BE_WMBP_WI";
    case RuleId::WMB_DIRECT_WMBP:
        return "WMB_DIRECT_WMBP";
    case RuleId::WMB_EMPTY:
        return "WMB_EMPTY";
    case RuleId::BE_BASE_SAMEPAIR:
        return "BE_BASE_SAMEPAIR";
    case RuleId::BE_STACK:
        return "BE_STACK";
    case RuleId::BE_INTERNAL_LOOP:
        return "BE_INTERNAL_LOOP";
    case RuleId::BE_WIP_WIP:
        return "BE_WIP_WIP";
    case RuleId::BE_WIP_BASEPAIR:
        return "BE_WIP_BASEPAIR";
    case RuleId::BE_BASEPAIR_WIP:
        return "BE_BASEPAIR_WIP";
    }
    return "UNKNOWN";
}

bool parse_rule_id(std::string_view name, RuleId *out) {
    const auto match = [&](RuleId id) {
        return name == rule_id_name(id);
    };

    if (match(RuleId::V_HAIRPIN)) {
        *out = RuleId::V_HAIRPIN;
        return true;
    }
    if (match(RuleId::V_INTERNAL)) {
        *out = RuleId::V_INTERNAL;
        return true;
    }
    if (match(RuleId::V_VM)) {
        *out = RuleId::V_VM;
        return true;
    }
    if (match(RuleId::WI_BASE_SINGLE)) {
        *out = RuleId::WI_BASE_SINGLE;
        return true;
    }
    if (match(RuleId::WI_SPLIT_V)) {
        *out = RuleId::WI_SPLIT_V;
        return true;
    }
    if (match(RuleId::WI_SPLIT_WMB)) {
        *out = RuleId::WI_SPLIT_WMB;
        return true;
    }
    if (match(RuleId::WI_EXTEND_UNPAIRED)) {
        *out = RuleId::WI_EXTEND_UNPAIRED;
        return true;
    }
    if (match(RuleId::W_EXTEND_UNPAIRED)) {
        *out = RuleId::W_EXTEND_UNPAIRED;
        return true;
    }
    if (match(RuleId::W_SPLIT_V)) {
        *out = RuleId::W_SPLIT_V;
        return true;
    }
    if (match(RuleId::W_SPLIT_WMB)) {
        *out = RuleId::W_SPLIT_WMB;
        return true;
    }
    if (match(RuleId::VM_SPLIT_WM_WMv)) {
        *out = RuleId::VM_SPLIT_WM_WMv;
        return true;
    }
    if (match(RuleId::VM_SPLIT_WM_WMp)) {
        *out = RuleId::VM_SPLIT_WM_WMp;
        return true;
    }
    if (match(RuleId::VM_SPLIT_WMp_BASE)) {
        *out = RuleId::VM_SPLIT_WMp_BASE;
        return true;
    }
    if (match(RuleId::VM_SCALE2)) {
        *out = RuleId::VM_SCALE2;
        return true;
    }
    if (match(RuleId::WMv_STEM_V)) {
        *out = RuleId::WMv_STEM_V;
        return true;
    }
    if (match(RuleId::WMp_STEM_WMB)) {
        *out = RuleId::WMp_STEM_WMB;
        return true;
    }
    if (match(RuleId::WMv_EXTEND_UNPAIRED)) {
        *out = RuleId::WMv_EXTEND_UNPAIRED;
        return true;
    }
    if (match(RuleId::WMp_EXTEND_UNPAIRED)) {
        *out = RuleId::WMp_EXTEND_UNPAIRED;
        return true;
    }
    if (match(RuleId::WM_START_V)) {
        *out = RuleId::WM_START_V;
        return true;
    }
    if (match(RuleId::WM_START_WMB)) {
        *out = RuleId::WM_START_WMB;
        return true;
    }
    if (match(RuleId::WM_SPLIT_V)) {
        *out = RuleId::WM_SPLIT_V;
        return true;
    }
    if (match(RuleId::WM_SPLIT_WMB)) {
        *out = RuleId::WM_SPLIT_WMB;
        return true;
    }
    if (match(RuleId::WM_EXTEND_UNPAIRED)) {
        *out = RuleId::WM_EXTEND_UNPAIRED;
        return true;
    }
    if (match(RuleId::WIP_BASE_V)) {
        *out = RuleId::WIP_BASE_V;
        return true;
    }
    if (match(RuleId::WIP_BASE_WMB)) {
        *out = RuleId::WIP_BASE_WMB;
        return true;
    }
    if (match(RuleId::WIP_SPLIT_V)) {
        *out = RuleId::WIP_SPLIT_V;
        return true;
    }
    if (match(RuleId::WIP_SPLIT_WMB)) {
        *out = RuleId::WIP_SPLIT_WMB;
        return true;
    }
    if (match(RuleId::WIP_BASEPAIR_V)) {
        *out = RuleId::WIP_BASEPAIR_V;
        return true;
    }
    if (match(RuleId::WIP_BASEPAIR_WMB)) {
        *out = RuleId::WIP_BASEPAIR_WMB;
        return true;
    }
    if (match(RuleId::WIP_EXTEND_UNPAIRED)) {
        *out = RuleId::WIP_EXTEND_UNPAIRED;
        return true;
    }
    if (match(RuleId::VPL_SPLIT_VP)) {
        *out = RuleId::VPL_SPLIT_VP;
        return true;
    }
    if (match(RuleId::VPR_SPLIT_VP_WIP)) {
        *out = RuleId::VPR_SPLIT_VP_WIP;
        return true;
    }
    if (match(RuleId::VPR_SPLIT_VP_BASEPAIR)) {
        *out = RuleId::VPR_SPLIT_VP_BASEPAIR;
        return true;
    }
    if (match(RuleId::VP_WI_CASE1)) {
        *out = RuleId::VP_WI_CASE1;
        return true;
    }
    if (match(RuleId::VP_WI_CASE2)) {
        *out = RuleId::VP_WI_CASE2;
        return true;
    }
    if (match(RuleId::VP_WI_CASE3)) {
        *out = RuleId::VP_WI_CASE3;
        return true;
    }
    if (match(RuleId::VP_STACK)) {
        *out = RuleId::VP_STACK;
        return true;
    }
    if (match(RuleId::VP_INTERNAL_LOOP)) {
        *out = RuleId::VP_INTERNAL_LOOP;
        return true;
    }
    if (match(RuleId::VP_WIP_VP_LEFT)) {
        *out = RuleId::VP_WIP_VP_LEFT;
        return true;
    }
    if (match(RuleId::VP_VP_WIP_RIGHT)) {
        *out = RuleId::VP_VP_WIP_RIGHT;
        return true;
    }
    if (match(RuleId::VP_WIP_VPR)) {
        *out = RuleId::VP_WIP_VPR;
        return true;
    }
    if (match(RuleId::VP_VPL_WIP)) {
        *out = RuleId::VP_VPL_WIP;
        return true;
    }
    if (match(RuleId::WMBW_SPLIT_WMBP_WI)) {
        *out = RuleId::WMBW_SPLIT_WMBP_WI;
        return true;
    }
    if (match(RuleId::WMBP_SPLIT_BE_WMBP_VP)) {
        *out = RuleId::WMBP_SPLIT_BE_WMBP_VP;
        return true;
    }
    if (match(RuleId::WMBP_SPLIT_BE_WMBW_VP)) {
        *out = RuleId::WMBP_SPLIT_BE_WMBW_VP;
        return true;
    }
    if (match(RuleId::WMBP_DIRECT_VP)) {
        *out = RuleId::WMBP_DIRECT_VP;
        return true;
    }
    if (match(RuleId::WMBP_SPLIT_BE_WI_VP)) {
        *out = RuleId::WMBP_SPLIT_BE_WI_VP;
        return true;
    }
    if (match(RuleId::WMB_SPLIT_BE_WMBP_WI)) {
        *out = RuleId::WMB_SPLIT_BE_WMBP_WI;
        return true;
    }
    if (match(RuleId::WMB_DIRECT_WMBP)) {
        *out = RuleId::WMB_DIRECT_WMBP;
        return true;
    }
    if (match(RuleId::WMB_EMPTY)) {
        *out = RuleId::WMB_EMPTY;
        return true;
    }
    if (match(RuleId::BE_BASE_SAMEPAIR)) {
        *out = RuleId::BE_BASE_SAMEPAIR;
        return true;
    }
    if (match(RuleId::BE_STACK)) {
        *out = RuleId::BE_STACK;
        return true;
    }
    if (match(RuleId::BE_INTERNAL_LOOP)) {
        *out = RuleId::BE_INTERNAL_LOOP;
        return true;
    }
    if (match(RuleId::BE_WIP_WIP)) {
        *out = RuleId::BE_WIP_WIP;
        return true;
    }
    if (match(RuleId::BE_WIP_BASEPAIR)) {
        *out = RuleId::BE_WIP_BASEPAIR;
        return true;
    }
    if (match(RuleId::BE_BASEPAIR_WIP)) {
        *out = RuleId::BE_BASEPAIR_WIP;
        return true;
    }

    return false;
}

} // namespace scfg
