#include "scfg/part_func_adapter.hh"

#include "part_func.hh"
#include "scfg/rules_part_func.hh"

extern double expPPS_penalty;
extern double expPSP_penalty;
extern double expPS_penalty;
extern double expPSM_penalty;
extern double expb_penalty;

namespace scfg {

struct PartFuncAdapterAccess {
    static cand_pos_t n(const W_final_pf &owner) { return owner.n; }
    static const std::vector<pf_t> &scale(const W_final_pf &owner) { return owner.scale; }
    static const std::vector<pf_t> &expMLbase(const W_final_pf &owner) { return owner.expMLbase; }
    static const std::vector<pf_t> &expPUP_pen(const W_final_pf &owner) { return owner.expPUP_pen; }
    static const std::vector<cand_pos_t> &index(const W_final_pf &owner) { return owner.index; }
    static std::vector<pf_t> &V(W_final_pf &owner) { return owner.V; }
    static std::vector<pf_t> &VM(W_final_pf &owner) { return owner.VM; }
    static std::vector<pf_t> &WI(W_final_pf &owner) { return owner.WI; }
    static std::vector<pf_t> &W(W_final_pf &owner) { return owner.W; }
    static std::vector<pf_t> &WMv(W_final_pf &owner) { return owner.WMv; }
    static std::vector<pf_t> &WMp(W_final_pf &owner) { return owner.WMp; }
    static std::vector<pf_t> &WM(W_final_pf &owner) { return owner.WM; }
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

} // namespace scfg
