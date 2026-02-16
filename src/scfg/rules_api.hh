#ifndef SCFG_RULES_API_HH_
#define SCFG_RULES_API_HH_

#include "base_types.hh"

namespace scfg {

enum class NonTerminal : unsigned char {
    V,
    WI,
    W,
    VM,
    WMv,
    WMp,
    WM,
    WIP,
    VPL,
    VPR,
    VP,
    WMBW,
    WMBP,
    WMB,
    BE,
};

enum class RuleId : unsigned short {
    // V
    V_HAIRPIN,
    V_INTERNAL,
    V_VM,

    // WI
    WI_BASE_SINGLE,
    WI_SPLIT_V,
    WI_SPLIT_WMB,
    WI_EXTEND_UNPAIRED,

    // W
    W_EXTEND_UNPAIRED,
    W_SPLIT_V,
    W_SPLIT_WMB,

    // VM
    VM_SPLIT_WM_WMv,
    VM_SPLIT_WM_WMp,
    VM_SPLIT_WMp_BASE,
    VM_SCALE2,

    // WMv/WMp
    WMv_STEM_V,
    WMp_STEM_WMB,
    WMv_EXTEND_UNPAIRED,
    WMp_EXTEND_UNPAIRED,

    // WM
    WM_START_V,
    WM_START_WMB,
    WM_SPLIT_V,
    WM_SPLIT_WMB,
    WM_EXTEND_UNPAIRED,

    // WIP
    WIP_BASE_V,
    WIP_BASE_WMB,
    WIP_SPLIT_V,
    WIP_SPLIT_WMB,
    WIP_BASEPAIR_V,
    WIP_BASEPAIR_WMB,
    WIP_EXTEND_UNPAIRED,

    // VPL
    VPL_SPLIT_VP,

    // VPR
    VPR_SPLIT_VP_WIP,
    VPR_SPLIT_VP_BASEPAIR,

    // VP
    VP_WI_CASE1,
    VP_WI_CASE2,
    VP_WI_CASE3,
    VP_STACK,
    VP_INTERNAL_LOOP,
    VP_WIP_VP_LEFT,
    VP_VP_WIP_RIGHT,
    VP_WIP_VPR,
    VP_VPL_WIP,

    // WMBW
    WMBW_SPLIT_WMBP_WI,

    // WMBP
    WMBP_SPLIT_BE_WMBP_VP,
    WMBP_SPLIT_BE_WMBW_VP,
    WMBP_DIRECT_VP,
    WMBP_SPLIT_BE_WI_VP,

    // WMB
    WMB_SPLIT_BE_WMBP_WI,
    WMB_DIRECT_WMBP,
    WMB_EMPTY,

    // BE
    BE_BASE_SAMEPAIR,
    BE_STACK,
    BE_INTERNAL_LOOP,
    BE_WIP_WIP,
    BE_WIP_BASEPAIR,
    BE_BASEPAIR_WIP,
};

enum class SplitKind : unsigned char {
    None,
    SplitK,
    SplitKPredicate,
    SplitKDataBounds,
    SplitKSplitL,
    SplitKSplitLMixed,
    SplitKBandParent,
};

struct SplitSpec {
    SplitKind kind = SplitKind::None;
};

} // namespace scfg

#endif
