#include "part_func.hh"
#include "scfg/part_func_adapter.hh"

namespace {
inline void ensure_local_pair_matrix_initialized() {
    static bool initialized = false;
    if (!initialized) {
        make_pair_matrix();
        initialized = true;
    }
}
} // namespace

pf_t W_final_pf::compute_internal_restricted(cand_pos_t i, cand_pos_t j, std::vector<int> &up) {
    ensure_local_pair_matrix_initialized();
    pf_t v_iloop = 0;
    cand_pos_t max_k = std::min(j - TURN - 2, i + MAXLOOP + 1);
    const pair_type ptype_closing = pair[S_[i]][S_[j]];
    for (cand_pos_t k = i + 1; k <= max_k; ++k) {
        if ((up[k - 1] >= (k - i - 1))) {
            cand_pos_t min_l = std::max(k + TURN + 1 + MAXLOOP + 2, k + j - i) - MAXLOOP - 2;
            for (cand_pos_t l = j - 1; l >= min_l; --l) {
                if (up[j - 1] >= (j - l - 1)) {
                    pf_t v_iloop_kl = get_energy(k, l)
                                      * exp_E_IntLoop(k - i - 1, j - l - 1, ptype_closing, rtype[pair[S_[k]][S_[l]]], S1_[i + 1], S1_[j - 1],
                                                      S1_[k - 1], S1_[l + 1], exp_params_);
                    cand_pos_t u1 = k - i - 1;
                    cand_pos_t u2 = j - l - 1;
                    v_iloop_kl *= scale[u1 + u2 + 2];
                    v_iloop += v_iloop_kl;
                }
            }
        }
    }

    return v_iloop;
}

void W_final_pf::compute_WMv_WMp(cand_pos_t i, cand_pos_t j, std::vector<Node> &tree) {
    scfg::compute_WMv_WMp_restricted(*this, i, j, tree);
}

void W_final_pf::compute_energy_WM_restricted(cand_pos_t i, cand_pos_t j, sparse_tree &tree) {
    scfg::compute_WM_restricted(*this, i, j, tree);
}

pf_t W_final_pf::compute_energy_VM_restricted(cand_pos_t i, cand_pos_t j, std::vector<int> &up) {
    return scfg::compute_VM_restricted(*this, i, j, up);
}

void W_final_pf::compute_energy_restricted(cand_pos_t i, cand_pos_t j, sparse_tree &tree) {
    scfg::compute_V_restricted(*this, i, j, tree);
}
