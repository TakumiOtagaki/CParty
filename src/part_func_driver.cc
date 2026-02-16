#include "part_func.hh"
#include "dot_plot.hh"
#include "scfg/part_func_adapter.hh"

#include <iostream>

pf_t W_final_pf::hfold_pf(sparse_tree &tree) {
    ensure_pair_matrix_initialized();

    for (cand_pos_t i = n; i >= 1; --i) {
        for (cand_pos_t j = i; j <= n; ++j) {

            const bool evaluate = tree.weakly_closed(i, j);
            const pair_type ptype_closing = pair[S_[i]][S_[j]];
            const bool restricted = tree.tree[i].pair == -1 || tree.tree[j].pair == -1;

            if (ptype_closing > 0 && evaluate && !restricted & !pk_only) compute_energy_restricted(i, j, tree);

            if (!pk_free) compute_pk_energies(i, j, tree);

            compute_WMv_WMp(i, j, tree.tree);
            compute_energy_WM_restricted(i, j, tree);
        }
    }
    scfg::compute_W_restricted(*this, tree);
    pf_t energy = to_Energy(W[n], n);

    // Base pair probability
    structure = std::string(n, '.');
    for (cand_pos_t i = 0; i < num_samples; ++i) {
        std::string structure(n, '.');
        Sample_W(1, n, structure, samples, tree);
        structures[structure]++;
    }
    std::unordered_map<std::string, int> fatgraphs;
    if(fatgraph){
        for(auto it: structures){
            std::string fatgraph = get_fatgraph(it.first);
            fatgraphs[fatgraph]+=it.second;
        }
        for(auto it: fatgraphs){
            std::cout << it.first << "  " << it.second << std::endl;
        }
    }

    pairing_tendency(samples, tree);
    this->frequency = (pf_t)structures[MFE_structure] / num_samples;     

    if (PSplot) {
        create_dot_plot(seq, tree.tree, MFE_structure, samples, num_samples);
    }

    return energy;
}

pf_t W_final_pf::hfold_MEA(sparse_tree &tree){
    pf_t MEA = compute_MEA(tree,1);
    return MEA;
}

pf_t W_final_pf::hfold_centroid(sparse_tree &tree){
    pf_t dist = 0;
    pf_t diversity = 0;
    std::string centroid = compute_centroid(tree,dist,diversity);
    this->centroid_structure = centroid;
    this->ensemble_diversity = diversity;
    return dist;
}
