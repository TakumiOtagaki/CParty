#include "scfg/rules_runtime.hh"

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

} // namespace scfg
