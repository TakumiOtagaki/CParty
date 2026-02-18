#ifndef SCFG_TRANSITION_WEIGHTS_HH_
#define SCFG_TRANSITION_WEIGHTS_HH_

#include "base_types.hh"

namespace scfg {

// transition_weight_* から参照する重み計算の集約ラッパ。
// Context の実装詳細からルール重み計算を切り離すための窓口。
template <class Ctx>
class TransitionWeights {
  public:
    explicit TransitionWeights(Ctx &ctx) : ctx_(ctx) {}

    pf_t scale1() const { return ctx_.scale1(); }
    pf_t scale(cand_pos_t length) const { return ctx_.scale(length); }

    pf_t exp_Extloop(cand_pos_t i, cand_pos_t j) { return ctx_.exp_Extloop(i, j); }
    pf_t expPS_penalty() const { return ctx_.expPS_penalty(); }

    pf_t hairpin_energy(cand_pos_t i, cand_pos_t j) { return ctx_.hairpin_energy(i, j); }
    pf_t internal_energy(cand_pos_t i, cand_pos_t j, std::vector<int> &up) { return ctx_.internal_energy(i, j, up); }
    pf_t vm_energy(cand_pos_t i, cand_pos_t j, std::vector<int> &up) { return ctx_.vm_energy(i, j, up); }

    pf_t expPUP_pen1() const { return ctx_.expPUP_pen1(); }
    pf_t expPPS_penalty() const { return ctx_.expPPS_penalty(); }
    pf_t expPSP_penalty() const { return ctx_.expPSP_penalty(); }

    pf_t exp_Mbloop(cand_pos_t i, cand_pos_t j) { return ctx_.exp_Mbloop(i, j); }
    pf_t expMLclosing() const { return ctx_.expMLclosing(); }
    pf_t expMLbase(cand_pos_t length) const { return ctx_.expMLbase(length); }
    pf_t expMLbase1() const { return ctx_.expMLbase1(); }

    pf_t exp_MLstem(cand_pos_t i, cand_pos_t j) { return ctx_.exp_MLstem(i, j); }
    pf_t expPSM_penalty() const { return ctx_.expPSM_penalty(); }
    pf_t expb_penalty() const { return ctx_.expb_penalty(); }

    pf_t expbp_penalty() const { return ctx_.expbp_penalty(); }
    pf_t expbp_penalty_sq() const { return ctx_.expbp_penalty_sq(); }
    pf_t expcp_pen(cand_pos_t length) const { return ctx_.expcp_pen(length); }
    pf_t expap_penalty() const { return ctx_.expap_penalty(); }

    pf_t get_e_stP(cand_pos_t i, cand_pos_t j) { return ctx_.get_e_stP(i, j); }
    pf_t get_e_intP(cand_pos_t i, cand_pos_t k, cand_pos_t l, cand_pos_t j) {
        return ctx_.get_e_intP(i, k, l, j);
    }

    pf_t expPB_penalty() const { return ctx_.expPB_penalty(); }

  private:
    Ctx &ctx_;
};

} // namespace scfg

#endif
