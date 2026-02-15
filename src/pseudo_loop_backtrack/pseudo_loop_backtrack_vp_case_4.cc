#include "pseudo_loop.hh"
#include "h_externs.hh"



static void ensure_pair_matrix_initialized_vp_case_4() {
    static bool initialized = false;
    if (!initialized) {
        make_pair_matrix();
        initialized = true;
    }
}


void pseudo_loop::back_track_vp_case_4(cand_pos_t i, cand_pos_t j, sparse_tree &tree, int &min, int &best_row) {
    ensure_pair_matrix_initialized_vp_case_4();
    pair_type ptype_closingip1jm1 = pair[S_[i + 1]][S_[j - 1]];
    if (tree.tree[i + 1].pair < 0 && tree.tree[j - 1].pair < 0 && ptype_closingip1jm1 > 0) {
        energy_t tmp = get_e_stP(i, j) + get_VP(i + 1, j - 1);
        if (tmp < min) {
            min = tmp;
            best_row = 4;
        }
    }
}
