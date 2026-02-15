#include "pseudo_loop.hh"
#include "h_externs.hh"



void pseudo_loop::back_track_wi(seq_interval *cur_interval, sparse_tree &tree) {
    cand_pos_t i = cur_interval->i;
    cand_pos_t j = cur_interval->j;
    if (i >= j) {
        return;
    }
    energy_t min = INF, tmp = INF;
    cand_pos_t best_row = -1, best_t = -1;

    tmp = V->get_energy(i, j) + PPS_penalty;
    if (tmp < min) {
        min = tmp;
        best_row = 1;
    }

    tmp = get_WMB(i, j) + PSP_penalty + PPS_penalty;
    if (tmp < min) {
        min = tmp;
        best_row = 2;
    }

    for (cand_pos_t t = i + 1; t < j; t++) {
        energy_t wi_1 = get_WI(i, t - 1);
        energy_t v_energy = wi_1 + V->get_energy(t, j) + PPS_penalty;
        if (v_energy < min) {
            min = v_energy;
            best_row = 3;
            best_t = t;
        }
    }

    for (cand_pos_t t = i + 1; t < j; t++) {
        energy_t wi_1 = get_WI(i, t - 1);
        energy_t wmb_energy = wi_1 + get_WMB(t, j) + PSP_penalty + PPS_penalty;
        if (wmb_energy < min) {
            min = wmb_energy;
            best_row = 4;
            best_t = t;
        }
    }
    if (tree.tree[j].pair < 0) {
        tmp = get_WI(i, j - 1) + PUP_penalty;
        if (tmp < min) {
            min = tmp;
            best_row = 5;
        }
    }

    switch (best_row) {
    case 1:
        if (i < j) {
            insert_node(i, j, LOOP);
        }
        break;

    case 2:
        if (i < j) {
            insert_node(i, j, P_WMB);
        }
        break;
    case 3:
        if (best_t != -1) {
            if (i <= best_t - 1) {
                insert_node(i, best_t - 1, P_WI);
            }
            if (best_t < j) {
                insert_node(best_t, j, LOOP);
            }
        }
        break;
    case 4:
        if (best_t != -1) {
            if (i <= best_t - 1) {
                insert_node(i, best_t - 1, P_WI);
            }
            if (best_t < j) {
                insert_node(best_t, j, P_WMB);
            }
        }
        break;
    case 5:
        if (i < j) {
            insert_node(i, j - 1, P_WI);
        }
        break;
    }
}
