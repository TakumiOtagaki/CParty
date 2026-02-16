#include "scfg/rules_runtime.hh"

#include "scfg/legacy_adapter.hh"

namespace scfg {

void compute_V_restricted_rules(PartFuncVContext &ctx, cand_pos_t i, cand_pos_t j, sparse_tree &tree, const RulesConfig &config) {
    const cand_pos_t ij = ctx.index_of(i, j);

    const bool unpaired = (tree.tree[i].pair < -1 && tree.tree[j].pair < -1);
    const bool paired = (tree.tree[i].pair == j && tree.tree[j].pair == i);

    pf_t contributions = 0;

    if (paired || unpaired) {
        if (is_rule_enabled(config, RuleId::V_HAIRPIN)) {
            const bool canH = !(tree.up[j - 1] < (j - i - 1));
            if (canH) contributions += ctx.hairpin_energy(i, j);
        }

        if (is_rule_enabled(config, RuleId::V_INTERNAL)) {
            contributions += ctx.internal_energy(i, j, tree.up);
        }

        if (is_rule_enabled(config, RuleId::V_VM)) {
            contributions += ctx.vm_energy(i, j, tree.up);
        }
    }

    ctx.set_V(ij, contributions);
}

void compute_W_restricted_rules(PartFuncWContext &ctx, sparse_tree &tree, const RulesConfig &config) {
    const cand_pos_t n = ctx.n();
    const cand_pos_t turn = ctx.turn();

    for (cand_pos_t j = turn + 1; j <= n; j++) {
        pf_t contributions = 0;

        if (is_rule_enabled(config, RuleId::W_EXTEND_UNPAIRED)) {
            if (tree.tree[j].pair < 0) contributions += ctx.get_W(j - 1) * ctx.scale1();
        }

        if (tree.weakly_closed(1, j)) {
            for (cand_pos_t k = 1; k <= j - turn - 1; ++k) {
                if (!tree.weakly_closed(1, k - 1)) continue;
                pf_t acc = (k > 1) ? ctx.get_W(k - 1) : 1;
                if (is_rule_enabled(config, RuleId::W_SPLIT_V)) {
                    contributions += acc * ctx.get_energy(k, j) * ctx.exp_Extloop(k, j);
                }
                if (is_rule_enabled(config, RuleId::W_SPLIT_WMB)) {
                    if (k == 1 || tree.weakly_closed(k, j)) {
                        contributions += acc * ctx.get_energy_WMB(k, j) * ctx.expPS_penalty();
                    }
                }
            }
        }
        ctx.set_W(j, contributions);
    }
}

void compute_WI_restricted_rules(PartFuncWIContext &ctx, cand_pos_t i, cand_pos_t j, sparse_tree &tree, const RulesConfig &config) {
    const cand_pos_t ij = ctx.index_of(i, j);
    pf_t contributions = 0;
    if (i == j) {
        if (is_rule_enabled(config, RuleId::WI_BASE_SINGLE)) {
            ctx.set_WI(ij, ctx.expPUP_pen1());
        } else {
            ctx.set_WI(ij, 0);
        }
        return;
    }
    const cand_pos_t turn = ctx.turn();
    for (cand_pos_t k = i; k <= j - turn - 1; ++k) {
        if (is_rule_enabled(config, RuleId::WI_SPLIT_V)) {
            contributions += (ctx.get_WI(i, k - 1) * ctx.get_energy(k, j) * ctx.expPPS_penalty());
        }
        if (is_rule_enabled(config, RuleId::WI_SPLIT_WMB)) {
            contributions += (ctx.get_WI(i, k - 1) * ctx.get_energy_WMB(k, j) * ctx.expPSP_penalty() * ctx.expPPS_penalty());
        }
    }
    if (tree.tree[j].pair < 0 && is_rule_enabled(config, RuleId::WI_EXTEND_UNPAIRED)) {
        contributions += (ctx.get_WI(i, j - 1) * ctx.expPUP_pen1());
    }

    ctx.set_WI(ij, contributions);
}

void compute_WMv_WMp_restricted_rules(PartFuncWMvWMpContext &ctx, cand_pos_t i, cand_pos_t j, std::vector<Node> &tree, const RulesConfig &config) {
    if (j - i - 1 < ctx.turn()) return;
    const cand_pos_t ij = ctx.index_of(i, j);

    pf_t WMv_contributions = 0;
    pf_t WMp_contributions = 0;

    if (is_rule_enabled(config, RuleId::WMv_STEM_V)) {
        WMv_contributions += (ctx.get_energy(i, j) * ctx.exp_MLstem(i, j));
    }
    if (is_rule_enabled(config, RuleId::WMp_STEM_WMB)) {
        WMp_contributions += (ctx.get_energy_WMB(i, j) * ctx.expPSM_penalty() * ctx.expb_penalty());
    }
    if (tree[j].pair < 0) {
        if (is_rule_enabled(config, RuleId::WMv_EXTEND_UNPAIRED)) {
            WMv_contributions += (ctx.get_energy_WMv(i, j - 1) * ctx.expMLbase1());
        }
        if (is_rule_enabled(config, RuleId::WMp_EXTEND_UNPAIRED)) {
            WMp_contributions += (ctx.get_energy_WMp(i, j - 1) * ctx.expMLbase1());
        }
    }

    ctx.set_WMv_WMp(ij, WMv_contributions, WMp_contributions);
}

void compute_WM_restricted_rules(PartFuncWMContext &ctx, cand_pos_t i, cand_pos_t j, sparse_tree &tree, const RulesConfig &config) {
    if (j - i + 1 < 4) return;
    pf_t contributions = 0;
    const cand_pos_t ij = ctx.index_of(i, j);
    const cand_pos_t turn = ctx.turn();

    for (cand_pos_t k = i; k < j - turn; ++k) {
        const pf_t qbt1 = ctx.get_energy(k, j) * ctx.exp_MLstem(k, j);
        const pf_t qbt2 = ctx.get_energy_WMB(k, j) * ctx.expPSM_penalty() * ctx.expb_penalty();
        const bool can_pair = scfg::can_pair_left_span(tree, i, k);
        if (can_pair && is_rule_enabled(config, RuleId::WM_START_V)) {
            contributions += (ctx.expMLbase(k - i) * qbt1);
        }
        if (can_pair && is_rule_enabled(config, RuleId::WM_START_WMB)) {
            contributions += (ctx.expMLbase(k - i) * qbt2);
        }
        if (is_rule_enabled(config, RuleId::WM_SPLIT_V)) {
            contributions += (ctx.get_energy_WM(i, k - 1) * qbt1);
        }
        if (is_rule_enabled(config, RuleId::WM_SPLIT_WMB)) {
            contributions += (ctx.get_energy_WM(i, k - 1) * qbt2);
        }
    }
    if (tree.tree[j].pair < 0 && is_rule_enabled(config, RuleId::WM_EXTEND_UNPAIRED)) {
        contributions += ctx.get_energy_WM(i, j - 1) * ctx.expMLbase(1);
    }
    ctx.set_WM(ij, contributions);
}

void compute_WIP_restricted_rules(PartFuncWIPContext &ctx, cand_pos_t i, cand_pos_t j, sparse_tree &tree, const RulesConfig &config) {
    const cand_pos_t ij = ctx.index_of(i, j);
    pf_t contributions = 0;
    if (is_rule_enabled(config, RuleId::WIP_BASE_V)) {
        contributions += ctx.get_energy(i, j) * ctx.expbp_penalty();
    }
    if (is_rule_enabled(config, RuleId::WIP_BASE_WMB)) {
        contributions += ctx.get_energy_WMB(i, j) * ctx.expbp_penalty() * ctx.expPSM_penalty();
    }
    const cand_pos_t turn = ctx.turn();
    for (cand_pos_t k = i + 1; k < j - turn - 1; ++k) {
        bool can_pair = scfg::can_pair_left_span(tree, i, k);

        if (is_rule_enabled(config, RuleId::WIP_SPLIT_V)) {
            contributions += (ctx.get_energy_WIP(i, k - 1) * ctx.get_energy(k, j) * ctx.expbp_penalty());
        }
        if (is_rule_enabled(config, RuleId::WIP_SPLIT_WMB)) {
            contributions += (ctx.get_energy_WIP(i, k - 1) * ctx.get_energy_WMB(k, j) * ctx.expbp_penalty() * ctx.expPSM_penalty());
        }
        if (can_pair && is_rule_enabled(config, RuleId::WIP_BASEPAIR_V)) {
            contributions += (ctx.expcp_pen(k - i) * ctx.get_energy(k, j) * ctx.expbp_penalty());
        }
        if (can_pair && is_rule_enabled(config, RuleId::WIP_BASEPAIR_WMB)) {
            contributions += (ctx.expcp_pen(k - i) * ctx.get_energy_WMB(k, j) * ctx.expbp_penalty() * ctx.expPSM_penalty());
        }
    }
    if (tree.tree[j].pair < 0 && is_rule_enabled(config, RuleId::WIP_EXTEND_UNPAIRED)) {
        contributions += (ctx.get_energy_WIP(i, j - 1) * ctx.expcp_pen(1));
    }
    ctx.set_WIP(ij, contributions);
}

void compute_VPL_restricted_rules(PartFuncVPLContext &ctx, cand_pos_t i, cand_pos_t j, sparse_tree &tree, const RulesConfig &config) {
    const cand_pos_t ij = ctx.index_of(i, j);
    pf_t contributions = 0;

    if (!is_rule_enabled(config, RuleId::VPL_SPLIT_VP)) {
        ctx.set_VPL(ij, 0);
        return;
    }

    cand_pos_t min_Bp_j = std::min((cand_pos_tu)tree.b(i, j), (cand_pos_tu)tree.Bp(i, j));
    for (cand_pos_t k = i + 1; k < min_Bp_j; ++k) {
        bool can_pair = scfg::can_pair_left_span(tree, i, k);
        if (can_pair) contributions += (ctx.expcp_pen(k - i) * ctx.get_energy_VP(k, j));
    }
    ctx.set_VPL(ij, contributions);
}

void compute_VPR_restricted_rules(PartFuncVPRContext &ctx, cand_pos_t i, cand_pos_t j, sparse_tree &tree, const RulesConfig &config) {
    const cand_pos_t ij = ctx.index_of(i, j);
    pf_t contributions = 0;

    cand_pos_t max_i_bp = std::max(tree.B(i, j), tree.bp(i, j));
    for (cand_pos_t k = max_i_bp + 1; k < j; ++k) {
        bool can_pair = scfg::can_pair_right_span(tree, k, j);
        if (is_rule_enabled(config, RuleId::VPR_SPLIT_VP_WIP)) {
            contributions += (ctx.get_energy_VP(i, k) * ctx.get_energy_WIP(k + 1, j));
        }
        if (can_pair && is_rule_enabled(config, RuleId::VPR_SPLIT_VP_BASEPAIR)) {
            contributions += (ctx.get_energy_VP(i, k) * ctx.expcp_pen(k - i));
        }
    }
    ctx.set_VPR(ij, contributions);
}

} // namespace scfg
