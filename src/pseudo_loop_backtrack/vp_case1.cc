#include "pseudo_loop.hh"



void pseudo_loop::back_track_vp_case_1(cand_pos_t i, cand_pos_t j, sparse_tree &tree, cand_pos_t Bp_ij, cand_pos_t B_ij, cand_pos_t b_ij,
                                       cand_pos_t bp_ij, int &min, int &best_row) {
    (void)b_ij;
    // case 1
    //  Hosna April 9th, 2007
    //  need to check the borders as they may be negative
    if ((tree.tree[i].parent->index) > 0 && (tree.tree[j].parent->index) < (tree.tree[i].parent->index) && Bp_ij >= 0 && B_ij >= 0 && bp_ij < 0) {
        energy_t WI_ipus1_BPminus = get_WI(i + 1, Bp_ij - 1);
        energy_t WI_Bplus_jminus = get_WI(B_ij + 1, j - 1);
        energy_t tmp = WI_ipus1_BPminus + WI_Bplus_jminus;
        if (tmp < min) {
            min = tmp;
            best_row = 1;
        }
    }
}
