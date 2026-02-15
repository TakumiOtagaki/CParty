#include "pseudo_loop.hh"
#include "h_externs.hh"



static void ensure_pair_matrix_initialized_vp_case_5() {
    static bool initialized = false;
    if (!initialized) {
        make_pair_matrix();
        initialized = true;
    }
}


void pseudo_loop::back_track_vp_case_5(cand_pos_t i, cand_pos_t j, sparse_tree &tree, cand_pos_t Bp_ij, cand_pos_t B_ij, cand_pos_t b_ij,
                                       cand_pos_t bp_ij, int &min, int &best_row, int &best_ip, int &best_jp) {
    ensure_pair_matrix_initialized_vp_case_5();
    cand_pos_t min_borders = std::min((cand_pos_tu)Bp_ij, (cand_pos_tu)b_ij);
    cand_pos_t edge_i = std::min(i + MAXLOOP + 1, j - TURN - 1);
    min_borders = std::min({min_borders, edge_i});
    for (cand_pos_t k = i + 1; k < min_borders; ++k) {
        // Hosna: April 20, 2007
        // i and ip and j and jp should be in the same arc
        // it should also be the case that [i+1,ip-1] && [jp+1,j-1] are empty regions
        if (tree.tree[k].pair < -1 && (tree.up[(k)-1] >= ((k) - (i)-1))) {
            // Hosna, April 9th, 2007
            // whenever we use get_borders we have to check for the correct values
            cand_pos_t max_borders = std::max(bp_ij, B_ij) + 1;
            cand_pos_t edge_j = k + j - i - MAXLOOP - 2;
            max_borders = std::max({max_borders, edge_j});
            for (cand_pos_t l = j - 1; l > max_borders; --l) {
                pair_type ptype_closingkj = pair[S_[k]][S_[l]];
                if (tree.tree[l].pair < -1 && ptype_closingkj > 0 && (tree.up[(j)-1] >= ((j) - (l)-1))) {
                    // Hosna: April 20, 2007
                    // i and ip and j and jp should be in the same arc
                    energy_t tmp = get_e_intP(i, k, l, j) + get_VP(k, l);
                    if (tmp < min) {
                        min = tmp;
                        best_row = 5;
                        best_ip = k;
                        best_jp = l;
                    }
                }
            }
        }
    }
}
