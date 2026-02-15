#include "part_func.hh"
#include "dot_plot.hh"
#include "h_externs.hh"
#include "scfg/constraint_oracle.hh"
#include "scfg/part_func_adapter.hh"
#include "scfg/rules_part_func.hh"
#include "pf_globals.hh"
#include "scfg/legacy_adapter.hh"

#include <algorithm>
#include <iostream>
#include <math.h>
#include <stdio.h>
#include <string>

/*
 * If the global use_mfelike_energies flag is set, truncate doubles to int
 * values and cast back to double. This makes the energy parameters of the
 * partition (folding get_scaled_exp_params()) compatible with the mfe folding
 * parameters (get_scaled_exp_params()), e.g. for explicit partition function
 * computations.
 */
#define TRUNC_MAYBE(X) ((!pf_smooth) ? (double)((int)(X)) : (X))
/* Rescale Free energy contribution according to deviation of temperature from measurement conditions */
#define RESCALE_dG(dG, dH, dT) ((dH) - ((dH) - (dG)) * dT)

/*
 * Rescale Free energy contribution according to deviation of temperature from measurement conditions
 * and convert it to Boltzmann Factor for specific kT
 */
#define RESCALE_BF(dG, dH, dT, kT) (exp(-TRUNC_MAYBE((double)RESCALE_dG((dG), (dH), (dT))) * 10. / kT))

W_final_pf::W_final_pf(std::string &seq, std::string &MFE_structure, bool pk_free,bool pk_only,bool fatgraph, int dangle, double energy, int num_samples, bool PSplot)
    : exp_params_(scale_pf_parameters()) {
    this->seq = seq;
    this->MFE_structure = MFE_structure;
    this->n = seq.length();
    this->pk_free = pk_free;
    this->pk_only = pk_only;
    this->fatgraph = fatgraph;
    this->PSplot = PSplot;
    this->num_samples = num_samples;

    make_pair_matrix();
    exp_params_->model_details.dangles = dangle;
    S_ = encode_sequence(seq.c_str(), 0);
    S1_ = encode_sequence(seq.c_str(), 1);

    index.resize(n + 1);
    scale.resize(n + 1);
    expMLbase.resize(n + 1);
    expcp_pen.resize(n + 1);
    expPUP_pen.resize(n + 1);
    cand_pos_t total_length = ((n + 1) * (n + 2)) / 2;
    index[1] = 0;
    for (cand_pos_t i = 2; i <= n; i++)
        index[i] = index[i - 1] + (n + 1) - i + 1;
    // Allocate space
    V.resize(total_length, 0);
    VM.resize(total_length, 0);
    WM.resize(total_length, 0);
    WMv.resize(total_length, 0);
    WMp.resize(total_length, 0);

    // PK
    WIP.resize(total_length, 0);
    VP.resize(total_length, 0);
    VPL.resize(total_length, 0);
    VPR.resize(total_length, 0);
    WMB.resize(total_length, 0);
    WMBP.resize(total_length, 0);
    WMBW.resize(total_length, 0);
    BE.resize(total_length, 0);

    rescale_pk_globals();
    exp_params_rescale(energy);
    W.resize(n + 1, scale[1]);
    WI.resize(total_length, scale[1]);

    /**     MEA       */
    // probs.resize(total_length,0);
}

W_final_pf::~W_final_pf() {}

void W_final_pf::exp_params_rescale(double mfe) {
    double e_per_nt, kT;
    kT = exp_params_->kT;

    e_per_nt = mfe * 1000. / this->n;

    exp_params_->pf_scale = exp(-(exp_params_->model_details.sfact * e_per_nt) / kT);

    if (exp_params_->pf_scale < 1.) exp_params_->pf_scale = 1.;

    exp_params_->pf_scale = 1.;

    this->scale[0] = 1.;
    this->scale[1] = (pf_t)(1. / exp_params_->pf_scale);
    this->expMLbase[0] = 1;
    this->expMLbase[1] = (pf_t)(exp_params_->expMLbase / exp_params_->pf_scale);

    this->expcp_pen[0] = 1;
    this->expcp_pen[1] = (pf_t)(expcp_penalty / exp_params_->pf_scale);
    this->expPUP_pen[0] = 1;
    this->expPUP_pen[1] = (pf_t)(expPUP_penalty / exp_params_->pf_scale);

    for (cand_pos_t i = 2; i <= this->n; i++) {
        this->scale[i] = this->scale[i / 2] * this->scale[i - (i / 2)];
        this->expMLbase[i] = (pf_t)pow(exp_params_->expMLbase, (double)i) * this->scale[i];
        this->expcp_pen[i] = (pf_t)pow(expcp_penalty, (double)i) * this->scale[i];
        this->expPUP_pen[i] = (pf_t)pow(expPUP_penalty, (double)i) * this->scale[i];
    }
}

void W_final_pf::rescale_pk_globals() {
    double kT = exp_params_->model_details.betaScale * (exp_params_->model_details.temperature + K0) * GASCONST; /* kT in cal/mol  */
    double TT = (exp_params_->model_details.temperature + K0) / (Tmeasure);
    int pf_smooth = exp_params_->model_details.pf_smooth;

    expPS_penalty = RESCALE_BF(PS_penalty, PS_penalty * 3, TT, kT);
    expPSM_penalty = RESCALE_BF(PSM_penalty, PSM_penalty * 3, TT, kT);
    expPSP_penalty = RESCALE_BF(PSP_penalty, PSP_penalty * 3, TT, kT);
    expPB_penalty = RESCALE_BF(PB_penalty, PB_penalty * 3, TT, kT);
    expPUP_penalty = RESCALE_BF(PUP_penalty, PUP_penalty * 3, TT, kT);
    expPPS_penalty = RESCALE_BF(PPS_penalty, PPS_penalty * 3, TT, kT);

    expa_penalty = RESCALE_BF(a_penalty, ML_closingdH, TT, kT);
    expb_penalty = RESCALE_BF(b_penalty, ML_interndH, TT, kT);
    expc_penalty = RESCALE_BF(c_penalty, ML_BASEdH, TT, kT);

    expap_penalty = RESCALE_BF(ap_penalty, ap_penalty * 3, TT, kT);
    expbp_penalty = RESCALE_BF(bp_penalty, bp_penalty * 3, TT, kT);
    expcp_penalty = RESCALE_BF(cp_penalty, cp_penalty * 3, TT, kT);
}

/**
 * In cases where the band border is not found, if specific cases are met, the value is Inf(i.e n) not -1.
 * When applied to WMBP, if all cases are 0, then we can proceed with WMBP
 * Mateo Jan 2025: Added to Fix WMBP problem
 */
int W_final_pf::compute_exterior_cases(cand_pos_t l, cand_pos_t j, sparse_tree &tree) {
    // Case 1 -> l is not covered
    bool case1 = tree.tree[l].parent->index <= 0;
    // Case 2 -> l is paired
    bool case2 = tree.tree[l].pair > 0;
    // Case 3 -> l is part of a closed subregion
    // bool case3 = 0;
    // Case 4 -> l.bp(l) i.e. l.j does not cross anything -- could I compare parents instead?
    bool case4 = j < tree.Bp(l, j);
    // By bitshifting each one, we have a more granular idea of what cases fail and is faster than branching
    return (case1 << 2) | (case2 << 1) | case4;
}

inline pf_t W_final_pf::to_Energy(pf_t energy, cand_pos_t length) {
    return ((-log(energy) - length * log(exp_params_->pf_scale)) * exp_params_->kT / 1000.0);
}

pf_t W_final_pf::hfold_pf(sparse_tree &tree) {

    for (cand_pos_t i = n; i >= 1; --i) {
        for (cand_pos_t j = i; j <= n; ++j) {

            const bool evaluate = tree.weakly_closed(i, j);
            const pair_type ptype_closing = pair[S_[i]][S_[j]];
            const bool restricted = tree.tree[i].pair == -1 || tree.tree[j].pair == -1;

            if (ptype_closing > 0 && evaluate && !restricted & !pk_only) compute_energy_restricted(i, j, tree);

            if (!pk_free) compute_pk_energies(i, j, tree);

            compute_WMv_WMp(i, j, tree.tree);
            compute_energy_WM_restricted(i, j, tree);
        }
    }
    scfg::compute_W_restricted(*this, tree);
    pf_t energy = to_Energy(W[n], n);

    // Base pair probability
    structure = std::string(n, '.');
    for (cand_pos_t i = 0; i < num_samples; ++i) {
        std::string structure(n, '.');
        Sample_W(1, n, structure, samples, tree);
        structures[structure]++;
    }
    std::unordered_map<std::string, int> fatgraphs;
    if(fatgraph){
        for(auto it: structures){
            std::string fatgraph = get_fatgraph(it.first);
            fatgraphs[fatgraph]+=it.second;
        }
        for(auto it: fatgraphs){
            std::cout << it.first << "  " << it.second << std::endl;
        }
    }

    pairing_tendency(samples, tree);
    this->frequency = (pf_t)structures[MFE_structure] / num_samples;     

    if (PSplot) {
        create_dot_plot(seq, tree.tree, MFE_structure, samples, num_samples);
    }

    return energy;
}
pf_t W_final_pf::hfold_MEA(sparse_tree &tree){
    pf_t MEA = compute_MEA(tree,1);
    return MEA;
}

pf_t W_final_pf::hfold_centroid(sparse_tree &tree){
    pf_t dist = 0;
    pf_t diversity = 0;
    std::string centroid = compute_centroid(tree,dist,diversity);
    this->centroid_structure = centroid;
    this->ensemble_diversity = diversity;
    return dist;
}

pf_t W_final_pf::compute_internal_restricted(cand_pos_t i, cand_pos_t j, std::vector<int> &up) {
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

void W_final_pf::compute_pk_energies(cand_pos_t i, cand_pos_t j, sparse_tree &tree) {

    cand_pos_t ij = index[i] + j - i;
    const pair_type ptype_closing = pair[S_[i]][S_[j]];
    bool weakly_closed_ij = tree.weakly_closed(i, j);

    if ((i == j || j - i < 4 || weakly_closed_ij)) {
        VP[ij] = 0;
        VPL[ij] = 0;
        VPR[ij] = 0;
    } else {
        if (ptype_closing > 0 && tree.tree[i].pair < -1 && tree.tree[j].pair < -1) compute_VP(i, j, tree);
        if (tree.tree[j].pair < -1) compute_VPL(i, j, tree);
        if (tree.tree[j].pair < j) compute_VPR(i, j, tree);
    }

    if (!((j - i - 1) <= TURN || (tree.tree[i].pair >= -1 && tree.tree[i].pair > j) || (tree.tree[j].pair >= -1 && tree.tree[j].pair < i)
          || (tree.tree[i].pair >= -1 && tree.tree[i].pair < i) || (tree.tree[j].pair >= -1 && j < tree.tree[j].pair))) {
        compute_WMBW(i, j, tree);
        compute_WMBP(i, j, tree);
        compute_WMB(i, j, tree);
    }

    if (!weakly_closed_ij) {
        WI[ij] = 0;
        WIP[ij] = 0;
    } else {
        compute_WI(i, j, tree);
        compute_WIP(i, j, tree);
    }
    cand_pos_t ip = tree.tree[i].pair; // i's pair ip should be right side so ip = )
    cand_pos_t jp = tree.tree[j].pair; // j's pair jp should be left side so jp = (
    compute_BE(i, ip, jp, j, tree);
}

void W_final_pf::compute_WI(cand_pos_t i, cand_pos_t j, sparse_tree &tree) {
    scfg::compute_WI_restricted(*this, i, j, tree);
}

void W_final_pf::compute_WIP(cand_pos_t i, cand_pos_t j, sparse_tree &tree) {
    scfg::compute_WIP_restricted(*this, i, j, tree);
}

void W_final_pf::compute_VPL(cand_pos_t i, cand_pos_t j, sparse_tree &tree) {
    scfg::compute_VPL_restricted(*this, i, j, tree);
}

void W_final_pf::compute_VPR(cand_pos_t i, cand_pos_t j, sparse_tree &tree) {
    scfg::compute_VPR_restricted(*this, i, j, tree);
}

void W_final_pf::compute_VP(cand_pos_t i, cand_pos_t j, sparse_tree &tree) {
    scfg::compute_VP_restricted(*this, i, j, tree);
}

void W_final_pf::compute_WMBW(cand_pos_t i, cand_pos_t j, sparse_tree &tree) {
    scfg::compute_WMBW_restricted(*this, i, j, tree);
}

void W_final_pf::compute_WMBP(cand_pos_t i, cand_pos_t j, sparse_tree &tree) {
    scfg::compute_WMBP_restricted(*this, i, j, tree);
}

void W_final_pf::compute_WMB(cand_pos_t i, cand_pos_t j, sparse_tree &tree) {
    scfg::compute_WMB_restricted(*this, i, j, tree);
}

void W_final_pf::compute_BE(cand_pos_t i, cand_pos_t j, cand_pos_t ip, cand_pos_t jp, sparse_tree &tree) {
    scfg::compute_BE_restricted(*this, i, j, ip, jp, tree);
}
