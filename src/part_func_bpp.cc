#include "part_func.hh"

char W_final_pf::bpp_symbol(pf_t *P) {
    if (P[0] > 0.667) return '.';
    if (P[0] > (P[1] + P[2] + P[3] + P[4])) return ',';

    if (P[1] > 0.667) return '(';
    if (P[2] > 0.667) return ')';
    if ((P[1] + P[2]) > P[0]) {
        if ((P[1] / (P[1] + P[2])) > 0.667) return '{';

        if ((P[2] / (P[1] + P[2])) > 0.667)
            return '}';
    }

    if (P[3] > 0.667) return '[';
    if (P[4] > 0.667) return ']';
    if ((P[4] + P[5]) > P[0]) {
        if ((P[3] / (P[3] + P[5])) > 0.667) return '/';

        if ((P[4] / (P[3] + P[4])) > 0.667)
            return '\\';
    }
    return '|';
    
    // return ':';
}

void W_final_pf::pairing_tendency(std::unordered_map<std::pair<cand_pos_t, cand_pos_t>, cand_pos_t, SzudzikHash> &samples, sparse_tree &tree) {

    for (cand_pos_t j = 1; j <= n; j++) {
        pf_t P[5] = {1, 0, 0, 0, 0}; // unpaired, PK-free left, PK-free right, PK left, PK right
        for (cand_pos_t i = 1; i < j; i++) {
            bool weakly_closed_ij = tree.weakly_closed(i, j);
            std::pair<cand_pos_tu, cand_pos_tu> base_pair(i, j);
            pf_t probability_ij = (pf_t)samples[base_pair] / num_samples;
            if(weakly_closed_ij) P[2] += probability_ij; else P[4] += probability_ij;
            P[0] -= probability_ij;
        }
        for (cand_pos_t i = j + 1; i <= n; i++) {
            bool weakly_closed_ji = tree.weakly_closed(j, i);
            std::pair<cand_pos_tu, cand_pos_tu> base_pair(j, i);
            pf_t probability_ji = (pf_t)samples[base_pair] / num_samples;
            if(weakly_closed_ji) P[1] += probability_ji; else P[3] += probability_ji;
            P[0] -= probability_ji;
        }
        structure[j - 1] = bpp_symbol(P);
    }
}
