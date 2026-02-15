#include "pseudo_loop.hh"
#include "h_externs.hh"



void pseudo_loop::back_track_wmb(seq_interval *cur_interval, sparse_tree &tree) {
    cand_pos_t i = cur_interval->i;
    cand_pos_t j = cur_interval->j;
    if (i >= j) return;
    cand_pos_t best_l = -1, best_row = -1;
    energy_t tmp = INF, min = INF;

    // case 1
    if (tree.tree[j].pair >= 0 && j > tree.tree[j].pair && tree.tree[j].pair > i) {
        energy_t acc = INF;
        cand_pos_t bp_j = tree.tree[j].pair;
        for (cand_pos_t l = bp_j + 1; l < j; l++) {
            // Hosna: April 24, 2007
            // correct case 2 such that a multi-pseudoknotted
            // loop would not be treated as case 2
            cand_pos_t Bp_lj = tree.Bp(l, j);

            if (Bp_lj >= 0 && Bp_lj < n) {
                energy_t sum = get_BE(bp_j, j, tree.tree[Bp_lj].pair, Bp_lj, tree) + get_WMBP(i, l) + get_WI(l + 1, Bp_lj - 1);
                if (acc > sum) {
                    acc = sum;
                    best_l = l;
                }
            }
        }
        tmp = PB_penalty + acc;
        if (tmp < min) {
            min = tmp;
            best_row = 1;
        }
    }
    // case WMBP
    tmp = get_WMBP(i, j);
    if (tmp < min) {
        min = tmp;
        best_row = 2;
    }

    switch (best_row) {
    case 1:
        if (best_l > -1) {
            insert_node(i, best_l, P_WMBP);
            insert_node(best_l + 1, tree.Bp(best_l, j) - 1, P_WI);
            insert_node(tree.tree[j].pair, tree.tree[tree.Bp(best_l, j)].pair, P_BE);
        }
        break;
    case 2:
        insert_node(i, j, P_WMBP);
        break;
    }
}
