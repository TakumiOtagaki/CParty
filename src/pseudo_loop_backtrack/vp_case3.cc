#include "pseudo_loop.hh"



void pseudo_loop::back_track_vp_case_3(cand_pos_t i, cand_pos_t j, sparse_tree &tree, cand_pos_t Bp_ij, cand_pos_t B_ij, cand_pos_t b_ij,
                                       cand_pos_t bp_ij, int &min, int &best_row) {
    // case 3
    //  Hosna April 9th, 2007
    //  checking the borders as they may be negative
    if ((tree.tree[i].parent->index) > 0 && (tree.tree[j].parent->index) > 0 && Bp_ij >= 0 && B_ij >= 0 && b_ij >= 0 && bp_ij >= 0) {
        energy_t WI_i_plus_Bp_minus = get_WI(i + 1, Bp_ij - 1);
        energy_t WI_B_plus_b_minus = get_WI(B_ij + 1, b_ij - 1);
        energy_t WI_bp_plus_j_minus = get_WI(bp_ij + 1, j - 1);
        energy_t tmp = WI_i_plus_Bp_minus + WI_B_plus_b_minus + WI_bp_plus_j_minus;
        if (tmp < min) {
            min = tmp;
            best_row = 3;
        }
    }
}
