#include "pseudo_loop.hh"
#include "h_externs.hh"
#include "scfg/legacy_adapter.hh"



void pseudo_loop::back_track_vpr(seq_interval *cur_interval, sparse_tree &tree) {
    cand_pos_t i = cur_interval->i;
    cand_pos_t j = cur_interval->j;
    if (i >= j) return;
    energy_t min = INF, tmp = INF;
    cand_pos_t best_k = INF, best_row = -1;

    cand_pos_t max_i_bp = std::max(tree.B(i, j), tree.bp(i, j));

    for (cand_pos_t k = max_i_bp + 1; k < j; ++k) {
        tmp = get_VP(i, k) + get_WIP(k + 1, j);
        if (tmp < min) {
            best_k = k;
            best_row = 1;
            min = tmp;
        }
    }

    for (cand_pos_t k = max_i_bp + 1; k < j; ++k) {
        energy_t VP_energy = get_VP(i, k);
        bool can_pair = scfg::can_pair_right_span(tree, k, j);
        if (can_pair) tmp = VP_energy + static_cast<energy_t>((j - k) * cp_penalty);
        if (tmp < min) {
            best_k = k;
            best_row = 2;
            min = tmp;
        }
    }

    switch (best_row) {
    case 1:
        if (best_k != -1) {
            insert_node(i, best_k, P_VP);
        }
        if (best_k != -1) {
            insert_node(best_k + 1, j, P_WIP);
        }
        break;
    case 2:
        if (best_k != -1) {
            insert_node(i, best_k, P_VP);
        }
        break;
    }
}
