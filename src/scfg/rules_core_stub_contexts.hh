#ifndef CPARTY_RULES_CORE_STUB_CONTEXTS_HH
#define CPARTY_RULES_CORE_STUB_CONTEXTS_HH

#include "scfg/rules_part_func.hh"
#include "sparse_tree.hh"

#include <ViennaRNA/params/constants.h>

namespace cparty::scfg::detail {

class RuleCoreStubWContext final : public ::scfg::PartFuncWContext {
 public:
  explicit RuleCoreStubWContext(int n) : n_(n) {}
  cand_pos_t n() const override { return n_; }
  pf_t scale1() const override { return 0; }
  pf_t get_W(cand_pos_t) const override { return 0; }
  pf_t get_V(cand_pos_t, cand_pos_t) override { return 0; }
  pf_t get_WMB(cand_pos_t, cand_pos_t) override { return 0; }
  pf_t exp_Extloop(cand_pos_t, cand_pos_t) override { return 0; }
  pf_t expPS_penalty() const override { return 0; }
  void set_W(cand_pos_t, pf_t) override {}
  cand_pos_t turn() const override { return TURN; }

 private:
  cand_pos_t n_;
};

class RuleCoreStubWIContext final : public ::scfg::PartFuncWIContext {
 public:
  RuleCoreStubWIContext() = default;
  cand_pos_t index_of(cand_pos_t, cand_pos_t) const override { return 0; }
  void set_WI(cand_pos_t, pf_t) override {}
  pf_t get_WI(cand_pos_t, cand_pos_t) override { return 0; }
  pf_t get_V(cand_pos_t, cand_pos_t) override { return 0; }
  pf_t get_WMB(cand_pos_t, cand_pos_t) override { return 0; }
  pf_t expPPS_penalty() const override { return 0; }
  pf_t expPSP_penalty() const override { return 0; }
  pf_t expPUP_pen1() const override { return 0; }
  cand_pos_t turn() const override { return TURN; }
};

class RuleCoreStubVContext final : public ::scfg::PartFuncVContext {
 public:
  cand_pos_t index_of(cand_pos_t, cand_pos_t) const override { return 0; }
  void set_V(cand_pos_t, pf_t) override {}
  pf_t hairpin_energy(cand_pos_t, cand_pos_t) override { return 0; }
  pf_t internal_energy(cand_pos_t, cand_pos_t, std::vector<int> &) override { return 0; }
  pf_t vm_energy(cand_pos_t, cand_pos_t, std::vector<int> &) override { return 0; }
};

class RuleCoreStubVMContext final : public ::scfg::PartFuncVMContext {
 public:
  cand_pos_t index_of(cand_pos_t, cand_pos_t) const override { return 0; }
  void set_VM(cand_pos_t, pf_t) override {}
  pf_t get_WM(cand_pos_t, cand_pos_t) override { return 0; }
  pf_t get_WMv(cand_pos_t, cand_pos_t) override { return 0; }
  pf_t get_WMp(cand_pos_t, cand_pos_t) override { return 0; }
  pf_t exp_Mbloop(cand_pos_t, cand_pos_t) override { return 0; }
  pf_t expMLclosing() const override { return 0; }
  pf_t expMLbase(cand_pos_t) const override { return 0; }
  pf_t scale2() const override { return 0; }
  cand_pos_t turn() const override { return TURN; }
};

class RuleCoreStubWMvWMpContext final : public ::scfg::PartFuncWMvWMpContext {
 public:
  cand_pos_t index_of(cand_pos_t, cand_pos_t) const override { return 0; }
  pf_t get_V(cand_pos_t, cand_pos_t) override { return 0; }
  pf_t get_WMB(cand_pos_t, cand_pos_t) override { return 0; }
  pf_t get_WMv(cand_pos_t, cand_pos_t) override { return 0; }
  pf_t get_WMp(cand_pos_t, cand_pos_t) override { return 0; }
  pf_t exp_MLstem(cand_pos_t, cand_pos_t) override { return 0; }
  pf_t expPSM_penalty() const override { return 0; }
  pf_t expb_penalty() const override { return 0; }
  pf_t expMLbase1() const override { return 0; }
  cand_pos_t turn() const override { return TURN; }
  void set_WMv_WMp(cand_pos_t, pf_t, pf_t) override {}
};

class RuleCoreStubWMContext final : public ::scfg::PartFuncWMContext {
 public:
  cand_pos_t index_of(cand_pos_t, cand_pos_t) const override { return 0; }
  pf_t get_WM(cand_pos_t, cand_pos_t) override { return 0; }
  pf_t get_WMv(cand_pos_t, cand_pos_t) override { return 0; }
  pf_t get_WMp(cand_pos_t, cand_pos_t) override { return 0; }
  pf_t get_V(cand_pos_t, cand_pos_t) override { return 0; }
  pf_t get_WMB(cand_pos_t, cand_pos_t) override { return 0; }
  pf_t exp_MLstem(cand_pos_t, cand_pos_t) override { return 0; }
  pf_t expPSM_penalty() const override { return 0; }
  pf_t expb_penalty() const override { return 0; }
  pf_t exp_Mbloop(cand_pos_t, cand_pos_t) override { return 0; }
  pf_t expMLclosing() const override { return 0; }
  pf_t expMLbase(cand_pos_t) const override { return 0; }
  cand_pos_t turn() const override { return TURN; }
  void set_WM(cand_pos_t, pf_t) override {}
};

class RuleCoreStubWIPContext final : public ::scfg::PartFuncWIPContext {
 public:
  cand_pos_t index_of(cand_pos_t, cand_pos_t) const override { return 0; }
  pf_t get_V(cand_pos_t, cand_pos_t) override { return 0; }
  pf_t get_WMB(cand_pos_t, cand_pos_t) override { return 0; }
  pf_t get_WIP(cand_pos_t, cand_pos_t) override { return 0; }
  pf_t expbp_penalty() const override { return 0; }
  pf_t expPSM_penalty() const override { return 0; }
  pf_t expcp_pen(cand_pos_t) const override { return 0; }
  void set_WIP(cand_pos_t, pf_t) override {}
  cand_pos_t turn() const override { return TURN; }
};

class RuleCoreStubVPLContext final : public ::scfg::PartFuncVPLContext {
 public:
  cand_pos_t index_of(cand_pos_t, cand_pos_t) const override { return 0; }
  pf_t get_VP(cand_pos_t, cand_pos_t) override { return 0; }
  pf_t expcp_pen(cand_pos_t) const override { return 0; }
  void set_VPL(cand_pos_t, pf_t) override {}
};

class RuleCoreStubVPRContext final : public ::scfg::PartFuncVPRContext {
 public:
  cand_pos_t index_of(cand_pos_t, cand_pos_t) const override { return 0; }
  pf_t get_VP(cand_pos_t, cand_pos_t) override { return 0; }
  pf_t get_WIP(cand_pos_t, cand_pos_t) override { return 0; }
  pf_t expcp_pen(cand_pos_t) const override { return 0; }
  void set_VPR(cand_pos_t, pf_t) override {}
};

inline bool is_allowed_pair(char left, char right) {
  return (left == 'A' && right == 'U') || (left == 'U' && right == 'A') ||
         (left == 'G' && right == 'C') || (left == 'C' && right == 'G') ||
         (left == 'G' && right == 'U') || (left == 'U' && right == 'G');
}

class RuleCoreStubVPContext final : public ::scfg::PartFuncVPContext {
 public:
  explicit RuleCoreStubVPContext(const std::string &seq) : seq_(seq) {}

  cand_pos_t index_of(cand_pos_t, cand_pos_t) const override { return 0; }
  pf_t get_WI(cand_pos_t, cand_pos_t) override { return 0; }
  pf_t get_VP(cand_pos_t, cand_pos_t) override { return 0; }
  pf_t get_WIP(cand_pos_t, cand_pos_t) override { return 0; }
  pf_t get_VPL(cand_pos_t, cand_pos_t) override { return 0; }
  pf_t get_VPR(cand_pos_t, cand_pos_t) override { return 0; }
  pf_t get_V(cand_pos_t, cand_pos_t) override { return 0; }
  pair_type pair_type_of(cand_pos_t i, cand_pos_t j) const override {
    if (i < 1 || j < 1 || static_cast<size_t>(i) > seq_.size() ||
        static_cast<size_t>(j) > seq_.size()) {
      return 0;
    }
    const char left = seq_[static_cast<size_t>(i - 1)];
    const char right = seq_[static_cast<size_t>(j - 1)];
    return is_allowed_pair(left, right) ? 1 : 0;
  }
  pf_t get_e_stP(cand_pos_t, cand_pos_t) override { return 0; }
  pf_t get_e_intP(cand_pos_t, cand_pos_t, cand_pos_t, cand_pos_t) override { return 0; }
  pf_t expap_penalty() const override { return 0; }
  pf_t expbp_penalty() const override { return 0; }
  pf_t expcp_pen(cand_pos_t) const override { return 0; }
  pf_t scale(cand_pos_t) const override { return 0; }
  pf_t expbp_penalty_sq() const override { return 0; }
  void set_VP(cand_pos_t, pf_t) override {}

 private:
  const std::string &seq_;
};

class RuleCoreStubWMBWContext final : public ::scfg::PartFuncWMBWContext {
 public:
  cand_pos_t index_of(cand_pos_t, cand_pos_t) const override { return 0; }
  pf_t get_WMBP(cand_pos_t, cand_pos_t) override { return 0; }
  pf_t get_WI(cand_pos_t, cand_pos_t) override { return 0; }
  void set_WMBW(cand_pos_t, pf_t) override {}
};

class RuleCoreStubWMBPContext final : public ::scfg::PartFuncWMBPContext {
 public:
  explicit RuleCoreStubWMBPContext(int n) : n_(n) {}
  cand_pos_t index_of(cand_pos_t, cand_pos_t) const override { return 0; }
  pf_t get_WMBP(cand_pos_t, cand_pos_t) override { return 0; }
  pf_t get_WMBW(cand_pos_t, cand_pos_t) override { return 0; }
  pf_t get_VP(cand_pos_t, cand_pos_t) override { return 0; }
  pf_t get_WI(cand_pos_t, cand_pos_t) override { return 0; }
  pf_t get_V(cand_pos_t, cand_pos_t) override { return 0; }
  pf_t get_BE(cand_pos_t, cand_pos_t, cand_pos_t, cand_pos_t, sparse_tree &) override { return 0; }
  pf_t expPB_penalty() const override { return 1; }
  cand_pos_t n() const override { return n_; }
  int compute_exterior_cases(cand_pos_t, cand_pos_t, sparse_tree &) override { return 0; }
  void set_WMBP(cand_pos_t, pf_t) override {}

 private:
  cand_pos_t n_;
};

class RuleCoreStubWMBContext final : public ::scfg::PartFuncWMBContext {
 public:
  explicit RuleCoreStubWMBContext(int n) : n_(n) {}
  cand_pos_t index_of(cand_pos_t, cand_pos_t) const override { return 0; }
  pf_t get_WMBP(cand_pos_t, cand_pos_t) override { return 0; }
  pf_t get_WI(cand_pos_t, cand_pos_t) override { return 0; }
  pf_t get_V(cand_pos_t, cand_pos_t) override { return 0; }
  pf_t get_BE(cand_pos_t, cand_pos_t, cand_pos_t, cand_pos_t, sparse_tree &) override { return 0; }
  pf_t expPB_penalty() const override { return 1; }
  cand_pos_t n() const override { return n_; }
  void set_WMB(cand_pos_t, pf_t) override {}

 private:
  cand_pos_t n_;
};

class RuleCoreStubBEContext final : public ::scfg::PartFuncBEContext {
 public:
  explicit RuleCoreStubBEContext(int n) : n_(n) {}
  cand_pos_t index_of(cand_pos_t, cand_pos_t) const override { return 0; }
  cand_pos_t n() const override { return n_; }
  pf_t get_WIP(cand_pos_t, cand_pos_t) override { return 0; }
  pf_t get_V(cand_pos_t, cand_pos_t) override { return 0; }
  pf_t get_BE(cand_pos_t, cand_pos_t, cand_pos_t, cand_pos_t, sparse_tree &) override { return 0; }
  pf_t get_e_stP(cand_pos_t, cand_pos_t) override { return 0; }
  pf_t get_e_intP(cand_pos_t, cand_pos_t, cand_pos_t, cand_pos_t) override { return 0; }
  pf_t expap_penalty() const override { return 0; }
  pf_t expbp_penalty_sq() const override { return 0; }
  pf_t expcp_pen(cand_pos_t) const override { return 0; }
  pf_t scale(cand_pos_t) const override { return 0; }
  void set_BE(cand_pos_t, pf_t) override {}

 private:
  cand_pos_t n_;
};

}  // namespace cparty::scfg::detail

#endif  // CPARTY_RULES_CORE_STUB_CONTEXTS_HH
