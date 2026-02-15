#include "pseudo_loop.hh"
#include "h_externs.hh"
#include "scfg/legacy_adapter.hh"



void pseudo_loop::back_track_vpl(seq_interval *cur_interval, sparse_tree &tree) {
    cand_pos_t i = cur_interval->i;
    cand_pos_t j = cur_interval->j;
    if (i >= j) return;
    energy_t min = INF, tmp = INF;
    cand_pos_t best_k = -1;

    cand_pos_t min_Bp_j = std::min((cand_pos_tu)tree.b(i, j), (cand_pos_tu)tree.Bp(i, j));
    for (cand_pos_t k = i + 1; k < min_Bp_j; ++k) {
        bool can_pair = scfg::can_pair_left_span(tree, i, k);
        if (can_pair) tmp = static_cast<energy_t>((k - i) * cp_penalty) + get_VP(k, j);
        if (tmp < min) {
            best_k = k;
            min = tmp;
        }
    }

    if (best_k != -1) {
        insert_node(best_k, j, P_VP);
    }
}
