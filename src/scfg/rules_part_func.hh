#ifndef SCFG_RULES_PART_FUNC_HH_
#define SCFG_RULES_PART_FUNC_HH_

#include "base_types.hh"
#include "sparse_tree.hh"

namespace scfg {

struct PartFuncModeConfig {
    pf_t exp_pb_penalty;
    cand_pos_t turn;
};

class PartFuncRuleHelpers {
  public:
    PartFuncRuleHelpers(sparse_tree &tree, const PartFuncModeConfig &config);

    cand_pos_t border_b(cand_pos_t i, cand_pos_t j);
    cand_pos_t border_bp(cand_pos_t i, cand_pos_t j);
    cand_pos_t border_B(cand_pos_t i, cand_pos_t j);
    cand_pos_t border_Bp(cand_pos_t i, cand_pos_t j);
    cand_pos_t pair_at(cand_pos_t pos) const;
    cand_pos_t parent_index(cand_pos_t pos) const;

    bool allow_exterior_split(cand_pos_t l, cand_pos_t j, cand_pos_t b_ij, int exterior_case) const;
    bool has_valid_band_borders(cand_pos_t i, cand_pos_t l, cand_pos_t j);
    bool parent_within_interval_and_turn(cand_pos_t i, cand_pos_t l, cand_pos_t j) const;
    bool has_valid_inner_arc_split(cand_pos_t i, cand_pos_t l, cand_pos_t j, cand_pos_t n_bound);

    pf_t apply_double_pb_penalty(pf_t value) const;

    template <typename Fn>
    void for_each_split(cand_pos_t i, cand_pos_t j, Fn fn) const {
        for (cand_pos_t l = i + 1; l < j; ++l) fn(l);
    }

    void on_traceback_hook(cand_pos_t i, cand_pos_t j) const;
    void on_fixed_parse_hook(cand_pos_t i, cand_pos_t j) const;

  private:
    sparse_tree &tree_;
    PartFuncModeConfig config_;
};

class PartFuncVContext {
  public:
    virtual ~PartFuncVContext() = default;

    virtual cand_pos_t index_of(cand_pos_t i, cand_pos_t j) const = 0;
    virtual void set_V(cand_pos_t ij, pf_t value) = 0;
    virtual pf_t hairpin_energy(cand_pos_t i, cand_pos_t j) = 0;
    virtual pf_t internal_energy(cand_pos_t i, cand_pos_t j, std::vector<int> &up) = 0;
    virtual pf_t vm_energy(cand_pos_t i, cand_pos_t j, std::vector<int> &up) = 0;
};

void compute_V_restricted(PartFuncVContext &ctx, cand_pos_t i, cand_pos_t j, sparse_tree &tree);

class PartFuncWIContext {
  public:
    virtual ~PartFuncWIContext() = default;

    virtual cand_pos_t index_of(cand_pos_t i, cand_pos_t j) const = 0;
    virtual void set_WI(cand_pos_t ij, pf_t value) = 0;
    virtual pf_t get_WI(cand_pos_t i, cand_pos_t j) = 0;
    virtual pf_t get_energy(cand_pos_t i, cand_pos_t j) = 0;
    virtual pf_t get_energy_WMB(cand_pos_t i, cand_pos_t j) = 0;
    virtual pf_t expPPS_penalty() const = 0;
    virtual pf_t expPSP_penalty() const = 0;
    virtual pf_t expPUP_pen1() const = 0;
    virtual cand_pos_t turn() const = 0;
};

void compute_WI_restricted(PartFuncWIContext &ctx, cand_pos_t i, cand_pos_t j, sparse_tree &tree);

class PartFuncWContext {
  public:
    virtual ~PartFuncWContext() = default;

    virtual cand_pos_t n() const = 0;
    virtual pf_t scale1() const = 0;
    virtual pf_t get_W(cand_pos_t j) const = 0;
    virtual pf_t get_energy(cand_pos_t i, cand_pos_t j) = 0;
    virtual pf_t get_energy_WMB(cand_pos_t i, cand_pos_t j) = 0;
    virtual pf_t exp_Extloop(cand_pos_t i, cand_pos_t j) = 0;
    virtual pf_t expPS_penalty() const = 0;
    virtual void set_W(cand_pos_t j, pf_t value) = 0;
    virtual cand_pos_t turn() const = 0;
};

void compute_W_restricted(PartFuncWContext &ctx, sparse_tree &tree);

class PartFuncVMContext {
  public:
    virtual ~PartFuncVMContext() = default;

    virtual cand_pos_t index_of(cand_pos_t i, cand_pos_t j) const = 0;
    virtual void set_VM(cand_pos_t ij, pf_t value) = 0;
    virtual pf_t get_energy_WM(cand_pos_t i, cand_pos_t j) = 0;
    virtual pf_t get_energy_WMv(cand_pos_t i, cand_pos_t j) = 0;
    virtual pf_t get_energy_WMp(cand_pos_t i, cand_pos_t j) = 0;
    virtual pf_t exp_Mbloop(cand_pos_t i, cand_pos_t j) = 0;
    virtual pf_t expMLclosing() const = 0;
    virtual pf_t expMLbase(cand_pos_t length) const = 0;
    virtual pf_t scale2() const = 0;
    virtual cand_pos_t turn() const = 0;
};

pf_t compute_VM_restricted(PartFuncVMContext &ctx, cand_pos_t i, cand_pos_t j, std::vector<int> &up);

class PartFuncWMvWMpContext {
  public:
    virtual ~PartFuncWMvWMpContext() = default;

    virtual cand_pos_t index_of(cand_pos_t i, cand_pos_t j) const = 0;
    virtual pf_t get_energy(cand_pos_t i, cand_pos_t j) = 0;
    virtual pf_t get_energy_WMB(cand_pos_t i, cand_pos_t j) = 0;
    virtual pf_t get_energy_WMv(cand_pos_t i, cand_pos_t j) = 0;
    virtual pf_t get_energy_WMp(cand_pos_t i, cand_pos_t j) = 0;
    virtual pf_t exp_MLstem(cand_pos_t i, cand_pos_t j) = 0;
    virtual pf_t expPSM_penalty() const = 0;
    virtual pf_t expb_penalty() const = 0;
    virtual pf_t expMLbase1() const = 0;
    virtual void set_WMv_WMp(cand_pos_t ij, pf_t wmv, pf_t wmp) = 0;
};

void compute_WMv_WMp_restricted(PartFuncWMvWMpContext &ctx, cand_pos_t i, cand_pos_t j, std::vector<Node> &tree);

class PartFuncWMContext {
  public:
    virtual ~PartFuncWMContext() = default;

    virtual cand_pos_t index_of(cand_pos_t i, cand_pos_t j) const = 0;
    virtual pf_t get_energy_WM(cand_pos_t i, cand_pos_t j) = 0;
    virtual pf_t get_energy_WMv(cand_pos_t i, cand_pos_t j) = 0;
    virtual pf_t get_energy_WMp(cand_pos_t i, cand_pos_t j) = 0;
    virtual pf_t get_energy(cand_pos_t i, cand_pos_t j) = 0;
    virtual pf_t get_energy_WMB(cand_pos_t i, cand_pos_t j) = 0;
    virtual pf_t exp_MLstem(cand_pos_t i, cand_pos_t j) = 0;
    virtual pf_t expPSM_penalty() const = 0;
    virtual pf_t expb_penalty() const = 0;
    virtual pf_t exp_Mbloop(cand_pos_t i, cand_pos_t j) = 0;
    virtual pf_t expMLclosing() const = 0;
    virtual pf_t expMLbase(cand_pos_t length) const = 0;
    virtual cand_pos_t turn() const = 0;
    virtual void set_WM(cand_pos_t ij, pf_t value) = 0;
};

void compute_WM_restricted(PartFuncWMContext &ctx, cand_pos_t i, cand_pos_t j, sparse_tree &tree);

} // namespace scfg

#endif
