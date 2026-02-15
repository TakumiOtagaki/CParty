#include "pseudo_loop.hh"
#include "h_externs.hh"
#include "scfg/constraint_oracle.hh"
#include "scfg/legacy_adapter.hh"
#include "scfg/rules_pseudo_loop.hh"
#include <algorithm>
#include <iostream>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

static void ensure_pair_matrix_initialized() {
    static bool initialized = false;
    if (!initialized) {
        make_pair_matrix();
        initialized = true;
    }
}


energy_t pseudo_loop::compute_int(cand_pos_t i, cand_pos_t j, cand_pos_t k, cand_pos_t l, const paramT *params) {
    ensure_pair_matrix_initialized();
    const pair_type ptype_closing = pair[S_[i]][S_[j]];
    return E_IntLoop(k - i - 1, j - l - 1, ptype_closing, rtype[pair[S_[k]][S_[l]]], S1_[i + 1], S1_[j - 1], S1_[k - 1], S1_[l + 1],
                     const_cast<paramT *>(params));
}



energy_t pseudo_loop::get_e_stP(cand_pos_t i, cand_pos_t j) {
    if (i + 1 == j - 1) { // TODO: do I need something like that or stack is taking care of this?
        return INF;
    }
    energy_t ss = compute_int(i, j, i + 1, j - 1, params_);
    return lrint(e_stP_penalty * ss);
}



energy_t pseudo_loop::get_e_intP(cand_pos_t i, cand_pos_t ip, cand_pos_t jp, cand_pos_t j) {
    // Hosna Feb 12th, 2007:
    // this function is only being called in branch 5 of VP
    // and branch 2 of BE
    // in both cases regions [i,ip] and [jp,j] are closed regions
    energy_t e_int = compute_int(i, j, ip, jp, params_);
    energy_t energy = lrint(e_intP_penalty * e_int);
    return energy;
}
