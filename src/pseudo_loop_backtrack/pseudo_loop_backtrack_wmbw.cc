#include "pseudo_loop.hh"



void pseudo_loop::back_track_wmbw(seq_interval *cur_interval, sparse_tree &tree) {
    cand_pos_t i = cur_interval->i;
    cand_pos_t j = cur_interval->j;
    if (i >= j) return;
    cand_pos_t best_l = -1;
    energy_t min = INF;

    if (tree.tree[j].pair < j) {
        for (cand_pos_t l = i + 1; l < j; l++) {
            if (tree.tree[l].pair < 0 && tree.tree[l].parent->index > -1 && tree.tree[j].parent->index > -1
                && tree.tree[j].parent->index == tree.tree[l].parent->index) {
                energy_t tmp = get_WMBP(i, l) + get_WI(l + 1, j);
                if (tmp < min) {
                    min = tmp;
                    best_l = l;
                }
            }
        }
    }

    if (best_l > -1) {
        insert_node(i, best_l, P_WMBP);
        insert_node(best_l + 1, j, P_WI);
    }
}
