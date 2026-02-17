#include "part_func.hh"
#include "dot_plot.hh"
#include "scfg/part_func_adapter.hh"

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <iostream>
#include <vector>

pf_t W_final_pf::hfold_pf(sparse_tree &tree) {
    ensure_pair_matrix_initialized();
    static bool local_pair_ready = false;
    if (!local_pair_ready) {
        make_pair_matrix();
        local_pair_ready = true;
    }
    const char *pf_debug_env = std::getenv("CPARTY_PF_DEBUG");
    const bool pf_debug = (pf_debug_env && *pf_debug_env != '\0' && std::strcmp(pf_debug_env, "0") != 0);

    for (cand_pos_t i = n; i >= 1; --i) {
        for (cand_pos_t j = i; j <= n; ++j) {

            const bool evaluate = tree.weakly_closed(i, j);
            const pair_type ptype_closing = pair[S_[i]][S_[j]];
            const bool restricted = tree.tree[i].pair == -1 || tree.tree[j].pair == -1;

            if (pf_debug && i == 1 && j == n) {
                std::cerr << "[PF_DEBUG] loop i=1 j=n evaluate=" << evaluate
                          << " ptype=" << ptype_closing
                          << " restricted_flag=" << restricted
                          << " pair_i=" << tree.tree[i].pair
                          << " pair_j=" << tree.tree[j].pair
                          << " S_i=" << static_cast<int>(S_[i])
                          << " S_j=" << static_cast<int>(S_[j])
                          << " energy_set=" << energy_set
                          << " noGU=" << noGU
                          << " pair[3][2]=" << pair[3][2]
                          << " pair[2][3]=" << pair[2][3]
                          << std::endl;
            }

            if (ptype_closing > 0 && evaluate && !restricted & !pk_only) {
                compute_energy_restricted(i, j, tree);
                if (pf_debug && i == 1 && j == n) {
                    const cand_pos_t ij = index[i] + j - i;
                    std::cerr << "[PF_DEBUG] V[1,n]=" << V[ij]
                              << " evaluate=" << evaluate
                              << " ptype=" << ptype_closing
                              << " restricted_flag=" << restricted
                              << " pair_i=" << tree.tree[i].pair
                              << " pair_j=" << tree.tree[j].pair
                              << std::endl;
                }
            }

            if (!pk_free) compute_pk_energies(i, j, tree);

            compute_WMv_WMp(i, j, tree.tree);
            compute_energy_WM_restricted(i, j, tree);
        }
    }
    scfg::compute_W_restricted(*this, tree);
    if (pf_debug) {
        std::cerr << "[PF_DEBUG] W[j]";
        for (cand_pos_t j = 1; j <= n; ++j) {
            std::cerr << " " << j << ":" << W[j];
        }
        std::cerr << std::endl;
    }
    const pf_t wn = W[n];
    if (pf_debug) {
        std::cerr << "[PF_DEBUG] W[n]=" << wn << " n=" << n << std::endl;
    }
    pf_t energy = to_Energy(wn, n);
    if (pf_debug) {
        std::cerr << "[PF_DEBUG] pf_energy_raw=" << energy << " isfinite=" << std::isfinite(energy) << std::endl;
    }

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
    if (pf_debug) {
        std::size_t pk_unique = 0;
        std::size_t pk_samples = 0;
        std::vector<std::pair<std::string, int>> top;
        top.reserve(structures.size());
        for (const auto &entry : structures) {
            top.push_back(entry);
            if (entry.first.find('[') != std::string::npos || entry.first.find(']') != std::string::npos) {
                pk_unique += 1;
                pk_samples += static_cast<std::size_t>(entry.second);
            }
        }
        std::sort(top.begin(), top.end(), [](const auto &lhs, const auto &rhs) {
            return lhs.second > rhs.second;
        });
        const int k = std::min<int>(5, static_cast<int>(top.size()));
        std::cerr << "[PF_DEBUG] sample_summary"
                  << " num_samples=" << num_samples
                  << " unique=" << structures.size()
                  << " pk_unique=" << pk_unique
                  << " pk_samples=" << pk_samples
                  << " mfe_freq=" << this->frequency
                  << " mfe_structure=" << MFE_structure
                  << std::endl;
        for (int i = 0; i < k; ++i) {
            std::cerr << "[PF_DEBUG] sample_top" << (i + 1)
                      << " count=" << top[i].second
                      << " structure=" << top[i].first
                      << std::endl;
        }
    }

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
