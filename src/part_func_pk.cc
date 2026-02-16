#include "part_func.hh"
#include "scfg/part_func_adapter.hh"

void W_final_pf::compute_pk_energies(cand_pos_t i, cand_pos_t j, sparse_tree &tree) {
    ensure_pair_matrix_initialized();

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
