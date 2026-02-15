#include "pseudo_loop.hh"



void pseudo_loop::back_track_vp_preamble(seq_interval *cur_interval) {
    cand_pos_t i = cur_interval->i;
    cand_pos_t j = cur_interval->j;
    f[i].pair = j;
    f[j].pair = i;
    this->structure[i] = '[';
    this->structure[j] = ']';
    // printf("----> original VP: adding (%d,%d) <-------\n",i,j);
    f[i].type = P_VP;
    f[j].type = P_VP;
}
