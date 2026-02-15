#include "pseudo_loop.hh"
#include "h_externs.hh"



void pseudo_loop::back_track_wmbp(seq_interval *cur_interval, sparse_tree &tree) {
    cand_pos_t i = cur_interval->i;
    cand_pos_t j = cur_interval->j;
    if (i >= j) return;
    cand_pos_t best_l = -1, best_row = -1;
    energy_t min = INF;

    // case 1
    if (tree.tree[j].pair < 0) {
        energy_t acc = INF;
        cand_pos_t l3 = -1;
        cand_pos_t b_ij = tree.b(i, j);
        for (cand_pos_t l = i + 1; l < j; l++) {
            cand_pos_t bp_il = tree.bp(i, l);
            cand_pos_t Bp_lj = tree.Bp(l, j);
            // Mateo Jan 2025 Added exterior cases to consider when looking at band borders. Solved case of [.(.].[.).]
            int ext_case = compute_exterior_cases(l, j, tree);
            if ((b_ij > 0 && l < b_ij) || (b_ij < 0 && ext_case == 0)) {
                if (bp_il >= 0 && l > bp_il && Bp_lj > 0 && l < Bp_lj) { // bp(i,l) < l < Bp(l,j)

                    cand_pos_t B_lj = tree.B(l, j);
                    if (i <= tree.tree[l].parent->index && tree.tree[l].parent->index < j && l + TURN <= j) {
                        energy_t sum = get_BE(tree.tree[B_lj].pair, B_lj, tree.tree[Bp_lj].pair, Bp_lj, tree) + get_WMBP(i, l - 1)
                                       + get_VP(l, j);
                        if (acc > sum) {
                            acc = sum;
                            l3 = l;
                        }
                    }
                }
            }
        }
        energy_t tmp = 2 * PB_penalty + acc;
        if (tmp < min) {
            min = tmp;
            best_row = 1;
            best_l = l3;
        }
    }

    // case 2
    if (tree.tree[j].pair < 0 && tree.tree[i].pair < 0) {
        energy_t acc = INF;
        cand_pos_t l3 = -1;
        cand_pos_t b_ij = tree.b(i, j);
        for (cand_pos_t l = i + 1; l < j; l++) {
            cand_pos_t bp_il = tree.bp(i, l);
            cand_pos_t Bp_lj = tree.Bp(l, j);
            // Mateo Jan 2025 Added exterior cases to consider when looking at band borders. Solved case of [.(.].[.).]
            int ext_case = compute_exterior_cases(l, j, tree);
            if ((b_ij > 0 && l < b_ij) || (b_ij < 0 && ext_case == 0)) {
                if (bp_il >= 0 && l > bp_il && Bp_lj > 0 && l < Bp_lj) { // bp(i,l) < l < Bp(l,j)

                    cand_pos_t B_lj = tree.B(l, j);
                    if (i <= tree.tree[l].parent->index && tree.tree[l].parent->index < j && l + TURN <= j) {
                        energy_t sum = get_BE(tree.tree[B_lj].pair, B_lj, tree.tree[Bp_lj].pair, Bp_lj, tree) + get_WMBW(i, l - 1)
                                       + get_VP(l, j);
                        if (acc > sum) {
                            acc = sum;
                            l3 = l;
                        }
                    }
                }
            }
        }
        energy_t tmp = 2 * PB_penalty + acc;
        if (tmp < min) {
            min = tmp;
            best_row = 2;
            best_l = l3;
        }
    }

    // case 3
    energy_t temp = get_VP(i, j) + PB_penalty;
    if (temp < min) {
        min = temp;
        best_row = 3;
    }

    // case 4
    if (tree.tree[j].pair < 0 && tree.tree[i].pair >= 0) {
        cand_pos_t l1 = -1;
        energy_t acc = INF;
        for (cand_pos_t l = i + 1; l < j; l++) {
            // Hosna, April 9th, 2007
            // checking the borders as they may be negative
            // Hosna: July 5th, 2007:
            cand_pos_t bp_il = tree.bp(i, l);
            // removed bp(l)<0 as VP should handle that
            if (bp_il >= 0 && bp_il < n && l + TURN <= j) {
                // Hosna: April 19th, 2007
                // the chosen l should be less than border_b(i,j)
                energy_t BE_energy = get_BE(i, tree.tree[i].pair, bp_il, tree.tree[bp_il].pair, tree);
                energy_t WI_energy = get_WI(bp_il + 1, l - 1);
                energy_t VP_energy = get_VP(l, j);
                energy_t sum = BE_energy + WI_energy + VP_energy;
                if (acc > sum) {
                    acc = sum;
                    l1 = l;
                }
            }
        }
        energy_t tmp = 2 * PB_penalty + acc;
        if (tmp < min) {
            min = tmp;
            best_row = 4;
            best_l = l1;
        }
    }

    switch (best_row) {
    case 1:
        if (best_l > -1) {
            insert_node(i, best_l - 1, P_WMBP);
            insert_node(best_l, j, P_VP);
            insert_node(tree.tree[tree.B(best_l, j)].pair, tree.tree[tree.Bp(best_l, j)].pair, P_BE);
        }
        break;
    case 2:
        if (best_l > -1) {
            insert_node(tree.tree[tree.B(best_l, j)].pair, tree.tree[tree.Bp(best_l, j)].pair, P_BE);
            insert_node(i, best_l - 1, P_WMBW);
            insert_node(best_l, j, P_VP);
        }
        break;
    case 3:
        insert_node(i, j, P_VP);
        break;
    case 4:
        if (best_l > -1) {
            insert_node(i, tree.bp(i, best_l), P_BE);
            insert_node(tree.bp(i, best_l) + 1, best_l - 1, P_WI);
            insert_node(best_l, j, P_VP);
        }
        break;
    }
}
