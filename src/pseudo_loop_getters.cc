#include "pseudo_loop.hh"



energy_t pseudo_loop::get_WI(cand_pos_t i, cand_pos_t j) {
    if (i > j) return 0;
    cand_pos_t ij = index[i] + j - i;
    return WI[ij];
}


energy_t pseudo_loop::get_WIP(cand_pos_t i, cand_pos_t j) {
    if (i >= j) return INF;
    cand_pos_t ij = index[i] + j - i;
    return WIP[ij];
}


energy_t pseudo_loop::get_VP(cand_pos_t i, cand_pos_t j) {
    if (i >= j) return INF;
    cand_pos_t ij = index[i] + j - i;
    return VP[ij];
}


energy_t pseudo_loop::get_VPL(cand_pos_t i, cand_pos_t j) {
    if (i >= j) return INF;
    cand_pos_t ij = index[i] + j - i;
    return VPL[ij];
}


energy_t pseudo_loop::get_VPR(cand_pos_t i, cand_pos_t j) {
    if (i >= j) return INF;
    cand_pos_t ij = index[i] + j - i;
    return VPR[ij];
}


energy_t pseudo_loop::get_WMB(cand_pos_t i, cand_pos_t j) {
    if (i >= j) return INF;
    cand_pos_t ij = index[i] + j - i;
    return WMB[ij];
}


energy_t pseudo_loop::get_WMBW(cand_pos_t i, cand_pos_t j) {
    if (i >= j) return INF;
    cand_pos_t ij = index[i] + j - i;
    return WMBW[ij];
}


energy_t pseudo_loop::get_WMBP(cand_pos_t i, cand_pos_t j) {
    if (i >= j) return INF;
    cand_pos_t ij = index[i] + j - i;
    return WMBP[ij];
}


energy_t pseudo_loop::get_BE(cand_pos_t i, cand_pos_t j, cand_pos_t ip, cand_pos_t jp, sparse_tree &tree) {
    // Hosna, March 16, 2012,
    // i and j should be at least 3 bases apart
    if (j - i >= TURN && i >= 1 && i <= ip && ip < jp && jp <= j && j <= n && tree.tree[i].pair >= 0 && tree.tree[j].pair >= 0
        && tree.tree[ip].pair >= 0 && tree.tree[jp].pair >= 0 && tree.tree[i].pair == j && tree.tree[j].pair == i && tree.tree[ip].pair == jp
        && tree.tree[jp].pair == ip) {
        if (i == ip && j == jp && i < j) {
            return 0;
        }
        cand_pos_t iip = index[i] + ip - i;

        return BE[iip];
    } else {
        return INF;
    }
}
