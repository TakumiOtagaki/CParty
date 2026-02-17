#include "part_func.hh"
#include "h_externs.hh"

#include <cmath>
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

pf_t W_final_pf::exp_Extloop(cand_pos_t i, cand_pos_t j) {
    ensure_local_pair_matrix_initialized();
    pair_type tt = pair[S_[i]][S_[j]];

    if (exp_params_->model_details.dangles == 1 || exp_params_->model_details.dangles == 2) {
        base_type si1 = i > 1 ? S_[i - 1] : -1;
        base_type sj1 = j < n ? S_[j + 1] : -1;
        return exp_E_ExtLoop(tt, si1, sj1, exp_params_);
    } else {
        return exp_E_ExtLoop(tt, -1, -1, exp_params_);
    }
}

pf_t W_final_pf::exp_MLstem(cand_pos_t i, cand_pos_t j) {
    ensure_local_pair_matrix_initialized();
    pair_type tt = pair[S_[i]][S_[j]];
    if (exp_params_->model_details.dangles == 1 || exp_params_->model_details.dangles == 2) {
        base_type si1 = i > 1 ? S_[i - 1] : -1;
        base_type sj1 = j < n ? S_[j + 1] : -1;
        return exp_E_MLstem(tt, si1, sj1, exp_params_);
    } else {
        return exp_E_MLstem(tt, -1, -1, exp_params_);
    }
}

pf_t W_final_pf::exp_Mbloop(cand_pos_t i, cand_pos_t j) {
    ensure_local_pair_matrix_initialized();
    pair_type tt = pair[S_[j]][S_[i]];
    if (exp_params_->model_details.dangles == 1 || exp_params_->model_details.dangles == 2) {
        base_type si1 = i > 1 ? S_[i + 1] : -1;
        base_type sj1 = j < n ? S_[j - 1] : -1;
        return exp_E_MLstem(tt, sj1, si1, exp_params_);
    } else {
        return exp_E_MLstem(tt, -1, -1, exp_params_);
    }
}

pf_t W_final_pf::HairpinE(cand_pos_t i, cand_pos_t j) {
    ensure_local_pair_matrix_initialized();
    const int ptype_closing = pair[S_[i]][S_[j]];
    if (ptype_closing == 0) return 0;
    pf_t e_h = static_cast<pf_t>(exp_E_Hairpin(j - i - 1, ptype_closing, S1_[i + 1], S1_[j - 1], &seq.c_str()[i - 1], exp_params_));
    e_h *= scale[j - i + 1];
    const char *pf_debug_env = std::getenv("CPARTY_PF_DEBUG");
    static bool hairpin_logged = false;
    if (!hairpin_logged && pf_debug_env && *pf_debug_env != '\0' && std::strcmp(pf_debug_env, "0") != 0 && i == 1 && j == n) {
        hairpin_logged = true;
        std::cerr << "[PF_DEBUG] HairpinE"
                  << " ptype=" << ptype_closing
                  << " loop_len=" << (j - i - 1)
                  << " S1_i1=" << static_cast<int>(S1_[i + 1])
                  << " S1_j1=" << static_cast<int>(S1_[j - 1])
                  << " seq_ij=" << seq.substr(i - 1, j - i + 1)
                  << " scale=" << scale[j - i + 1]
                  << " pf_scale=" << exp_params_->pf_scale
                  << " kT=" << exp_params_->kT
                  << " result=" << e_h
                  << std::endl;
    }
    return e_h;
}

pf_t W_final_pf::compute_int(cand_pos_t i, cand_pos_t j, cand_pos_t k, cand_pos_t l) {
    ensure_local_pair_matrix_initialized();
    const pair_type ptype_closing = pair[S_[i]][S_[j]];
    return exp_E_IntLoop(k - i - 1, j - l - 1, ptype_closing, rtype[pair[S_[k]][S_[l]]], S1_[i + 1], S1_[j - 1], S1_[k - 1], S1_[l + 1], exp_params_);
}

pf_t W_final_pf::get_e_stP(cand_pos_t i, cand_pos_t j) {
    if (i + 1 == j - 1) { // TODO: do I need something like that or stack is taking care of this?
        return 0;
    }
    pf_t e_st = compute_int(i, j, i + 1, j - 1);

    return pow(e_st, e_stP_penalty);
}

pf_t W_final_pf::get_e_intP(cand_pos_t i, cand_pos_t ip, cand_pos_t jp, cand_pos_t j) {
    if (ip == i + 1 && jp == j - 1) return 0;
    pf_t e_int = compute_int(i, j, ip, jp);

    return pow(e_int, e_intP_penalty);
}
