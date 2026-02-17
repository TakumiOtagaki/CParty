#include "part_func.hh"
#include "scfg/part_func_adapter.hh"

#include <cstdlib>
#include <cstring>
#include <iostream>

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
    const char *trace_env = std::getenv("CPARTY_PF_TRACE_INTERNAL");
    bool trace = false;
    if (trace_env && *trace_env != '\0' && std::strcmp(trace_env, "0") != 0) {
        const char *comma = std::strchr(trace_env, ',');
        if (comma) {
            const int ti = std::atoi(trace_env);
            const int tj = std::atoi(comma + 1);
            if (ti == i && tj == j) {
                trace = true;
            }
        }
    }
    pf_t v_iloop = 0;
    int trace_count = 0;
    pf_t trace_max = 0;
    cand_pos_t max_k = std::min(j - TURN - 2, i + MAXLOOP + 1);
    const pair_type ptype_closing = pair[S_[i]][S_[j]];
    for (cand_pos_t k = i + 1; k <= max_k; ++k) {
        if ((up[k - 1] >= (k - i - 1))) {
            cand_pos_t min_l = std::max(k + TURN + 1 + MAXLOOP + 2, k + j - i) - MAXLOOP - 2;
            for (cand_pos_t l = j - 1; l >= min_l; --l) {
                if (up[j - 1] >= (j - l - 1)) {
                    const pair_type ptype_inner = pair[S_[k]][S_[l]];
                    pf_t energy_kl = get_energy(k, l);
                    pf_t v_iloop_kl = energy_kl
                                      * exp_E_IntLoop(k - i - 1, j - l - 1, ptype_closing, rtype[ptype_inner], S1_[i + 1], S1_[j - 1],
                                                      S1_[k - 1], S1_[l + 1], exp_params_);
                    cand_pos_t u1 = k - i - 1;
                    cand_pos_t u2 = j - l - 1;
                    v_iloop_kl *= scale[u1 + u2 + 2];
                    v_iloop += v_iloop_kl;
                    if (trace) {
                        trace_count += 1;
                        if (v_iloop_kl > trace_max) trace_max = v_iloop_kl;
                        if (trace_count <= 10) {
                            std::cerr << "[PF_TRACE_INTERNAL_TERM] k=" << k
                                      << " l=" << l
                                      << " energy_kl=" << energy_kl
                                      << " ptype_inner=" << ptype_inner
                                      << " u1=" << u1
                                      << " u2=" << u2
                                      << " term=" << v_iloop_kl
                                      << std::endl;
                        }
                    }
                }
            }
        }
    }
    if (trace) {
        std::cerr << "[PF_TRACE_INTERNAL] i=" << i
                  << " j=" << j
                  << " ptype=" << ptype_closing
                  << " count=" << trace_count
                  << " max_term=" << trace_max
                  << " total=" << v_iloop
                  << std::endl;
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
