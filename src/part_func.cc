#include "part_func.hh"
#include "dot_plot.hh"
#include "scfg/constraint_oracle.hh"
#include "scfg/part_func_adapter.hh"
#include "scfg/rules_part_func.hh"
#include "scfg/legacy_adapter.hh"

#include <algorithm>
#include <iostream>
#include <math.h>
#include <stdio.h>
#include <string>

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
