#include "scfg/part_func_adapter.hh"

#include "part_func.hh"
#include "scfg/rules_part_func.hh"

extern double expPPS_penalty;
extern double expPSP_penalty;
extern double expPS_penalty;
extern double expPSM_penalty;
extern double expb_penalty;
extern double expap_penalty;
extern double expbp_penalty;
extern double expPB_penalty;

namespace scfg {

struct PartFuncAdapterAccess {
    static cand_pos_t n(const W_final_pf &owner) { return owner.n; }
    static const std::vector<pf_t> &scale(const W_final_pf &owner) { return owner.scale; }
    static const std::vector<pf_t> &expMLbase(const W_final_pf &owner) { return owner.expMLbase; }
    static const std::vector<pf_t> &expPUP_pen(const W_final_pf &owner) { return owner.expPUP_pen; }
    static const std::vector<pf_t> &expcp_pen(const W_final_pf &owner) { return owner.expcp_pen; }
    static const std::vector<cand_pos_t> &index(const W_final_pf &owner) { return owner.index; }
    static std::vector<pf_t> &V(W_final_pf &owner) { return owner.V; }
    static std::vector<pf_t> &VM(W_final_pf &owner) { return owner.VM; }
    static std::vector<pf_t> &WI(W_final_pf &owner) { return owner.WI; }
    static std::vector<pf_t> &W(W_final_pf &owner) { return owner.W; }
    static std::vector<pf_t> &WMv(W_final_pf &owner) { return owner.WMv; }
    static std::vector<pf_t> &WMp(W_final_pf &owner) { return owner.WMp; }
    static std::vector<pf_t> &WM(W_final_pf &owner) { return owner.WM; }
    static std::vector<pf_t> &WIP(W_final_pf &owner) { return owner.WIP; }
    static std::vector<pf_t> &VP(W_final_pf &owner) { return owner.VP; }
    static std::vector<pf_t> &VPL(W_final_pf &owner) { return owner.VPL; }
    static std::vector<pf_t> &VPR(W_final_pf &owner) { return owner.VPR; }
    static std::vector<pf_t> &WMBW(W_final_pf &owner) { return owner.WMBW; }
    static std::vector<pf_t> &WMBP(W_final_pf &owner) { return owner.WMBP; }
    static std::vector<pf_t> &WMB(W_final_pf &owner) { return owner.WMB; }
    static std::vector<pf_t> &BE(W_final_pf &owner) { return owner.BE; }
    static short *S(W_final_pf &owner) { return owner.S_; }
    static pf_t exp_Extloop(W_final_pf &owner, cand_pos_t i, cand_pos_t j) { return owner.exp_Extloop(i, j); }
    static pf_t exp_MLstem(W_final_pf &owner, cand_pos_t i, cand_pos_t j) { return owner.exp_MLstem(i, j); }
    static pf_t exp_Mbloop(W_final_pf &owner, cand_pos_t i, cand_pos_t j) { return owner.exp_Mbloop(i, j); }
    static pf_t HairpinE(W_final_pf &owner, cand_pos_t i, cand_pos_t j) { return owner.HairpinE(i, j); }
    static pf_t compute_internal_restricted(W_final_pf &owner, cand_pos_t i, cand_pos_t j, std::vector<int> &up) {
        return owner.compute_internal_restricted(i, j, up);
    }
    static pf_t compute_energy_VM_restricted(W_final_pf &owner, cand_pos_t i, cand_pos_t j, std::vector<int> &up) {
        return owner.compute_energy_VM_restricted(i, j, up);
    }
    static pf_t get_e_stP(W_final_pf &owner, cand_pos_t i, cand_pos_t j) {
        return owner.get_e_stP(i, j);
    }
    static pf_t get_e_intP(W_final_pf &owner, cand_pos_t i, cand_pos_t k, cand_pos_t l, cand_pos_t j) {
        return owner.get_e_intP(i, k, l, j);
    }
    static int compute_exterior_cases(W_final_pf &owner, cand_pos_t l, cand_pos_t j, sparse_tree &tree) {
        return owner.compute_exterior_cases(l, j, tree);
    }
};

namespace {

class LocalWContext final : public PartFuncWContext {
  public:
    explicit LocalWContext(W_final_pf &owner) : owner_(owner) {}

    cand_pos_t n() const override { return PartFuncAdapterAccess::n(owner_); }
    pf_t scale1() const override { return PartFuncAdapterAccess::scale(owner_)[1]; }
    pf_t get_W(cand_pos_t j) const override { return PartFuncAdapterAccess::W(owner_)[j]; }
    pf_t get_energy(cand_pos_t i, cand_pos_t j) override { return owner_.get_energy(i, j); }
    pf_t get_energy_WMB(cand_pos_t i, cand_pos_t j) override { return owner_.get_energy_WMB(i, j); }
    pf_t exp_Extloop(cand_pos_t i, cand_pos_t j) override { return PartFuncAdapterAccess::exp_Extloop(owner_, i, j); }
    pf_t expPS_penalty() const override { return ::expPS_penalty; }
    void set_W(cand_pos_t j, pf_t value) override { PartFuncAdapterAccess::W(owner_)[j] = value; }
    cand_pos_t turn() const override { return TURN; }

  private:
    W_final_pf &owner_;
};

class LocalVMContext final : public PartFuncVMContext {
  public:
    explicit LocalVMContext(W_final_pf &owner) : owner_(owner) {}

    cand_pos_t index_of(cand_pos_t i, cand_pos_t j) const override {
        return PartFuncAdapterAccess::index(owner_)[i] + j - i;
    }
    void set_VM(cand_pos_t ij, pf_t value) override { PartFuncAdapterAccess::VM(owner_)[ij] = value; }
    pf_t get_energy_WM(cand_pos_t i, cand_pos_t j) override { return owner_.get_energy_WM(i, j); }
    pf_t get_energy_WMv(cand_pos_t i, cand_pos_t j) override { return owner_.get_energy_WMv(i, j); }
    pf_t get_energy_WMp(cand_pos_t i, cand_pos_t j) override { return owner_.get_energy_WMp(i, j); }
    pf_t exp_Mbloop(cand_pos_t i, cand_pos_t j) override { return PartFuncAdapterAccess::exp_Mbloop(owner_, i, j); }
    pf_t expMLclosing() const override { return owner_.exp_params_->expMLclosing; }
    pf_t expMLbase(cand_pos_t length) const override { return PartFuncAdapterAccess::expMLbase(owner_)[length]; }
    pf_t scale2() const override { return PartFuncAdapterAccess::scale(owner_)[2]; }
    cand_pos_t turn() const override { return TURN; }

  private:
    W_final_pf &owner_;
};

class LocalVContext final : public PartFuncVContext {
  public:
    explicit LocalVContext(W_final_pf &owner) : owner_(owner) {}

    cand_pos_t index_of(cand_pos_t i, cand_pos_t j) const override {
        return PartFuncAdapterAccess::index(owner_)[i] + j - i;
    }
    void set_V(cand_pos_t ij, pf_t value) override { PartFuncAdapterAccess::V(owner_)[ij] = value; }
    pf_t hairpin_energy(cand_pos_t i, cand_pos_t j) override { return PartFuncAdapterAccess::HairpinE(owner_, i, j); }
    pf_t internal_energy(cand_pos_t i, cand_pos_t j, std::vector<int> &up) override {
        return PartFuncAdapterAccess::compute_internal_restricted(owner_, i, j, up);
    }
    pf_t vm_energy(cand_pos_t i, cand_pos_t j, std::vector<int> &up) override {
        return PartFuncAdapterAccess::compute_energy_VM_restricted(owner_, i, j, up);
    }

  private:
    W_final_pf &owner_;
};

class LocalWIContext final : public PartFuncWIContext {
  public:
    explicit LocalWIContext(W_final_pf &owner) : owner_(owner) {}

    cand_pos_t index_of(cand_pos_t i, cand_pos_t j) const override {
        return PartFuncAdapterAccess::index(owner_)[i] + j - i;
    }
    void set_WI(cand_pos_t ij, pf_t value) override { PartFuncAdapterAccess::WI(owner_)[ij] = value; }
    pf_t get_WI(cand_pos_t i, cand_pos_t j) override { return owner_.get_energy_WI(i, j); }
    pf_t get_energy(cand_pos_t i, cand_pos_t j) override { return owner_.get_energy(i, j); }
    pf_t get_energy_WMB(cand_pos_t i, cand_pos_t j) override { return owner_.get_energy_WMB(i, j); }
    pf_t expPPS_penalty() const override { return ::expPPS_penalty; }
    pf_t expPSP_penalty() const override { return ::expPSP_penalty; }
    pf_t expPUP_pen1() const override { return PartFuncAdapterAccess::expPUP_pen(owner_)[1]; }
    cand_pos_t turn() const override { return TURN; }

  private:
    W_final_pf &owner_;
};

class LocalWMvWMpContext final : public PartFuncWMvWMpContext {
  public:
    explicit LocalWMvWMpContext(W_final_pf &owner) : owner_(owner) {}

    cand_pos_t index_of(cand_pos_t i, cand_pos_t j) const override {
        return PartFuncAdapterAccess::index(owner_)[i] + j - i;
    }
    pf_t get_energy(cand_pos_t i, cand_pos_t j) override { return owner_.get_energy(i, j); }
    pf_t get_energy_WMB(cand_pos_t i, cand_pos_t j) override { return owner_.get_energy_WMB(i, j); }
    pf_t get_energy_WMv(cand_pos_t i, cand_pos_t j) override { return owner_.get_energy_WMv(i, j); }
    pf_t get_energy_WMp(cand_pos_t i, cand_pos_t j) override { return owner_.get_energy_WMp(i, j); }
    pf_t exp_MLstem(cand_pos_t i, cand_pos_t j) override { return PartFuncAdapterAccess::exp_MLstem(owner_, i, j); }
    pf_t expPSM_penalty() const override { return ::expPSM_penalty; }
    pf_t expb_penalty() const override { return ::expb_penalty; }
    pf_t expMLbase1() const override { return PartFuncAdapterAccess::expMLbase(owner_)[1]; }
    void set_WMv_WMp(cand_pos_t ij, pf_t wmv, pf_t wmp) override {
        PartFuncAdapterAccess::WMv(owner_)[ij] = wmv;
        PartFuncAdapterAccess::WMp(owner_)[ij] = wmp;
    }

  private:
    W_final_pf &owner_;
};

class LocalWMContext final : public PartFuncWMContext {
  public:
    explicit LocalWMContext(W_final_pf &owner) : owner_(owner) {}

    cand_pos_t index_of(cand_pos_t i, cand_pos_t j) const override {
        return PartFuncAdapterAccess::index(owner_)[i] + j - i;
    }
    pf_t get_energy_WM(cand_pos_t i, cand_pos_t j) override { return owner_.get_energy_WM(i, j); }
    pf_t get_energy_WMv(cand_pos_t i, cand_pos_t j) override { return owner_.get_energy_WMv(i, j); }
    pf_t get_energy_WMp(cand_pos_t i, cand_pos_t j) override { return owner_.get_energy_WMp(i, j); }
    pf_t get_energy(cand_pos_t i, cand_pos_t j) override { return owner_.get_energy(i, j); }
    pf_t get_energy_WMB(cand_pos_t i, cand_pos_t j) override { return owner_.get_energy_WMB(i, j); }
    pf_t exp_MLstem(cand_pos_t i, cand_pos_t j) override { return PartFuncAdapterAccess::exp_MLstem(owner_, i, j); }
    pf_t expPSM_penalty() const override { return ::expPSM_penalty; }
    pf_t expb_penalty() const override { return ::expb_penalty; }
    pf_t exp_Mbloop(cand_pos_t i, cand_pos_t j) override { return PartFuncAdapterAccess::exp_Mbloop(owner_, i, j); }
    pf_t expMLclosing() const override { return owner_.exp_params_->expMLclosing; }
    pf_t expMLbase(cand_pos_t length) const override { return PartFuncAdapterAccess::expMLbase(owner_)[length]; }
    cand_pos_t turn() const override { return TURN; }
    void set_WM(cand_pos_t ij, pf_t value) override { PartFuncAdapterAccess::WM(owner_)[ij] = value; }

  private:
    W_final_pf &owner_;
};

class LocalWIPContext final : public PartFuncWIPContext {
  public:
    explicit LocalWIPContext(W_final_pf &owner) : owner_(owner) {}

    cand_pos_t index_of(cand_pos_t i, cand_pos_t j) const override {
        return PartFuncAdapterAccess::index(owner_)[i] + j - i;
    }
    pf_t get_energy(cand_pos_t i, cand_pos_t j) override { return owner_.get_energy(i, j); }
    pf_t get_energy_WMB(cand_pos_t i, cand_pos_t j) override { return owner_.get_energy_WMB(i, j); }
    pf_t get_energy_WIP(cand_pos_t i, cand_pos_t j) override { return owner_.get_energy_WIP(i, j); }
    pf_t expbp_penalty() const override { return ::expbp_penalty; }
    pf_t expPSM_penalty() const override { return ::expPSM_penalty; }
    pf_t expcp_pen(cand_pos_t length) const override { return PartFuncAdapterAccess::expcp_pen(owner_)[length]; }
    void set_WIP(cand_pos_t ij, pf_t value) override { PartFuncAdapterAccess::WIP(owner_)[ij] = value; }
    cand_pos_t turn() const override { return TURN; }

  private:
    W_final_pf &owner_;
};

class LocalVPLContext final : public PartFuncVPLContext {
  public:
    explicit LocalVPLContext(W_final_pf &owner) : owner_(owner) {}

    cand_pos_t index_of(cand_pos_t i, cand_pos_t j) const override {
        return PartFuncAdapterAccess::index(owner_)[i] + j - i;
    }
    pf_t get_energy_VP(cand_pos_t i, cand_pos_t j) override { return owner_.get_energy_VP(i, j); }
    pf_t expcp_pen(cand_pos_t length) const override { return PartFuncAdapterAccess::expcp_pen(owner_)[length]; }
    void set_VPL(cand_pos_t ij, pf_t value) override { PartFuncAdapterAccess::VPL(owner_)[ij] = value; }

  private:
    W_final_pf &owner_;
};

class LocalVPRContext final : public PartFuncVPRContext {
  public:
    explicit LocalVPRContext(W_final_pf &owner) : owner_(owner) {}

    cand_pos_t index_of(cand_pos_t i, cand_pos_t j) const override {
        return PartFuncAdapterAccess::index(owner_)[i] + j - i;
    }
    pf_t get_energy_VP(cand_pos_t i, cand_pos_t j) override { return owner_.get_energy_VP(i, j); }
    pf_t get_energy_WIP(cand_pos_t i, cand_pos_t j) override { return owner_.get_energy_WIP(i, j); }
    pf_t expcp_pen(cand_pos_t length) const override { return PartFuncAdapterAccess::expcp_pen(owner_)[length]; }
    void set_VPR(cand_pos_t ij, pf_t value) override { PartFuncAdapterAccess::VPR(owner_)[ij] = value; }

  private:
    W_final_pf &owner_;
};

class LocalVPContext final : public PartFuncVPContext {
  public:
    explicit LocalVPContext(W_final_pf &owner) : owner_(owner) {}

    cand_pos_t index_of(cand_pos_t i, cand_pos_t j) const override {
        return PartFuncAdapterAccess::index(owner_)[i] + j - i;
    }
    pf_t get_energy_WI(cand_pos_t i, cand_pos_t j) override { return owner_.get_energy_WI(i, j); }
    pf_t get_energy_VP(cand_pos_t i, cand_pos_t j) override { return owner_.get_energy_VP(i, j); }
    pf_t get_energy_WIP(cand_pos_t i, cand_pos_t j) override { return owner_.get_energy_WIP(i, j); }
    pf_t get_energy_VPL(cand_pos_t i, cand_pos_t j) override { return owner_.get_energy_VPL(i, j); }
    pf_t get_energy_VPR(cand_pos_t i, cand_pos_t j) override { return owner_.get_energy_VPR(i, j); }
    pair_type pair_type_of(cand_pos_t i, cand_pos_t j) const override {
        return pair[PartFuncAdapterAccess::S(owner_)[i]][PartFuncAdapterAccess::S(owner_)[j]];
    }
    pf_t get_e_stP(cand_pos_t i, cand_pos_t j) override { return PartFuncAdapterAccess::get_e_stP(owner_, i, j); }
    pf_t get_e_intP(cand_pos_t i, cand_pos_t k, cand_pos_t l, cand_pos_t j) override {
        return PartFuncAdapterAccess::get_e_intP(owner_, i, k, l, j);
    }
    pf_t expap_penalty() const override { return ::expap_penalty; }
    pf_t expbp_penalty() const override { return ::expbp_penalty; }
    pf_t expcp_pen(cand_pos_t length) const override { return PartFuncAdapterAccess::expcp_pen(owner_)[length]; }
    pf_t scale(cand_pos_t length) const override { return PartFuncAdapterAccess::scale(owner_)[length]; }
    pf_t expbp_penalty_sq() const override { return ::expbp_penalty * ::expbp_penalty; }
    void set_VP(cand_pos_t ij, pf_t value) override { PartFuncAdapterAccess::VP(owner_)[ij] = value; }

  private:
    W_final_pf &owner_;
};

class LocalWMBWContext final : public PartFuncWMBWContext {
  public:
    explicit LocalWMBWContext(W_final_pf &owner) : owner_(owner) {}

    cand_pos_t index_of(cand_pos_t i, cand_pos_t j) const override {
        return PartFuncAdapterAccess::index(owner_)[i] + j - i;
    }
    pf_t get_energy_WMBP(cand_pos_t i, cand_pos_t j) override { return owner_.get_energy_WMBP(i, j); }
    pf_t get_energy_WI(cand_pos_t i, cand_pos_t j) override { return owner_.get_energy_WI(i, j); }
    void set_WMBW(cand_pos_t ij, pf_t value) override { PartFuncAdapterAccess::WMBW(owner_)[ij] = value; }

  private:
    W_final_pf &owner_;
};

class LocalWMBPContext final : public PartFuncWMBPContext {
  public:
    explicit LocalWMBPContext(W_final_pf &owner) : owner_(owner) {}

    cand_pos_t index_of(cand_pos_t i, cand_pos_t j) const override {
        return PartFuncAdapterAccess::index(owner_)[i] + j - i;
    }
    pf_t get_energy_WMBP(cand_pos_t i, cand_pos_t j) override { return owner_.get_energy_WMBP(i, j); }
    pf_t get_energy_WMBW(cand_pos_t i, cand_pos_t j) override { return owner_.get_energy_WMBW(i, j); }
    pf_t get_energy_VP(cand_pos_t i, cand_pos_t j) override { return owner_.get_energy_VP(i, j); }
    pf_t get_energy_WI(cand_pos_t i, cand_pos_t j) override { return owner_.get_energy_WI(i, j); }
    pf_t get_BE(cand_pos_t i, cand_pos_t j, cand_pos_t ip, cand_pos_t jp, sparse_tree &tree) override {
        return owner_.get_BE(i, j, ip, jp, tree);
    }
    pf_t expPB_penalty() const override { return ::expPB_penalty; }
    pf_t expPB_penalty_sq() const override { return ::expPB_penalty * ::expPB_penalty; }
    cand_pos_t n() const override { return PartFuncAdapterAccess::n(owner_); }
    int compute_exterior_cases(cand_pos_t l, cand_pos_t j, sparse_tree &tree) override {
        return PartFuncAdapterAccess::compute_exterior_cases(owner_, l, j, tree);
    }
    void set_WMBP(cand_pos_t ij, pf_t value) override { PartFuncAdapterAccess::WMBP(owner_)[ij] = value; }

  private:
    W_final_pf &owner_;
};

class LocalWMBContext final : public PartFuncWMBContext {
  public:
    explicit LocalWMBContext(W_final_pf &owner) : owner_(owner) {}

    cand_pos_t index_of(cand_pos_t i, cand_pos_t j) const override {
        return PartFuncAdapterAccess::index(owner_)[i] + j - i;
    }
    pf_t get_energy_WMBP(cand_pos_t i, cand_pos_t j) override { return owner_.get_energy_WMBP(i, j); }
    pf_t get_energy_WI(cand_pos_t i, cand_pos_t j) override { return owner_.get_energy_WI(i, j); }
    pf_t get_BE(cand_pos_t i, cand_pos_t j, cand_pos_t ip, cand_pos_t jp, sparse_tree &tree) override {
        return owner_.get_BE(i, j, ip, jp, tree);
    }
    pf_t expPB_penalty() const override { return ::expPB_penalty; }
    cand_pos_t n() const override { return PartFuncAdapterAccess::n(owner_); }
    void set_WMB(cand_pos_t ij, pf_t value) override { PartFuncAdapterAccess::WMB(owner_)[ij] = value; }

  private:
    W_final_pf &owner_;
};

class LocalBEContext final : public PartFuncBEContext {
  public:
    explicit LocalBEContext(W_final_pf &owner) : owner_(owner) {}

    cand_pos_t index_of(cand_pos_t i, cand_pos_t j) const override {
        return PartFuncAdapterAccess::index(owner_)[i] + j - i;
    }
    cand_pos_t n() const override { return PartFuncAdapterAccess::n(owner_); }
    pf_t get_energy_WIP(cand_pos_t i, cand_pos_t j) override { return owner_.get_energy_WIP(i, j); }
    pf_t get_BE(cand_pos_t i, cand_pos_t j, cand_pos_t ip, cand_pos_t jp, sparse_tree &tree) override {
        return owner_.get_BE(i, j, ip, jp, tree);
    }
    pf_t get_e_stP(cand_pos_t i, cand_pos_t j) override { return PartFuncAdapterAccess::get_e_stP(owner_, i, j); }
    pf_t get_e_intP(cand_pos_t i, cand_pos_t k, cand_pos_t l, cand_pos_t j) override {
        return PartFuncAdapterAccess::get_e_intP(owner_, i, k, l, j);
    }
    pf_t expap_penalty() const override { return ::expap_penalty; }
    pf_t expbp_penalty_sq() const override { return ::expbp_penalty * ::expbp_penalty; }
    pf_t expcp_pen(cand_pos_t length) const override { return PartFuncAdapterAccess::expcp_pen(owner_)[length]; }
    pf_t scale(cand_pos_t length) const override { return PartFuncAdapterAccess::scale(owner_)[length]; }
    void set_BE(cand_pos_t ij, pf_t value) override { PartFuncAdapterAccess::BE(owner_)[ij] = value; }

  private:
    W_final_pf &owner_;
};

} // namespace

void compute_W_restricted(W_final_pf &owner, sparse_tree &tree) {
    LocalWContext ctx(owner);
    compute_W_restricted(ctx, tree);
}

pf_t compute_VM_restricted(W_final_pf &owner, cand_pos_t i, cand_pos_t j, std::vector<int> &up) {
    LocalVMContext ctx(owner);
    return compute_VM_restricted(ctx, i, j, up);
}

void compute_V_restricted(W_final_pf &owner, cand_pos_t i, cand_pos_t j, sparse_tree &tree) {
    LocalVContext ctx(owner);
    compute_V_restricted(ctx, i, j, tree);
}

void compute_WI_restricted(W_final_pf &owner, cand_pos_t i, cand_pos_t j, sparse_tree &tree) {
    LocalWIContext ctx(owner);
    compute_WI_restricted(ctx, i, j, tree);
}

void compute_WMv_WMp_restricted(W_final_pf &owner, cand_pos_t i, cand_pos_t j, std::vector<Node> &tree) {
    LocalWMvWMpContext ctx(owner);
    compute_WMv_WMp_restricted(ctx, i, j, tree);
}

void compute_WM_restricted(W_final_pf &owner, cand_pos_t i, cand_pos_t j, sparse_tree &tree) {
    LocalWMContext ctx(owner);
    compute_WM_restricted(ctx, i, j, tree);
}

void compute_WIP_restricted(W_final_pf &owner, cand_pos_t i, cand_pos_t j, sparse_tree &tree) {
    LocalWIPContext ctx(owner);
    compute_WIP_restricted(ctx, i, j, tree);
}

void compute_VPL_restricted(W_final_pf &owner, cand_pos_t i, cand_pos_t j, sparse_tree &tree) {
    LocalVPLContext ctx(owner);
    compute_VPL_restricted(ctx, i, j, tree);
}

void compute_VPR_restricted(W_final_pf &owner, cand_pos_t i, cand_pos_t j, sparse_tree &tree) {
    LocalVPRContext ctx(owner);
    compute_VPR_restricted(ctx, i, j, tree);
}

void compute_VP_restricted(W_final_pf &owner, cand_pos_t i, cand_pos_t j, sparse_tree &tree) {
    LocalVPContext ctx(owner);
    compute_VP_restricted(ctx, i, j, tree);
}

void compute_WMBW_restricted(W_final_pf &owner, cand_pos_t i, cand_pos_t j, sparse_tree &tree) {
    LocalWMBWContext ctx(owner);
    compute_WMBW_restricted(ctx, i, j, tree);
}

void compute_WMBP_restricted(W_final_pf &owner, cand_pos_t i, cand_pos_t j, sparse_tree &tree) {
    LocalWMBPContext ctx(owner);
    compute_WMBP_restricted(ctx, i, j, tree);
}

void compute_WMB_restricted(W_final_pf &owner, cand_pos_t i, cand_pos_t j, sparse_tree &tree) {
    LocalWMBContext ctx(owner);
    compute_WMB_restricted(ctx, i, j, tree);
}

void compute_BE_restricted(W_final_pf &owner,
                           cand_pos_t i,
                           cand_pos_t j,
                           cand_pos_t ip,
                           cand_pos_t jp,
                           sparse_tree &tree) {
    LocalBEContext ctx(owner);
    compute_BE_restricted(ctx, i, j, ip, jp, tree);
}

} // namespace scfg
