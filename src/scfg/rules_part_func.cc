#include "scfg/rules_part_func.hh"

#include <cmath>

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

} // namespace scfg
