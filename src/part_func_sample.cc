#include "part_func.hh"
#include "scfg/legacy_adapter.hh"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define debug 0

extern double expPS_penalty;
extern double expPSM_penalty;
extern double expPSP_penalty;
extern double expPB_penalty;
extern double expPUP_penalty;
extern double expPPS_penalty;

extern double expa_penalty;
extern double expb_penalty;
extern double expc_penalty;

extern double expap_penalty;
extern double expbp_penalty;
extern double expcp_penalty;

extern double expstart_hybrid_penalty;

void W_final_pf::Sample_W(cand_pos_t start, cand_pos_t end, std::string &structure,
                          std::unordered_map<std::pair<cand_pos_t, cand_pos_t>, cand_pos_t, SzudzikHash> &samples, sparse_tree &tree) {
    if (debug) printf("W at %d and %d with W[j]=%f,%f\n", start, end, W[end], to_Energy(W[end], end));
    cand_pos_t j = end;
    cand_pos_t m = end;

    pf_t W_temp = 0;
    if (end > start) {
        for (; j > start; --j) {         // Moving through the unpaired bases in j
            W_temp = W[j - 1] * scale[1];
            if (tree.tree[j].pair < 0) { // Checking if j can be unpaired
                pf_t r = vrna_urn() * W[j];
                if (r > W_temp) { // Checking if our random sample means j is paired or unpaired
                    break;        // j is paired
                }
            } else {
                break; // j can't be unpaired so it must be paired
            }
        }
        if (j <= start + TURN) return; // No more base pairs can occur, but still successful
        pf_t r = vrna_urn() * (W[j] - W_temp);
        std::vector<cand_pos_t> is = boustrophedon(start, j - 1); // applies an alternating list so that the base pairing isn't biased to the right side
        cand_pos_t bous_n = is.size();
        pf_t qt = 0;
        cand_pos_t k = start;
        bool pseudoknot = false;
        if (tree.weakly_closed(1, j)) {
            for (m = 1; m < bous_n; ++m) {
                k = is[m];
                if (tree.weakly_closed(1, k - 1)) {
                    pf_t acc = (k > 1) ? W[k - 1] : 1;
                    pf_t Wkl = acc * get_energy(k, j) * exp_Extloop(k, j);
                    qt += Wkl;
                    if (qt > r) {
                        break; // k pairs with j
                    }

                    if (k == 1 || tree.weakly_closed(k, j)) {
                        Wkl = acc * get_energy_WMB(k, j) * expPS_penalty;
                        qt += Wkl;
                        if (qt > r) {
                            pseudoknot = true;
                            break; // k pairs with j as a pseudoknot
                        }
                    }
                }
            }
        }
        if (k + start > j) {
            printf("backtracking failed in ext loop at %d and %d with W[j] = %f, qt:%f < r:%f\n", start, end, W[j], qt, r);
            exit(0); /* error */
        }
        Sample_W(start, k - 1, structure, samples, tree);
        if (!pseudoknot) {
            Sample_V(k, j, structure, samples, tree);
        } else {
            Sample_WMB(k, j, structure, samples, tree);
        }
    }
}

void W_final_pf::Sample_V(cand_pos_t i, cand_pos_t j, std::string &structure,
                          std::unordered_map<std::pair<cand_pos_t, cand_pos_t>, cand_pos_t, SzudzikHash> &samples, sparse_tree &tree) {
    ensure_pair_matrix_initialized();
    if (debug) printf("V at %d and %d\n", i, j);

    cand_pos_t k = i;
    cand_pos_t l = j;
    structure[i - 1] = '(';
    structure[j - 1] = ')';

    pf_t qbr = get_energy(i, j);
    pf_t V_temp = 0;

    std::pair<cand_pos_tu, cand_pos_tu> base_pair(i, j);
    std::pair<cand_pos_tu, cand_pos_tu> base_pair_reversed(j, i);
    ++samples[base_pair]; // Increments the base pair found in V
    ++samples[base_pair_reversed];

    pf_t r = vrna_urn() * qbr;
    pf_t qbt1 = 0;
    bool canH = !(tree.up[j - 1] < (j - i - 1));

    if (canH) V_temp = HairpinE(i, j);

    qbt1 += V_temp;
    if (qbt1 >= r) return;
    cand_pos_t max_k = std::min(j - TURN - 2, i + MAXLOOP + 1); // i+1+tree.up[i+1]?
    const pair_type ptype_closing = pair[S_[i]][S_[j]];
    for (k = i + 1; k <= max_k; k++) {
        if (tree.up[k - 1] >= (k - i - 1)) {
            cand_pos_t min_l = std::max(k + TURN + 1 + MAXLOOP + 2, k + j - i) - MAXLOOP - 2;
            for (l = j - 1; l >= min_l; --l) {
                if (tree.up[j - 1] >= (j - l - 1)) {
                    cand_pos_t u1 = k - i - 1;
                    cand_pos_t u2 = j - l - 1;
                    V_temp = get_energy(k, l)
                             * exp_E_IntLoop(u1, u2, ptype_closing, rtype[pair[S_[k]][S_[l]]], S1_[i + 1], S1_[j - 1], S1_[k - 1], S1_[l + 1],
                                             exp_params_);
                    V_temp *= scale[u1 + u2 + 2];
                    qbt1 += V_temp;
                    if (qbt1 >= r) break;
                }
            }
            if (qbt1 >= r) break;
        }
    }
    if (qbt1 >= r) {
        Sample_V(k, l, structure, samples, tree); // Backtrack the internal loop
        return;
    }

    V_temp = get_energy_VM(i, j); // VM includes everything since it includes the basepair (i.e. not like WM2 region), so is this fine?
    qbt1 += V_temp;
    if (qbt1 < r) {
        printf("Backtracking failed for pair (%d,%d)\n", i, j);
        exit(0);
    }

    // Must be a multiloop
    Sample_VM(i, j, structure, samples, tree);
}

void W_final_pf::Sample_VM(cand_pos_t i, cand_pos_t j, std::string &structure,
                           std::unordered_map<std::pair<cand_pos_t, cand_pos_t>, cand_pos_t, SzudzikHash> &samples, sparse_tree &tree) {
    if (debug) printf("VM at %d and %d\n", i, j);
    cand_pos_t k;
    pf_t qt = 0;
    if ((i + 1) + 2 * TURN + 2 >= (j - 1)) {
        printf("backtracking impossible for VM[%d, %d]\n", i, j);
        exit(0); /* error */
    }
    pf_t V_temp = 0.;
    pf_t VM_inside = get_energy_VM(i, j) / scale[2]; // If I remove scale from VM's saved values, I may save time here.
    pf_t r = vrna_urn() * VM_inside;
    bool unpaired = false;
    bool pseudoknot = false;
    for (k = i + 1; k <= j - TURN - 1; ++k) {
        V_temp = get_energy_WM(i + 1, k - 1) * get_energy_WMv(k, j - 1) * exp_Mbloop(i, j) * exp_params_->expMLclosing;
        qt += V_temp;
        if (qt > r) {
            break;
        }

        V_temp = (get_energy_WM(i + 1, k - 1) * get_energy_WMp(k, j - 1) * exp_Mbloop(i, j) * exp_params_->expMLclosing);
        qt += V_temp;
        if (qt > r) {
            pseudoknot = true;
            break;
        }

        if (scfg::can_pair_left_span(tree, i + 1, k)) {
            V_temp = (expMLbase[k - i - 1] * get_energy_WMp(k, j - 1) * exp_Mbloop(i, j) * exp_params_->expMLclosing);
            qt += V_temp;
            if (qt > r) {
                unpaired = true;
                pseudoknot = true;
                break;
            }
        }
    }
    if (k > j - TURN) {
        printf("backtracking failed for VM at i=%d and j =%d\n", i, j);
        exit(0);
    }

    if (!unpaired) {
        Sample_WM(i + 1, k - 1, structure, samples, tree);
    }
    if (!pseudoknot) { // Case 1
        Sample_WMV(k, j - 1, structure, samples, tree);
    } else { // Case 2 or 3
        Sample_WMP(k, j - 1, structure, samples, tree);
    }
}

void W_final_pf::Sample_WM(cand_pos_t i, cand_pos_t j, std::string &structure,
                           std::unordered_map<std::pair<cand_pos_t, cand_pos_t>, cand_pos_t, SzudzikHash> &samples, sparse_tree &tree) {
    if (debug) printf("WM at %d and %d\n", i, j);
    cand_pos_t k;
    pf_t qt = 0;
    pf_t qbt1 = 0;
    pf_t qbt2 = 0;
    bool unpaired = false;
    bool pseudoknot = false;

    pf_t V_temp = 0.;

    if (i + TURN >= j) {
        // return;
        printf("backtracking impossible for WM[%u, %u]\n", i, j);
        exit(0); /* error */
    }

    for (; j > i + TURN; --j) {
        if (tree.tree[j].pair < 0) {
            pf_t r = vrna_urn() * (get_energy_WM(i, j));

            V_temp = get_energy_WM(i, j - 1) * expMLbase[1];
            qt = V_temp;
            if (r > qt) {
                break;
            }
        } else {
            break; // j can't be unpaired so it must be paired
        }
    }

    if (i + TURN == j) {
        printf("backtracking failed for WM\n");
        exit(0); /* error */
    }

    qt = 0.;
    pf_t qm_rem = get_energy_WM(i, j) - V_temp;
    pf_t r = vrna_urn() * qm_rem;
    for (k = i; k < j - TURN; ++k) {
        qbt1 = get_energy(k, j) * exp_MLstem(k, j);
        qbt2 = get_energy_WMB(k, j) * expPSM_penalty * expb_penalty;
        bool can_pair = scfg::can_pair_left_span(tree, i, k);
        if (can_pair) {

            V_temp = static_cast<pf_t>(expMLbase[k - i]) * qbt1;
            qt += V_temp;
            if (qt >= r) {
                unpaired = true;
                break;
            }

            V_temp = static_cast<pf_t>(expMLbase[k - i]) * qbt2;
            qt += V_temp;
            if (qt >= r) {
                unpaired = true;
                pseudoknot = true;
                break;
            }
		}

		V_temp = get_energy_WM(i, k - 1) * qbt1;
		qt += V_temp;
		if (qt >= r) break;

		V_temp = get_energy_WM(i, k - 1) * qbt2;
		qt += V_temp;
		if (qt >= r) {
			pseudoknot = true;
			break;
		}
        
    }
    if (k > j - TURN || qt < r) {
        printf("backtracking failed for WM at i=%d and j =%d with k=%d, qt=%f and r =%f and qt<r=%d\n", i, j,k,qt,r,qt<r);
        exit(0);
    }
    if (!unpaired) {
        Sample_WM(i, k - 1, structure, samples, tree);
    }
    if (!pseudoknot) {
        Sample_V(k, j, structure, samples, tree);
    } else {
        Sample_WMB(k, j, structure, samples, tree);
    }
}

void W_final_pf::Sample_WMV(cand_pos_t i, cand_pos_t j, std::string &structure,
                            std::unordered_map<std::pair<cand_pos_t, cand_pos_t>, cand_pos_t, SzudzikHash> &samples, sparse_tree &tree) {
    if (debug) printf("WMv at %d and %d\n", i, j);
    pf_t qt = 0;

    pf_t V_temp = 0.;

    for (; j > i + TURN; --j) {
        if (tree.tree[j].pair < 0) { // Checking if j can be unpaired
            pf_t r = vrna_urn() * get_energy_WMv(i, j);

            V_temp = get_energy_WMv(i, j - 1) * expMLbase[1];
            qt = V_temp;
            if (r > qt) {
                break;
            }
        } else {
            break; // j can't be unpaired so it must be paired
        }
    }

    if (i + TURN == j) {
        printf("backtracking failed for WMV\n");
        exit(0); /* error */
    }

    Sample_V(i, j, structure, samples, tree);
}

void W_final_pf::Sample_WMP(cand_pos_t i, cand_pos_t j, std::string &structure,
                            std::unordered_map<std::pair<cand_pos_t, cand_pos_t>, cand_pos_t, SzudzikHash> &samples, sparse_tree &tree) {
    if (debug) printf("WMp at %d and %d\n", i, j);
    pf_t qt = 0;

    pf_t V_temp = 0.;

    for (; j > i + TURN; --j) {
        if (tree.tree[j].pair < 0) { // Checking if j can be unpaired
            pf_t r = vrna_urn() * get_energy_WMp(i, j);

            V_temp = get_energy_WMp(i, j - 1) * expMLbase[1];
            qt = V_temp;
            if (r > qt) {
                break;
            }
        } else {
            break; // j can't be unpaired so it must be paired
        }
    }

    if (i + TURN == j) { // I'm kinda assuming something like ([..)] for this
        printf("backtracking failed for WMP\n");
        exit(0); /* error */
    }

    Sample_WMB(i, j, structure, samples, tree);
}

void W_final_pf::Sample_WMB(cand_pos_t i, cand_pos_t j, std::string &structure,
                            std::unordered_map<std::pair<cand_pos_t, cand_pos_t>, cand_pos_t, SzudzikHash> &samples, sparse_tree &tree) {
    if (debug) printf("WMB at %d and %d\n", i, j);
    cand_pos_t l = j;
    pf_t qt = 0;
    cand_pos_t Bp_lj = 0;
    cand_pos_t bp_j = 0;

    pf_t V_temp = 0.;

    pf_t r = vrna_urn() * get_energy_WMB(i, j);

    if (tree.tree[j].pair >= 0 && j > tree.tree[j].pair && tree.tree[j].pair > i) {
        bp_j = tree.tree[j].pair;
        for (l = (bp_j + 1); (l < j); ++l) {
            // if(tree.tree[l].pair>0) continue;
            Bp_lj = tree.Bp(l, j);
            if (Bp_lj >= 0 && Bp_lj < n) {
                V_temp = get_BE(bp_j, j, tree.tree[Bp_lj].pair, Bp_lj, tree) * get_energy_WMBP(i, l) * get_energy_WI(l + 1, Bp_lj - 1) * expPB_penalty;
                qt += V_temp;
                if (qt > r) {
                    break;
                }
            }
        }
    }
    if (qt >= r) { // I could put this in the for loop then just do sample_WMBP if it doesn't sample in there
        Sample_BE(bp_j, j, tree.tree[Bp_lj].pair, Bp_lj, structure, samples, tree);
        Sample_WMBP(i, l, structure, samples, tree);
        Sample_WI(l + 1, Bp_lj - 1, structure, samples, tree);
		return;
    }
    V_temp = get_energy_WMBP(i, j);
    qt += V_temp;
    if (qt <= r) {
        printf("backtracking failed for WMB\n");
        exit(0); /* error */
    }
    Sample_WMBP(i, j, structure, samples, tree);
}

void W_final_pf::Sample_WI(cand_pos_t i, cand_pos_t j, std::string &structure,
                           std::unordered_map<std::pair<cand_pos_t, cand_pos_t>, cand_pos_t, SzudzikHash> &samples, sparse_tree &tree) {
    if (debug) printf("WI at %d and %d\n", i, j);
    cand_pos_t k;
    pf_t qt = 0, qbt1 = 0, qbt2 = 0;
    bool pseudoknot = false;

    pf_t V_temp = 0;
    if (j > i) {
        for (; j > i + TURN; --j) {
            if (tree.tree[j].pair < 0) { // Checking if j can be unpaired
                pf_t r = vrna_urn() * (get_energy_WI(i, j));

                V_temp = get_energy_WI(i, j - 1) * expPUP_pen[1];
                qt = V_temp;
                if (r > qt) {
                    break;
                }
            } else {
                break; // j can't be unpaired so it must be paired
            }
        }
        if (j <= i + TURN) return; // No more base pairs can occur, but still successful

        qt = 0;
        pf_t qm_rem = get_energy_WI(i, j) - V_temp;

        pf_t r = vrna_urn() * qm_rem;

        for (k = i; k <= j - TURN - 1; k++) {
            qbt1 = get_energy(k, j) * expPPS_penalty;
            qbt2 = get_energy_WMB(k, j) * expPSP_penalty * expPPS_penalty;

            V_temp = qbt1 * get_energy_WI(i, k - 1);
            qt += V_temp;
            if (qt >= r) break;

            V_temp = qbt2 * get_energy_WI(i, k - 1);
            qt += V_temp;
            if (qt >= r) {
                pseudoknot = true;
                break;
            }
        }

        Sample_WI(i, k - 1, structure, samples, tree);
        if (!pseudoknot) {
            Sample_V(k, j, structure, samples, tree);
        } else {
            Sample_WMB(k, j, structure, samples, tree);
        }
    }
}

void W_final_pf::Sample_WIP(cand_pos_t i, cand_pos_t j, std::string &structure,
                            std::unordered_map<std::pair<cand_pos_t, cand_pos_t>, cand_pos_t, SzudzikHash> &samples, sparse_tree &tree) {
    if (debug) printf("WIP at %d and %d\n", i, j);
    cand_pos_t k;
    pf_t fbd = 0;
    pf_t qt = 0, qbt1 = 0, qbt2 = 0;
    bool unpaired = false;
    bool pseudoknot = false;

    pf_t V_temp = 0;
    if (j <= i) return;
    for (; j > i + TURN; --j) {
        if (tree.tree[j].pair < 0) { // Checking if j can be unpaired
            pf_t r = vrna_urn() * (get_energy_WIP(i, j) - fbd);

            V_temp = get_energy_WIP(i, j - 1) * expcp_pen[1];
            qt = V_temp;
            if (r > qt) {
                break;
            }
        } else {
            break; // j can't be unpaired so it must be paired
        }
    }
    if (i + TURN == j) {
        printf("backtracking failed for WIP\n");
        exit(0); /* error */
    }

    qt = 0;
    pf_t qm_rem = get_energy_WIP(i, j) - V_temp;

    pf_t r = vrna_urn() * (qm_rem - fbd);

    for (k = i; k < j - TURN; ++k) {
        qbt1 = get_energy(k, j) * expbp_penalty;
        qbt2 = get_energy_WMB(k, j) * expbp_penalty * expPSM_penalty;

        bool can_pair = scfg::can_pair_left_span(tree, i, k);
        if (can_pair) {
            V_temp = qbt1 * expcp_pen[k - i];
            qt += V_temp;
            if (qt >= r) {
                unpaired = true;
                break;
            }

            V_temp = qbt2 * expcp_pen[k - i];
            qt += V_temp;
            if (qt >= r) {
                unpaired = true;
                pseudoknot = true;
                break;
            }
        }

        if (k >= i) { // It had > but for me, it should be >=?
            V_temp = qbt1 * get_energy_WM(i, k - 1);
            qt += V_temp;
            if (qt >= r) break;

            V_temp = qbt2 * get_energy_WM(i, k - 1);
            qt += V_temp;
            if (qt >= r) {
                pseudoknot = true;
                break;
            }
        }
    }

    if (k + TURN >= j) {
        printf("backtracking failed for WIP right base pair with k=%d and j =%d, and qt=%f with r-%f\n", k, j, qt, r);
        exit(0);
    }
    if (!unpaired) {
        Sample_WIP(i, k - 1, structure, samples, tree);
    }
    if (!pseudoknot) {
        Sample_V(k, j, structure, samples, tree);
    } else {
        Sample_WMB(k, j, structure, samples, tree);
    }
}

void W_final_pf::Sample_WMBW(cand_pos_t i, cand_pos_t j, std::string &structure,
                             std::unordered_map<std::pair<cand_pos_t, cand_pos_t>, cand_pos_t, SzudzikHash> &samples, sparse_tree &tree) {
    if (debug) printf("WMBW at %d and %d\n", i, j);
    cand_pos_t l = j;
    pf_t fbd = 0;
    pf_t qt = 0;

    pf_t V_temp = 0;
    pf_t r = vrna_urn() * (get_energy_WMBW(i, j) - fbd);
    if (tree.tree[j].pair < j) {
        for (l = i + 1; l < j; l++) {
            if (tree.tree[l].pair < 0 && tree.tree[l].parent->index > -1 && tree.tree[j].parent->index > -1
                && tree.tree[j].parent->index == tree.tree[l].parent->index) {
                V_temp = get_energy_WMBP(i, l) * get_energy_WI(l + 1, j);
                qt += V_temp;
                if (qt >= r) {
                    break;
                }
            }
        }
    }
    if (l >= j) {
        printf("Backtracking failed in WMBW for pair (%d,%d) with qt=%f < r=%f and l = %d\n", i, j, qt, r, l);
        exit(0);
    }
    Sample_WMBP(i, l, structure, samples, tree);
    Sample_WI(l + 1, j, structure, samples, tree);
	return;
}

void W_final_pf::Sample_WMBP(cand_pos_t i, cand_pos_t j, std::string &structure,
                             std::unordered_map<std::pair<cand_pos_t, cand_pos_t>, cand_pos_t, SzudzikHash> &samples, sparse_tree &tree) {
    if (debug) printf("WMBP at %d and %d\n", i, j);
    cand_pos_t l = j;
    pf_t qt = 0;
    cand_pos_t bp_il = 0;
    cand_pos_t Bp_lj = 0;
    cand_pos_t B_lj = 0;
    cand_pos_t b_ij = tree.b(i, j);
    bool case1 = false, case2 = false, case4 = false;

    pf_t V_temp = 0;
    pf_t r = vrna_urn() * get_energy_WMBP(i, j);

    if (tree.tree[j].pair < 0) {
        for (l = i + 1; l < j - TURN; ++l) {
            // Mateo Jan 2025 Added exterior cases to consider when looking at band borders. Solved case of [.(.].[.).]
            int ext_case = compute_exterior_cases(l, j, tree);
            if ((b_ij > 0 && l < b_ij) || (b_ij < 0 && ext_case == 0)) {
                bp_il = tree.bp(i, l);
                Bp_lj = tree.Bp(l, j);
                if (bp_il >= 0 && l > bp_il && Bp_lj > 0 && l < Bp_lj) {
                    B_lj = tree.B(l, j);
                    if (i <= tree.tree[l].parent->index && tree.tree[l].parent->index < j && l + TURN <= j) {
                        V_temp = get_BE(tree.tree[B_lj].pair, B_lj, tree.tree[Bp_lj].pair, Bp_lj, tree) * get_energy_WMBP(i, l - 1)
                                 * get_energy_VP(l, j) * pow(expPB_penalty, 2);
                        qt += V_temp;
                        if (qt >= r) {
                            case1 = true;
                            break;
                        }
                    }
                }
            }
        }
    }
    if (case1) {
        Sample_BE(tree.tree[B_lj].pair, B_lj, tree.tree[Bp_lj].pair, Bp_lj, structure, samples, tree);
        Sample_WMBP(i, l - 1, structure, samples, tree);
        Sample_VP(l, j, structure, samples, tree);
        return;
    }

    if (tree.tree[j].pair < 0) {
        for (l = i + 1; l < j - TURN; l++) {
            bp_il = tree.bp(i, l);
            Bp_lj = tree.Bp(l, j);
            // Mateo Jan 2025 Added exterior cases to consider when looking at band borders. Solved case of [.(.].[.).]
            int ext_case = compute_exterior_cases(l, j, tree);
            if ((b_ij > 0 && l < b_ij) || (b_ij < 0 && ext_case == 0)) {
                if (bp_il >= 0 && l > bp_il && Bp_lj > 0 && l < Bp_lj) {
                    B_lj = tree.B(l, j);
                    if (i <= tree.tree[l].parent->index && tree.tree[l].parent->index < j && l + TURN <= j) {
                        V_temp = get_BE(tree.tree[B_lj].pair, B_lj, tree.tree[Bp_lj].pair, Bp_lj, tree) * get_energy_WMBW(i, l - 1)
                                 * get_energy_VP(l, j) * pow(expPB_penalty, 2);
                        qt += V_temp;
                        if (qt >= r) {
                            case2 = true;
                            break;
                        }
                    }
                }
            }
        }
    }
    if (case2) {
        Sample_BE(tree.tree[B_lj].pair, B_lj, tree.tree[Bp_lj].pair, Bp_lj, structure, samples, tree);
        Sample_WMBW(i, l - 1, structure, samples, tree);
        Sample_VP(l, j, structure, samples, tree);
        return;
    }

    V_temp = get_energy_VP(i, j) * expPB_penalty;
    qt += V_temp;
    if (qt >= r) {
        Sample_VP(i, j, structure, samples, tree);
        return;
    }

    if (tree.tree[j].pair < 0 && tree.tree[i].pair >= 0) {
        for (l = i + 1; l < j; l++) {
            bp_il = tree.bp(i, l);
            if (bp_il >= 0 && bp_il < n && l + TURN <= j) {
                if (i <= tree.tree[l].parent->index && tree.tree[l].parent->index < j && l + TURN <= j) {
                    V_temp = get_BE(i, tree.tree[i].pair, bp_il, tree.tree[bp_il].pair, tree) * get_energy_WI(bp_il + 1, l - 1) * get_energy_VP(l, j)
                             * pow(expPB_penalty, 2);
                    qt += V_temp;
                    if (qt >= r) {
                        case4 = true;
                        break;
                    }
                }
            }
        }
    }
    if (case4) {
        Sample_BE(i, tree.tree[i].pair, bp_il, tree.tree[bp_il].pair, structure, samples, tree);
        Sample_WI(bp_il + 1, l - 1, structure, samples, tree);
        Sample_VP(l, j, structure, samples, tree);
		return;
    } else {
        printf("backtracking failed for WMBP\n");
        exit(0);
    }
}

void W_final_pf::Sample_VP(cand_pos_t i, cand_pos_t j, std::string &structure,
                           std::unordered_map<std::pair<cand_pos_t, cand_pos_t>, cand_pos_t, SzudzikHash> &samples, sparse_tree &tree) {
    ensure_pair_matrix_initialized();
    if (debug) printf("VP at %d and %d\n", i, j);
    cand_pos_t k, l;
    pf_t qt = 0;
    structure[i - 1] = '[';
    structure[j - 1] = ']';

    pf_t V_temp = 0;
    pf_t r = vrna_urn() * get_energy_VP(i, j);

    std::pair<cand_pos_tu, cand_pos_tu> base_pair(i, j);
    std::pair<cand_pos_tu, cand_pos_tu> base_pair_reversed(j, i);
    ++samples[base_pair]; // Increments the base pair found in VP
    ++samples[base_pair_reversed];

    cand_pos_t Bp_ij = tree.Bp(i, j);
    cand_pos_t B_ij = tree.B(i, j);
    cand_pos_t b_ij = tree.b(i, j);
    cand_pos_t bp_ij = tree.bp(i, j);

    if ((tree.tree[i].parent->index) > 0 && (tree.tree[j].parent->index) < (tree.tree[i].parent->index) && Bp_ij >= 0 && B_ij >= 0 && bp_ij < 0) {
        V_temp = (get_energy_WI(i + 1, Bp_ij - 1) * get_energy_WI(B_ij + 1, j - 1));
        V_temp *= scale[2];
        qt += V_temp;
        if (qt >= r) {
            Sample_WI(i + 1, Bp_ij - 1, structure, samples, tree);
            Sample_WI(B_ij + 1, j - 1, structure, samples, tree);
            return;
        }
    }
    if ((tree.tree[i].parent->index) < (tree.tree[j].parent->index) && (tree.tree[j].parent->index) > 0 && b_ij >= 0 && bp_ij >= 0 && Bp_ij < 0) {
        V_temp = (get_energy_WI(i + 1, b_ij - 1) * get_energy_WI(bp_ij + 1, j - 1));
        V_temp *= scale[2];
        qt += V_temp;
        if (qt >= r) {
            Sample_WI(i + 1, b_ij - 1, structure, samples, tree);
            Sample_WI(bp_ij + 1, j - 1, structure, samples, tree);
            return;
        }
    }
    if ((tree.tree[i].parent->index) > 0 && (tree.tree[j].parent->index) > 0 && Bp_ij >= 0 && B_ij >= 0 && b_ij >= 0 && bp_ij >= 0) {
        V_temp = (get_energy_WI(i + 1, Bp_ij - 1) * get_energy_WI(B_ij + 1, b_ij - 1) * get_energy_WI(bp_ij + 1, j - 1));
        V_temp *= scale[2];
        qt += V_temp;
        if (qt >= r) {
            Sample_WI(i + 1, Bp_ij - 1, structure, samples, tree);
            Sample_WI(B_ij + 1, b_ij - 1, structure, samples, tree);
            Sample_WI(bp_ij + 1, j - 1, structure, samples, tree);
            return;
        }
    }
    pair_type ptype_closingip1jm1 = pair[S_[i + 1]][S_[j - 1]];
    if ((tree.tree[i + 1].pair) < -1 && (tree.tree[j - 1].pair) < -1 && ptype_closingip1jm1 > 0) {
        V_temp = (get_e_stP(i, j) * get_energy_VP(i + 1, j - 1));
        V_temp *= scale[2];
        qt += V_temp;
        if (qt >= r) {
            Sample_VP(i + 1, j - 1, structure, samples, tree);
            return;
        }
    }

    cand_pos_t min_borders = std::min((cand_pos_tu)Bp_ij, (cand_pos_tu)b_ij);
    cand_pos_t edge_i = std::min(i + MAXLOOP + 1, j - TURN - 1);
    min_borders = std::min(min_borders, edge_i);
    for (k = i + 1; k < min_borders; ++k) {
        if (tree.tree[k].pair < -1 && (tree.up[(k)-1] >= ((k) - (i)-1))) {
            cand_pos_t max_borders = std::max(bp_ij, B_ij) + 1;
            cand_pos_t edge_j = k + j - i - MAXLOOP - 2;
            max_borders = std::max(max_borders, edge_j);
            for (l = j - 1; l > max_borders; --l) {
                pair_type ptype_closingkj = pair[S_[k]][S_[l]];
                if (k == i + 1 && l == j - 1) continue; // I have to add or else it will add a stP version and an eintP version to the sum
                if (tree.tree[l].pair < -1 && ptype_closingkj > 0 && (tree.up[(j)-1] >= ((j) - (l)-1))) {
                    cand_pos_t u1 = k - i - 1;
                    cand_pos_t u2 = j - l - 1;
                    V_temp = (get_e_intP(i, k, l, j) * get_energy_VP(k, l));
                    V_temp *= scale[u1 + u2 + 2];
                    qt += V_temp;
                    if (qt >= r) {
                        break;
                    }
                }
            }
            if (qt >= r) {
                break;
            }
        }
    }
    if (k < min_borders) {
        Sample_VP(k, l, structure, samples, tree);
        return;
    }

    cand_pos_t min_Bp_j = std::min((cand_pos_tu)tree.b(i, j), (cand_pos_tu)tree.Bp(i, j));
    cand_pos_t max_i_bp = std::max(tree.B(i, j), tree.bp(i, j));
    for (k = i + 1; k < min_Bp_j; ++k) {
        V_temp = (get_energy_WIP(i + 1, k - 1) * get_energy_VP(k, j - 1) * expap_penalty * pow(expbp_penalty, 2));
        V_temp *= scale[2];
        qt += V_temp;
        if (qt > r) {
            break;
        }
    }
    if (k < min_Bp_j) {
        Sample_WIP(i + 1, k - 1, structure, samples, tree);
        Sample_VP(k, j - 1, structure, samples, tree);
		return;
    }

    for (k = max_i_bp + 1; k < j; ++k) {
        V_temp = (get_energy_VP(i + 1, k) * get_energy_WIP(k + 1, j - 1) * expap_penalty * pow(expbp_penalty, 2));
        V_temp *= scale[2];
        qt += V_temp;
        if (qt > r) {
            break;
        }
    }
    if (k < j) {
        Sample_VP(i + 1, k, structure, samples, tree);
        Sample_WIP(k + 1, j - 1, structure, samples, tree);
        return;
    }

    for (k = i + 1; k < min_Bp_j; ++k) {
        V_temp = (get_energy_WIP(i + 1, k - 1) * get_energy_VPR(k, j - 1) * expap_penalty * pow(expbp_penalty, 2));
        V_temp *= scale[2];
        qt += V_temp;
        if (qt > r) {
            break;
        }
    }
    if (k < min_Bp_j) {
        Sample_WIP(i + 1, k - 1, structure, samples, tree);
        Sample_VPR(k, j - 1, structure, samples, tree);
        return;
    }

    for (k = max_i_bp + 1; k < j; ++k) {
        V_temp = (get_energy_VPL(i + 1, k) * get_energy_WIP(k + 1, j - 1) * expap_penalty * pow(expbp_penalty, 2));
        V_temp *= scale[2];
        qt += V_temp;
        if (qt > r) {
            break;
        }
    }
    if (k < j) {
        Sample_VPL(i + 1, k, structure, samples, tree);
        Sample_WIP(k + 1, j - 1, structure, samples, tree);
        return;
    }
}

void W_final_pf::Sample_VPL(cand_pos_t i, cand_pos_t j, std::string &structure,
                            std::unordered_map<std::pair<cand_pos_t, cand_pos_t>, cand_pos_t, SzudzikHash> &samples, sparse_tree &tree) {
    if (debug) printf("VPL at %d and %d\n", i, j);
    cand_pos_t k;
    pf_t fbd = 0;
    pf_t qt = 0;

    pf_t V_temp = 0;
    pf_t r = vrna_urn() * (get_energy_VPL(i, j) - fbd);
    cand_pos_t min_Bp_j = std::min((cand_pos_tu)tree.b(i, j), (cand_pos_tu)tree.Bp(i, j));
    for (k = i + 1; k < min_Bp_j; ++k) {
        bool can_pair = scfg::can_pair_left_span(tree, i, k);
        if (can_pair) {
            V_temp = (expcp_pen[k - i] * get_energy_VP(k, j));
            qt += V_temp;
            if (qt >= r) {
                break;
            }
        }
    }
    if (k < min_Bp_j) {
        Sample_VP(k, j, structure, samples, tree);
    } else {
        printf("Backtracking error in VPL\n");
        exit(0);
    }
}

void W_final_pf::Sample_VPR(cand_pos_t i, cand_pos_t j, std::string &structure,
                            std::unordered_map<std::pair<cand_pos_t, cand_pos_t>, cand_pos_t, SzudzikHash> &samples, sparse_tree &tree) {
    if (debug) printf("VPR at %d and %d\n", i, j);
    cand_pos_t k;
    pf_t fbd = 0;
    pf_t qt = 0;
    bool unpaired = false;

    pf_t V_temp = 0;
    pf_t r = vrna_urn() * (get_energy_VPR(i, j) - fbd);

    cand_pos_t max_i_bp = std::max(tree.B(i, j), tree.bp(i, j));
    for (k = max_i_bp + 1; k < j; ++k) {
        bool can_pair = scfg::can_pair_right_span(tree, k, j);
        V_temp = (get_energy_VP(i, k) * get_energy_WIP(k + 1, j));
        qt += V_temp;
        if (qt >= r) {
            break;
        }
        if (can_pair) {
            V_temp = (get_energy_VP(i, k) * expcp_pen[k - i]);
            qt += V_temp;
            if (qt >= r) {
                unpaired = true;
                break;
            }
        }
    }
    if (k == j) {
        printf("Backtracking error in VPR\n");
        exit(0);
    }

    if (!unpaired) {
        Sample_WIP(k + 1, j, structure, samples, tree);
    }
    Sample_VP(i, k, structure, samples, tree);
}

void W_final_pf::Sample_BE(cand_pos_t i, cand_pos_t j, cand_pos_t ip, cand_pos_t jp, std::string &structure,
                           std::unordered_map<std::pair<cand_pos_t, cand_pos_t>, cand_pos_t, SzudzikHash> &samples, sparse_tree &tree) {
    
	if (debug) printf("BE at %d and %d, and %d and %d\n", i, j, ip, jp);

    if (!(i >= 1 && i <= ip && ip < jp && jp <= j && j <= n && tree.tree[i].pair > 0 && tree.tree[j].pair > 0 && tree.tree[ip].pair > 0
          && tree.tree[jp].pair > 0)) { // impossible cases
        printf("Backtracking failed in BE: impossible case -- %d and %d, and %d and %d\n", i, j, ip, jp);
        exit(0);
    }

    if (tree.tree[i].pair != j || tree.tree[ip].pair != jp) {
        printf("Backtracking failed in BE: base case: i.j and ip.jp must be in G\n");
        exit(0);
    }

	cand_pos_t l = j;
    cand_pos_t lp = j;
    pf_t qt = 0;
    bool unpaired_left = false;
    bool unpaired_right = false;
    structure[i - 1] = '(';
    structure[j - 1] = ')';
    std::pair<cand_pos_tu, cand_pos_tu> base_pair(i, j);
    std::pair<cand_pos_tu, cand_pos_tu> base_pair_reversed(j, i);
    ++samples[base_pair]; // Increments the base pair found in BE
    ++samples[base_pair_reversed];

    pf_t V_temp = 0;
    pf_t r = vrna_urn() * get_BE(i, j, ip, jp, tree);

    if (i == ip && j == jp && i < j) {
        return;
    }

    if (tree.tree[i + 1].pair == j - 1) {
        Sample_BE(i + 1, j - 1, ip, jp, structure, samples, tree);
        return;
    }
    pf_t expbp2 = pow(expbp_penalty, 2);
    for (l = i + 1; l <= ip; l++) {
        if (tree.tree[l].pair >= -1 && jp <= tree.tree[l].pair && tree.tree[l].pair < j) {
            lp = tree.tree[l].pair;

            bool empty_region_il = (tree.up[(l)-1] >= l - i - 1);       // empty between i+1 and l-1
            bool empty_region_lpj = (tree.up[(j)-1] >= j - lp - 1);     // empty between lp+1 and j-1
            bool weakly_closed_il = tree.weakly_closed(i + 1, l - 1);   // weakly closed between i+1 and l-1
            bool weakly_closed_lpj = tree.weakly_closed(lp + 1, j - 1); // weakly closed between lp+1 and j-1
            if (empty_region_il && empty_region_lpj) {
                cand_pos_t u1 = l - i - 1;
                cand_pos_t u2 = j - lp - 1;
                V_temp = get_e_intP(i, l, lp, j) * get_BE(l, lp, ip, jp, tree);
                V_temp *= scale[u1 + u2 + 2];
                qt += V_temp; // Added to e_intP that l != i+1 and lp != j-1 at the same time
                if (qt >= r) {
                    unpaired_left = true;
                    unpaired_right = true;
                    break;
                }
            }
            if (weakly_closed_il && weakly_closed_lpj) {
                V_temp = get_energy_WIP(i + 1, l - 1) * get_BE(l, lp, ip, jp, tree) * get_energy_WIP(lp + 1, j - 1) * expap_penalty * expbp2;
                V_temp *= scale[2];
                qt += V_temp;
                if (qt >= r) {
                    break;
                }
            }
            if (weakly_closed_il && empty_region_lpj) {
                V_temp = get_energy_WIP(i + 1, l - 1) * get_BE(l, lp, ip, jp, tree) * expcp_pen[j - lp - 1] * expap_penalty * expbp2;
                V_temp *= scale[2];
                qt += V_temp;
                if (qt >= r) {
                    unpaired_right = true;
                    break;
                }
            }
            if (empty_region_il && weakly_closed_lpj) {
                V_temp = expcp_pen[l - i - 1] * get_BE(l, lp, ip, jp, tree) * get_energy_WIP(lp + 1, j - 1) * expap_penalty * expbp2;
                V_temp *= scale[2];
                qt += V_temp;
                if (qt >= r) {
                    unpaired_left = true;
                    break;
                }
            }
        }
    }
    if(qt<r){
        printf("Error in BE, qt=%f < r=%f with i=%d and j=%d and ip=%d and jp is %d\n",qt,r,i,j,ip,jp);
        exit(0);
    }

    if (!unpaired_left) {
        Sample_WIP(i + 1, l - 1, structure, samples, tree);
    }
    if (!unpaired_right) {
        Sample_WIP(lp + 1, j - 1, structure, samples, tree);
    }
    Sample_BE(l, lp, ip, jp, structure, samples, tree);
}
