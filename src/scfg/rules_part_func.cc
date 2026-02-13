#include "scfg/rules_part_func.hh"

#include "scfg/legacy_adapter.hh"
#include "scfg/constraint_oracle.hh"

#include <cmath>
#include <ViennaRNA/params/constants.h>

namespace scfg {

PartFuncRuleHelpers::PartFuncRuleHelpers(sparse_tree &tree, const PartFuncModeConfig &config) : tree_(tree), config_(config) {}

cand_pos_t PartFuncRuleHelpers::border_b(cand_pos_t i, cand_pos_t j) { return tree_.b(i, j); }

cand_pos_t PartFuncRuleHelpers::border_bp(cand_pos_t i, cand_pos_t j) { return tree_.bp(i, j); }

cand_pos_t PartFuncRuleHelpers::border_B(cand_pos_t i, cand_pos_t j) { return tree_.B(i, j); }

cand_pos_t PartFuncRuleHelpers::border_Bp(cand_pos_t i, cand_pos_t j) { return tree_.Bp(i, j); }

cand_pos_t PartFuncRuleHelpers::pair_at(cand_pos_t pos) const { return tree_.tree[pos].pair; }

cand_pos_t PartFuncRuleHelpers::parent_index(cand_pos_t pos) const { return tree_.tree[pos].parent->index; }

bool PartFuncRuleHelpers::allow_exterior_split(cand_pos_t l, cand_pos_t j, cand_pos_t b_ij, int exterior_case) const {
    (void)j;
    return (b_ij > 0 && l < b_ij) || (b_ij < 0 && exterior_case == 0);
}

bool PartFuncRuleHelpers::has_valid_band_borders(cand_pos_t i, cand_pos_t l, cand_pos_t j) {
    const cand_pos_t bp_il = border_bp(i, l);
    const cand_pos_t Bp_lj = border_Bp(l, j);
    return bp_il >= 0 && l > bp_il && Bp_lj > 0 && l < Bp_lj;
}

bool PartFuncRuleHelpers::parent_within_interval_and_turn(cand_pos_t i, cand_pos_t l, cand_pos_t j) const {
    return i <= parent_index(l) && parent_index(l) < j && l + config_.turn <= j;
}

bool PartFuncRuleHelpers::has_valid_inner_arc_split(cand_pos_t i, cand_pos_t l, cand_pos_t j, cand_pos_t n_bound) {
    const cand_pos_t bp_il = border_bp(i, l);
    return bp_il >= 0 && bp_il < n_bound && l + config_.turn <= j;
}

pf_t PartFuncRuleHelpers::apply_double_pb_penalty(pf_t value) const { return value * std::pow(config_.exp_pb_penalty, 2); }

void PartFuncRuleHelpers::on_traceback_hook(cand_pos_t i, cand_pos_t j) const {
    (void)i;
    (void)j;
}

void PartFuncRuleHelpers::on_fixed_parse_hook(cand_pos_t i, cand_pos_t j) const {
    (void)i;
    (void)j;
}

void compute_V_restricted(PartFuncVContext &ctx, cand_pos_t i, cand_pos_t j, sparse_tree &tree) {
    const cand_pos_t ij = ctx.index_of(i, j);

    const bool unpaired = (tree.tree[i].pair < -1 && tree.tree[j].pair < -1);
    const bool paired = (tree.tree[i].pair == j && tree.tree[j].pair == i);

    pf_t contributions = 0;

    if (paired || unpaired) {
        bool canH = !(tree.up[j - 1] < (j - i - 1));
        if (canH) contributions += ctx.hairpin_energy(i, j);

        contributions += ctx.internal_energy(i, j, tree.up);

        contributions += ctx.vm_energy(i, j, tree.up);
    }

    ctx.set_V(ij, contributions);
}

void compute_WI_restricted(PartFuncWIContext &ctx, cand_pos_t i, cand_pos_t j, sparse_tree &tree) {
    const cand_pos_t ij = ctx.index_of(i, j);
    pf_t contributions = 0;
    if (i == j) {
        ctx.set_WI(ij, ctx.expPUP_pen1());
        return;
    }
    const cand_pos_t turn = ctx.turn();
    for (cand_pos_t k = i; k <= j - turn - 1; ++k) {
        contributions += (ctx.get_WI(i, k - 1) * ctx.get_energy(k, j) * ctx.expPPS_penalty());
        contributions += (ctx.get_WI(i, k - 1) * ctx.get_energy_WMB(k, j) * ctx.expPSP_penalty() * ctx.expPPS_penalty());
    }
    if (tree.tree[j].pair < 0) {
        contributions += (ctx.get_WI(i, j - 1) * ctx.expPUP_pen1());
    }

    ctx.set_WI(ij, contributions);
}

void compute_W_restricted(PartFuncWContext &ctx, sparse_tree &tree) {
    const cand_pos_t n = ctx.n();
    const cand_pos_t turn = ctx.turn();

    for (cand_pos_t j = turn + 1; j <= n; j++) {
        pf_t contributions = 0;
        if (tree.tree[j].pair < 0) contributions += ctx.get_W(j - 1) * ctx.scale1();
        if (tree.weakly_closed(1, j)) {
            for (cand_pos_t k = 1; k <= j - turn - 1; ++k) {
                if (tree.weakly_closed(1, k - 1)) {
                    pf_t acc = (k > 1) ? ctx.get_W(k - 1) : 1;
                    contributions += acc * ctx.get_energy(k, j) * ctx.exp_Extloop(k, j);
                    if (k == 1 || tree.weakly_closed(k, j)) {
                        contributions += acc * ctx.get_energy_WMB(k, j) * ctx.expPS_penalty();
                    }
                }
            }
        }
        ctx.set_W(j, contributions);
    }
}

pf_t compute_VM_restricted(PartFuncVMContext &ctx, cand_pos_t i, cand_pos_t j, std::vector<int> &up) {
    pf_t contributions = 0;
    const cand_pos_t ij = ctx.index_of(i, j);
    const cand_pos_t turn = ctx.turn();
    for (cand_pos_t k = i + 1; k <= j - turn - 1; ++k) {
        contributions += (ctx.get_energy_WM(i + 1, k - 1) * ctx.get_energy_WMv(k, j - 1) *
                          ctx.exp_Mbloop(i, j) * ctx.expMLclosing());
        contributions += (ctx.get_energy_WM(i + 1, k - 1) * ctx.get_energy_WMp(k, j - 1) *
                          ctx.exp_Mbloop(i, j) * ctx.expMLclosing());
        if (up[k - 1] >= (k - (i + 1))) {
            contributions += (ctx.expMLbase(k - i - 1) * ctx.get_energy_WMp(k, j - 1) *
                              ctx.exp_Mbloop(i, j) * ctx.expMLclosing());
        }
    }

    contributions *= ctx.scale2();
    ctx.set_VM(ij, contributions);
    return contributions;
}

void compute_WMv_WMp_restricted(PartFuncWMvWMpContext &ctx, cand_pos_t i, cand_pos_t j, std::vector<Node> &tree) {
    if (j - i - 1 < 3) return;
    const cand_pos_t ij = ctx.index_of(i, j);

    pf_t WMv_contributions = 0;
    pf_t WMp_contributions = 0;

    WMv_contributions += (ctx.get_energy(i, j) * ctx.exp_MLstem(i, j));
    WMp_contributions += (ctx.get_energy_WMB(i, j) * ctx.expPSM_penalty() * ctx.expb_penalty());
    if (tree[j].pair < 0) {
        WMv_contributions += (ctx.get_energy_WMv(i, j - 1) * ctx.expMLbase1());
        WMp_contributions += (ctx.get_energy_WMp(i, j - 1) * ctx.expMLbase1());
    }

    ctx.set_WMv_WMp(ij, WMv_contributions, WMp_contributions);
}

void compute_WM_restricted(PartFuncWMContext &ctx, cand_pos_t i, cand_pos_t j, sparse_tree &tree) {
    if (j - i + 1 < 4) return;
    pf_t contributions = 0;
    const cand_pos_t ij = ctx.index_of(i, j);
    const cand_pos_t turn = ctx.turn();

    for (cand_pos_t k = i; k < j - turn; ++k) {
        const pf_t qbt1 = ctx.get_energy(k, j) * ctx.exp_MLstem(k, j);
        const pf_t qbt2 = ctx.get_energy_WMB(k, j) * ctx.expPSM_penalty() * ctx.expb_penalty();
        const bool can_pair = scfg::can_pair_left_span(tree, i, k);
        if (can_pair) contributions += (ctx.expMLbase(k - i) * qbt1);
        if (can_pair) contributions += (ctx.expMLbase(k - i) * qbt2);
        contributions += (ctx.get_energy_WM(i, k - 1) * qbt1);
        contributions += (ctx.get_energy_WM(i, k - 1) * qbt2);
    }
    if (tree.tree[j].pair < 0) contributions += ctx.get_energy_WM(i, j - 1) * ctx.expMLbase(1);
    ctx.set_WM(ij, contributions);
}

void compute_WIP_restricted(PartFuncWIPContext &ctx, cand_pos_t i, cand_pos_t j, sparse_tree &tree) {
    const cand_pos_t ij = ctx.index_of(i, j);
    pf_t contributions = 0;
    contributions += ctx.get_energy(i, j) * ctx.expbp_penalty();
    contributions += ctx.get_energy_WMB(i, j) * ctx.expbp_penalty() * ctx.expPSM_penalty();
    const cand_pos_t turn = ctx.turn();
    for (cand_pos_t k = i + 1; k < j - turn - 1; ++k) {
        bool can_pair = scfg::can_pair_left_span(tree, i, k);

        contributions += (ctx.get_energy_WIP(i, k - 1) * ctx.get_energy(k, j) * ctx.expbp_penalty());
        contributions += (ctx.get_energy_WIP(i, k - 1) * ctx.get_energy_WMB(k, j) * ctx.expbp_penalty() * ctx.expPSM_penalty());
        if (can_pair) contributions += (ctx.expcp_pen(k - i) * ctx.get_energy(k, j) * ctx.expbp_penalty());
        if (can_pair) contributions += (ctx.expcp_pen(k - i) * ctx.get_energy_WMB(k, j) * ctx.expbp_penalty() * ctx.expPSM_penalty());
    }
    if (tree.tree[j].pair < 0) contributions += (ctx.get_energy_WIP(i, j - 1) * ctx.expcp_pen(1));
    ctx.set_WIP(ij, contributions);
}

void compute_VPL_restricted(PartFuncVPLContext &ctx, cand_pos_t i, cand_pos_t j, sparse_tree &tree) {
    const cand_pos_t ij = ctx.index_of(i, j);
    pf_t contributions = 0;

    cand_pos_t min_Bp_j = std::min((cand_pos_tu)tree.b(i, j), (cand_pos_tu)tree.Bp(i, j));
    for (cand_pos_t k = i + 1; k < min_Bp_j; ++k) {
        bool can_pair = scfg::can_pair_left_span(tree, i, k);
        if (can_pair) contributions += (ctx.expcp_pen(k - i) * ctx.get_energy_VP(k, j));
    }
    ctx.set_VPL(ij, contributions);
}

void compute_VPR_restricted(PartFuncVPRContext &ctx, cand_pos_t i, cand_pos_t j, sparse_tree &tree) {
    const cand_pos_t ij = ctx.index_of(i, j);
    pf_t contributions = 0;
    cand_pos_t max_i_bp = std::max(tree.B(i, j), tree.bp(i, j));
    for (cand_pos_t k = max_i_bp + 1; k < j; ++k) {
        bool can_pair = scfg::can_pair_right_span(tree, k, j);
        contributions += (ctx.get_energy_VP(i, k) * ctx.get_energy_WIP(k + 1, j));
        if (can_pair) contributions += (ctx.get_energy_VP(i, k) * ctx.expcp_pen(k - i));
    }
    ctx.set_VPR(ij, contributions);
}

void compute_VP_restricted(PartFuncVPContext &ctx, cand_pos_t i, cand_pos_t j, sparse_tree &tree) {
    const cand_pos_t ij = ctx.index_of(i, j);

    pf_t contributions = 0;

    cand_pos_t Bp_ij = tree.Bp(i, j);
    cand_pos_t B_ij = tree.B(i, j);
    cand_pos_t b_ij = tree.b(i, j);
    cand_pos_t bp_ij = tree.bp(i, j);

    if ((tree.tree[i].parent->index) > 0 && (tree.tree[j].parent->index) < (tree.tree[i].parent->index) &&
        Bp_ij >= 0 && B_ij >= 0 && bp_ij < 0) {
        pf_t m1 = (ctx.get_energy_WI(i + 1, Bp_ij - 1) * ctx.get_energy_WI(B_ij + 1, j - 1));
        m1 *= ctx.scale(2);
        contributions += m1;
    }

    if ((tree.tree[i].parent->index) < (tree.tree[j].parent->index) && (tree.tree[j].parent->index) > 0 &&
        b_ij >= 0 && bp_ij >= 0 && Bp_ij < 0) {
        pf_t m2 = (ctx.get_energy_WI(i + 1, b_ij - 1) * ctx.get_energy_WI(bp_ij + 1, j - 1));
        m2 *= ctx.scale(2);
        contributions += m2;
    }

    if ((tree.tree[i].parent->index) > 0 && (tree.tree[j].parent->index) > 0 && Bp_ij >= 0 && B_ij >= 0 &&
        b_ij >= 0 && bp_ij >= 0) {
        pf_t m3 = (ctx.get_energy_WI(i + 1, Bp_ij - 1) * ctx.get_energy_WI(B_ij + 1, b_ij - 1) *
                   ctx.get_energy_WI(bp_ij + 1, j - 1));
        m3 *= ctx.scale(2);
        contributions += m3;
    }

    pair_type ptype_closingip1jm1 = ctx.pair_type_of(i + 1, j - 1);
    if ((tree.tree[i + 1].pair) < -1 && (tree.tree[j - 1].pair) < -1 && ptype_closingip1jm1 > 0) {
        pf_t vp_stp = (ctx.get_e_stP(i, j) * ctx.get_energy_VP(i + 1, j - 1));
        vp_stp *= ctx.scale(2);
        contributions += vp_stp;
    }

    cand_pos_t min_borders = std::min((cand_pos_tu)Bp_ij, (cand_pos_tu)b_ij);
    cand_pos_t edge_i = std::min(i + MAXLOOP + 1, j - TURN - 1);
    min_borders = std::min(min_borders, edge_i);
    for (cand_pos_t k = i + 1; k < min_borders; ++k) {
        if (scfg::is_unpaired_position(tree, k) && scfg::is_empty_region(tree, i, k)) {
            cand_pos_t max_borders = std::max(bp_ij, B_ij) + 1;
            cand_pos_t edge_j = k + j - i - MAXLOOP - 2;
            max_borders = std::max(max_borders, edge_j);
            for (cand_pos_t l = j - 1; l > max_borders; --l) {
                pair_type ptype_closingkj = ctx.pair_type_of(k, l);
                if (k == i + 1 && l == j - 1) continue;
                if (scfg::is_unpaired_position(tree, l) && scfg::is_pair_type_allowed(ptype_closingkj) &&
                    scfg::is_empty_region(tree, l, j)) {
                    pf_t vp_iloop_kl = (ctx.get_e_intP(i, k, l, j) * ctx.get_energy_VP(k, l));
                    cand_pos_t u1 = k - i - 1;
                    cand_pos_t u2 = j - l - 1;
                    vp_iloop_kl *= ctx.scale(u1 + u2 + 2);
                    contributions += vp_iloop_kl;
                }
            }
        }
    }

    cand_pos_t min_Bp_j = std::min((cand_pos_tu)tree.b(i, j), (cand_pos_tu)tree.Bp(i, j));
    cand_pos_t max_i_bp = std::max(tree.B(i, j), tree.bp(i, j));

    for (cand_pos_t k = i + 1; k < min_Bp_j; ++k) {
        pf_t m6 = (ctx.get_energy_WIP(i + 1, k - 1) * ctx.get_energy_VP(k, j - 1) *
                   ctx.expap_penalty() * ctx.expbp_penalty_sq());
        m6 *= ctx.scale(2);
        contributions += m6;
    }

    for (cand_pos_t k = max_i_bp + 1; k < j; ++k) {
        pf_t m7 = (ctx.get_energy_VP(i + 1, k) * ctx.get_energy_WIP(k + 1, j - 1) *
                   ctx.expap_penalty() * ctx.expbp_penalty_sq());
        m7 *= ctx.scale(2);
        contributions += m7;
    }

    for (cand_pos_t k = i + 1; k < min_Bp_j; ++k) {
        pf_t m8 = (ctx.get_energy_WIP(i + 1, k - 1) * ctx.get_energy_VPR(k, j - 1) *
                   ctx.expap_penalty() * ctx.expbp_penalty_sq());
        m8 *= ctx.scale(2);
        contributions += m8;
    }

    for (cand_pos_t k = max_i_bp + 1; k < j; ++k) {
        pf_t m9 = (ctx.get_energy_VPL(i + 1, k) * ctx.get_energy_WIP(k + 1, j - 1) *
                   ctx.expap_penalty() * ctx.expbp_penalty_sq());
        m9 *= ctx.scale(2);
        contributions += m9;
    }

    ctx.set_VP(ij, contributions);
}

} // namespace scfg
