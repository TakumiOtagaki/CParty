#include "pseudo_loop.hh"
#include "h_externs.hh"



void pseudo_loop::back_track_be(seq_interval *cur_interval, sparse_tree &tree) {
    cand_pos_t i = cur_interval->i;
    cand_pos_t j = tree.tree[i].pair;
    cand_pos_t ip = cur_interval->j;
    cand_pos_t jp = tree.tree[ip].pair;
    if (i > ip || i > j || ip > jp || jp > j) {
        return;
    }

    f[i].pair = j;
    f[j].pair = i;
    this->structure[i] = '(';
    this->structure[j] = ')';
    f[i].type = P_BE;
    f[j].type = P_BE;
    f[ip].pair = jp;
    f[jp].pair = ip;
    this->structure[ip] = '(';
    this->structure[jp] = ')';
    f[ip].type = P_BE;
    f[jp].type = P_BE;

    energy_t min = INF, tmp = INF;
    cand_pos_t best_row = -1, best_l = INF;
    // case 1
    if (tree.tree[i + 1].pair == j - 1) {
        tmp = get_e_stP(i, j) + get_BE(i + 1, j - 1, ip, jp, tree);
        if (tmp < min) {
            min = tmp;
            best_row = 1;
        }
    }
    for (cand_pos_t l = i + 1; l <= ip; l++) {
        if (tree.tree[l].pair >= 0 && jp <= tree.tree[l].pair && tree.tree[l].pair < j) {
            cand_pos_t lp = tree.tree[l].pair;

            bool empty_region_il = (tree.up[(l)-1] >= l - i - 1);       // empty between i+1 and lp-1
            bool empty_region_lpj = (tree.up[(j)-1] >= j - lp - 1);     // empty between l+1 and ip-1
            bool weakly_closed_il = tree.weakly_closed(i + 1, l - 1);   // weakly closed between i+1 and lp-1
            bool weakly_closed_lpj = tree.weakly_closed(lp + 1, j - 1); // weakly closed between l+1 and ip-1

            if (empty_region_il && empty_region_lpj) {
                tmp = get_e_intP(i, l, lp, j) + get_BE(l, lp, ip, jp, tree);
                if (min > tmp) {
                    min = tmp;
                    best_row = 2;
                    best_l = l;
                }
            }

            // case 3
            if (weakly_closed_il && weakly_closed_lpj) {
                tmp = get_WIP(i + 1, l - 1) + get_BE(l, lp, ip, jp, tree) + get_WIP(lp + 1, j - 1);
                if (min > tmp) {
                    min = tmp;
                    best_row = 3;
                    best_l = l;
                }
            }

            // case 4
            if (weakly_closed_il && empty_region_lpj) {
                // Hosna: July 5th, 2007
                // After meeting with Anne and Cristina --> ap should have 2* bp to consider the biggest and the one that crosses
                // in a multiloop that spans a band
                tmp = get_WIP(i + 1, l - 1) + get_BE(l, lp, ip, jp, tree) + c_penalty * (j - lp - 1) + ap_penalty + 2 * bp_penalty;
                if (min > tmp) {
                    min = tmp;
                    best_row = 4;
                    best_l = l;
                }
            }

            // case 5
            if (empty_region_il && weakly_closed_lpj) {
                // Hosna: July 5th, 2007
                // After meeting with Anne and Cristina --> ap should have 2* bp to consider the biggest and the one that crosses
                // in a multiloop that spans a band
                tmp = ap_penalty + 2 * bp_penalty + c_penalty * (l - i - 1) + get_BE(l, lp, ip, jp, tree) + get_WIP(lp + 1, j - 1);
                if (min > tmp) {
                    min = tmp;
                    best_row = 5;
                    best_l = l;
                }
            }
        }
    }
    switch (best_row) {
    case 1:
        if (i + 1 <= ip) {
            insert_node(i + 1, ip, P_BE);
        }
        break;
    case 2:
        if (best_l != INF && best_l <= ip) {
            insert_node(best_l, ip, P_BE);
        }
        break;
    case 3:
        if (best_l != INF) {
            if (i + 1 <= best_l - 1) {
                insert_node(i + 1, best_l - 1, P_WIP);
            }
            if (best_l <= ip) {
                insert_node(best_l, ip, P_BE);
            }
            if (tree.tree[best_l].pair <= j - 1) {
                insert_node(tree.tree[best_l].pair + 1, j - 1, P_WIP);
            }
        }
        break;
    case 4:
        if (best_l != INF) {
            if (i + 1 <= best_l - 1) {
                insert_node(i + 1, best_l - 1, P_WIP);
            }
            if (best_l <= ip) {
                insert_node(best_l, ip, P_BE);
            }
        }
        break;
    case 5:
        if (best_l != INF) {
            if (best_l <= ip) {
                insert_node(best_l, ip, P_BE);
            }
            if (tree.tree[best_l].pair <= j - 1) {
                insert_node(tree.tree[best_l].pair + 1, j - 1, P_WIP);
            }
        }
        break;
    }
}
