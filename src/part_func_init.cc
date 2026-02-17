#include "part_func.hh"

#include <cstdlib>
#include <cstring>
#include <iostream>

W_final_pf::W_final_pf(std::string &seq, std::string &MFE_structure, bool pk_free,bool pk_only,bool fatgraph, int dangle, double energy, int num_samples, bool PSplot)
    : exp_params_(scale_pf_parameters()) {
    make_pair_matrix();
    this->seq = seq;
    this->MFE_structure = MFE_structure;
    this->n = seq.length();
    this->pk_free = pk_free;
    this->pk_only = pk_only;
    this->fatgraph = fatgraph;
    this->PSplot = PSplot;
    this->num_samples = num_samples;

    exp_params_->model_details.dangles = dangle;
    S_ = encode_sequence(seq.c_str(), 0);
    S1_ = encode_sequence(seq.c_str(), 1);
    const char *pf_debug_env = std::getenv("CPARTY_PF_DEBUG");
    static bool seq_logged = false;
    if (!seq_logged && pf_debug_env && *pf_debug_env != '\0' && std::strcmp(pf_debug_env, "0") != 0) {
        seq_logged = true;
        std::cerr << "[PF_DEBUG] encoded_sequence"
                  << " n=" << n
                  << " S:";
        for (cand_pos_t i = 1; i <= n; ++i) {
            std::cerr << " " << static_cast<int>(S_[i]);
        }
        std::cerr << " S1:";
        for (cand_pos_t i = 1; i <= n; ++i) {
            std::cerr << " " << static_cast<int>(S1_[i]);
        }
        std::cerr << std::endl;
    }

    index.resize(n + 1);
    scale.resize(n + 1);
    expMLbase.resize(n + 1);
    expcp_pen.resize(n + 1);
    expPUP_pen.resize(n + 1);
    cand_pos_t total_length = ((n + 1) * (n + 2)) / 2;
    index[1] = 0;
    for (cand_pos_t i = 2; i <= n; i++)
        index[i] = index[i - 1] + (n + 1) - i + 1;
    // Allocate space
    V.resize(total_length, 0);
    VM.resize(total_length, 0);
    WM.resize(total_length, 0);
    WMv.resize(total_length, 0);
    WMp.resize(total_length, 0);

    // PK
    WIP.resize(total_length, 0);
    VP.resize(total_length, 0);
    VPL.resize(total_length, 0);
    VPR.resize(total_length, 0);
    WMB.resize(total_length, 0);
    WMBP.resize(total_length, 0);
    WMBW.resize(total_length, 0);
    BE.resize(total_length, 0);

    rescale_pk_globals();
    exp_params_rescale(energy);
    W.resize(n + 1, scale[1]);
    WI.resize(total_length, scale[1]);

    /**     MEA       */
    // probs.resize(total_length,0);
}

W_final_pf::~W_final_pf() {}
