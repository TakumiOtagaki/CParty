#include "pseudo_loop.hh"
#include "h_externs.hh"
#include "scfg/legacy_adapter.hh"



void pseudo_loop::back_track_wip(seq_interval *cur_interval, sparse_tree &tree) {
    cand_pos_t i = cur_interval->i;
    cand_pos_t j = cur_interval->j;
    if (i == j) {
        return;
    }
    energy_t min = INF, tmp = INF;
    cand_pos_t best_row = -1, best_k = INF;

    tmp = V->get_energy(i, j) + bp_penalty;
    if (tmp < min) {
        min = tmp;
        best_row = 1;
    }

    tmp = get_WMB(i, j) + PSM_penalty + bp_penalty;
    if (tmp < min) {
        min = tmp;
        best_row = 2;
    }

    for (cand_pos_t k = i + 1; k < j - TURN - 1; ++k) {
        energy_t wi_1 = get_WIP(i, k - 1);
        tmp = wi_1 + V->get_energy(k, j);
        if (tmp < min) {
            min = tmp;
            best_row = 3;
            best_k = k;
        }
    }
    for (cand_pos_t k = i + 1; k < j - TURN - 1; ++k) {
        energy_t wi_1 = get_WIP(i, k - 1);
        tmp = wi_1 + get_WMB(k, j);
        if (tmp < min) {
            min = tmp;
            best_row = 4;
            best_k = k;
        }
    }
    for (cand_pos_t k = i + 1; k < j - TURN - 1; ++k) {
        bool can_pair = scfg::can_pair_left_span(tree, i, k);
        if (can_pair) tmp = static_cast<energy_t>((k - i) * cp_penalty) + V->get_energy(k, j);
        if (tmp < min) {
            min = tmp;
            best_row = 5;
            best_k = k;
        }
    }
    for (cand_pos_t k = i + 1; k < j - TURN - 1; ++k) {
        bool can_pair = scfg::can_pair_left_span(tree, i, k);
        if (can_pair) tmp = static_cast<energy_t>((k - i) * cp_penalty) + get_WMB(k, j);
        if (tmp < min) {
            min = tmp;
            best_row = 6;
            best_k = k;
        }
    }
    // case 2
    if (tree.tree[j].pair < 0) {
        tmp = get_WIP(i, j - 1) + cp_penalty;
        if (tmp < min) {
            min = tmp;
            best_row = 7;
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
        if (best_k != INF) {
            if (i <= best_k - 1) {
                insert_node(i, best_k - 1, P_WIP);
            }
            if (best_k < j) {
                insert_node(best_k, j, LOOP);
            }
        }
        break;
    case 4:
        if (best_k != INF) {
            if (i <= best_k - 1) {
                insert_node(i, best_k - 1, P_WIP);
            }
            if (best_k <= j) {
                insert_node(best_k, j, P_WMB);
            }
        }
        break;
    case 5:
        if (best_k != INF) {
            if (best_k < j) {
                insert_node(best_k, j, LOOP);
            }
        }
        break;
    case 6:
        if (best_k != INF) {
            if (best_k <= j) {
                insert_node(best_k, j, P_WMB);
            }
        }
        break;
    case 7:
        if (i <= j - 1) {
            insert_node(i, j - 1, P_WIP);
        }
        break;
    }
}
