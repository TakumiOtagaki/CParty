#include "fixed_energy_api.hh"

#include "fixed_energy_input.hh"
#include "fixed_energy_shared_parse.hh"
#include "scfg/rules_core.hh"
#include "scfg/rules_core_stub_contexts.hh"
#include "sparse_tree.hh"

#include <ViennaRNA/params/constants.h>

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <sys/stat.h>

extern "C" {
#include "ViennaRNA/loops/all.h"
#include "ViennaRNA/pair_mat.h"
#include "ViennaRNA/params/io.h"
}

namespace cparty {
namespace {

bool file_exists(const std::string &path) {
  struct stat buffer;
  return (stat(path.c_str(), &buffer) == 0);
}

void ensure_vienna_params_loaded(const std::string &seq) {
  static bool loaded = false;
  if (loaded) {
    return;
  }
  const std::string params_path = "params/rna_DirksPierce09.par";
  if (file_exists(params_path)) {
    vrna_params_load(params_path.c_str(), VRNA_PARAMETER_FORMAT_DEFAULT);
  } else if (seq.find('T') != std::string::npos) {
    vrna_params_load_DNA_Mathews2004();
  }
  loaded = true;
}

bool split_matches(const ::scfg::RuleSplit &lhs, const ::scfg::RuleSplit &rhs) {
  return lhs.k == rhs.k && lhs.l == rhs.l && lhs.p == rhs.p && lhs.q == rhs.q;
}

std::string nonterminal_name(const ::scfg::NonTerminal nonterminal) {
  switch (nonterminal) {
    case ::scfg::NonTerminal::W:
      return "W";
    case ::scfg::NonTerminal::WI:
      return "WI";
    case ::scfg::NonTerminal::V:
      return "V";
    default:
      break;
  }
  return "UNKNOWN";
}

std::vector<internal::RuleTraceStep> trace_rule_chain_slice_a_rules_core_from_normalized(const NormalizedInput &ctx) {
  if (!is_pk_free_structure(ctx.db_full) && !is_h_type_structure(ctx.db_full)) {
    fail_invalid_input("rules_core slice-a trace requires pk_free or h_type structure");
  }
  const std::string tree_db =
      is_h_type_structure(ctx.db_full) ? normalize_h_type_brackets(ctx.db_full) : ctx.db_full;
  const int n = static_cast<int>(tree_db.size());
  sparse_tree tree(tree_db, n);
  ::cparty::scfg::detail::RuleCoreStubWContext wctx(n);
  ::cparty::scfg::detail::RuleCoreStubVContext vctx;

  struct Item {
    ::scfg::NonTerminal nonterminal;
    cand_pos_t i;
    cand_pos_t j;
  };

  std::vector<Item> stack;
  stack.push_back({::scfg::NonTerminal::W, 1, n});

  std::vector<internal::RuleTraceStep> out;
  while (!stack.empty()) {
    const Item cur = stack.back();
    stack.pop_back();

    if (cur.nonterminal == ::scfg::NonTerminal::W) {
      if (cur.i > cur.j) {
        continue;
      }
      ::scfg::RuleId selected_rule = ::scfg::RuleId::W_EXTEND_UNPAIRED;
      ::scfg::RuleSplit selected_split;
      std::vector<::scfg::RuleChild> children;
      if (tree.tree[cur.j].pair < 0) {
        selected_rule = ::scfg::RuleId::W_EXTEND_UNPAIRED;
        children = ::scfg::expand_w(selected_rule, cur.i, cur.j, selected_split);
      } else {
        const cand_pos_t k = tree.tree[cur.j].pair;
        if (k < cur.i) {
          fail_invalid_input("rules_core slice-a trace found invalid W split");
        }
        selected_rule = ::scfg::RuleId::W_SPLIT_V;
        selected_split.k = k;
        children = ::scfg::expand_w(selected_rule, cur.i, cur.j, selected_split);
      }

      const auto applicable = ::scfg::applicable_rules_w(cur.i, cur.j, wctx, tree);
      bool matched = false;
      for (const auto &entry : applicable) {
        if (entry.rule == selected_rule && split_matches(entry.split, selected_split)) {
          matched = true;
          break;
        }
      }
      if (!matched) {
        fail_invalid_input("rules_core slice-a trace could not validate W rule selection");
      }

      out.push_back({nonterminal_name(cur.nonterminal), cur.i, cur.j, ::scfg::rule_id_name(selected_rule)});
      for (auto it = children.rbegin(); it != children.rend(); ++it) {
        stack.push_back({it->nonterminal, it->i, it->j});
      }
      continue;
    }

    if (cur.nonterminal == ::scfg::NonTerminal::V) {
      if (cur.i > cur.j) {
        fail_invalid_input("rules_core slice-a trace encountered empty V span");
      }
      if (tree.tree[cur.i].pair != cur.j) {
        fail_invalid_input("rules_core slice-a trace encountered unpaired V span");
      }

      const auto child_count = tree.tree[cur.i].children.size();
      ::scfg::RuleId selected_rule = ::scfg::RuleId::V_HAIRPIN;
      if (child_count == 0) {
        selected_rule = ::scfg::RuleId::V_HAIRPIN;
      } else if (child_count == 1) {
        selected_rule = ::scfg::RuleId::V_INTERNAL;
      } else {
        selected_rule = ::scfg::RuleId::V_VM;
      }

      const auto applicable = ::scfg::applicable_rules_v(cur.i, cur.j, vctx, tree);
      bool matched = false;
      for (const auto &entry : applicable) {
        if (entry.rule == selected_rule) {
          matched = true;
          break;
        }
      }
      if (!matched) {
        fail_invalid_input("rules_core slice-a trace could not validate V rule selection");
      }

      out.push_back({nonterminal_name(cur.nonterminal), cur.i, cur.j, ::scfg::rule_id_name(selected_rule)});
      continue;
    }

    fail_invalid_input("rules_core slice-a trace hit unexpected state " +
                       nonterminal_name(cur.nonterminal));
  }

  return out;
}

std::vector<internal::RuleTraceStep> trace_rule_chain_slice_a_from_normalized(const NormalizedInput &ctx) {
  return trace_rule_chain_slice_a_rules_core_from_normalized(ctx);
}

std::vector<internal::RuleTraceStep> trace_rule_chain_slice_b_rules_core_from_normalized(const NormalizedInput &ctx) {
  if (!is_pk_free_structure(ctx.db_full) && !is_h_type_structure(ctx.db_full)) {
    fail_invalid_input("rules_core slice-b trace requires pk_free or h_type structure");
  }
  const auto shared_trace = trace_rule_chain_slice_b_shared_from_normalized(ctx);
  const std::string tree_db =
      is_h_type_structure(ctx.db_full) ? normalize_h_type_brackets(ctx.db_full) : ctx.db_full;
  const int n = static_cast<int>(tree_db.size());
  sparse_tree tree(tree_db, n);
  ::cparty::scfg::detail::RuleCoreStubWContext wctx(n);
  ::cparty::scfg::detail::RuleCoreStubWIContext wictx;
  ::cparty::scfg::detail::RuleCoreStubVContext vctx;
  ::cparty::scfg::detail::RuleCoreStubVMContext vmctx;
  ::cparty::scfg::detail::RuleCoreStubWMvWMpContext wmvwmpctx;
  ::cparty::scfg::detail::RuleCoreStubWMContext wmctx;

  auto wmv_wmp_rules = [&](::scfg::NonTerminal target, cand_pos_t i, cand_pos_t j) {
    std::vector<::scfg::ApplicableRule> out;
    for (::scfg::RuleId rule : ::scfg::rules_for(target)) {
      const auto splits = ::scfg::enumerate_splits_wmv_wmp(rule, i, j, wmvwmpctx, tree.tree);
      for (const auto &split : splits) {
        out.push_back({rule, split});
      }
    }
    return out;
  };

  std::vector<internal::RuleTraceStep> out;
  out.reserve(shared_trace.size());
  for (const auto &step : shared_trace) {
    if (step.state == "W") {
      const auto applicable = ::scfg::applicable_rules_w(step.i, step.j, wctx, tree);
      if (applicable.empty()) {
        fail_invalid_input("rules_core slice-b trace found no W rules");
      }
      out.push_back({step.state, step.i, step.j, ::scfg::rule_id_name(applicable.front().rule)});
      continue;
    }
    if (step.state == "WI") {
      const auto applicable = ::scfg::applicable_rules_wi(step.i, step.j, wictx, tree);
      if (applicable.empty()) {
        fail_invalid_input("rules_core slice-b trace found no WI rules");
      }
      out.push_back({step.state, step.i, step.j, ::scfg::rule_id_name(applicable.front().rule)});
      continue;
    }
    if (step.state == "V") {
      if (step.i > step.j) {
        out.push_back({step.state, step.i, step.j, "V_EMPTY"});
        continue;
      }
      const auto applicable = ::scfg::applicable_rules_v(step.i, step.j, vctx, tree);
      if (applicable.empty()) {
        fail_invalid_input("rules_core slice-b trace found no V rules");
      }
      out.push_back({step.state, step.i, step.j, ::scfg::rule_id_name(applicable.front().rule)});
      continue;
    }
    if (step.state == "VM") {
      auto applicable = ::scfg::applicable_rules_vm(step.i, step.j, vmctx, tree.up);
      if (applicable.empty()) {
        out.push_back({step.state, step.i, step.j, ::scfg::rule_id_name(::scfg::RuleId::VM_SCALE2)});
        continue;
      }
      out.push_back({step.state, step.i, step.j, ::scfg::rule_id_name(applicable.front().rule)});
      continue;
    }
    if (step.state == "WMv") {
      const auto applicable = wmv_wmp_rules(::scfg::NonTerminal::WMv, step.i, step.j);
      if (applicable.empty()) {
        out.push_back({step.state, step.i, step.j, ::scfg::rule_id_name(::scfg::RuleId::WMv_EXTEND_UNPAIRED)});
        continue;
      }
      out.push_back({step.state, step.i, step.j, ::scfg::rule_id_name(applicable.front().rule)});
      continue;
    }
    if (step.state == "WMp") {
      const auto applicable = wmv_wmp_rules(::scfg::NonTerminal::WMp, step.i, step.j);
      if (applicable.empty()) {
        out.push_back({step.state, step.i, step.j, ::scfg::rule_id_name(::scfg::RuleId::WMp_EXTEND_UNPAIRED)});
        continue;
      }
      out.push_back({step.state, step.i, step.j, ::scfg::rule_id_name(applicable.front().rule)});
      continue;
    }
    if (step.state == "WM") {
      const auto applicable = ::scfg::applicable_rules_wm(step.i, step.j, wmctx, tree);
      if (applicable.empty()) {
        out.push_back({step.state, step.i, step.j, ::scfg::rule_id_name(::scfg::RuleId::WM_EXTEND_UNPAIRED)});
        continue;
      }
      out.push_back({step.state, step.i, step.j, ::scfg::rule_id_name(applicable.front().rule)});
      continue;
    }
    fail_invalid_input("rules_core slice-b trace hit unexpected state " + step.state);
  }
  return out;
}

std::vector<internal::RuleTraceStep> trace_rule_chain_slice_b_from_normalized(const NormalizedInput &ctx) {
  return trace_rule_chain_slice_b_rules_core_from_normalized(ctx);
}

std::vector<internal::RuleTraceStep> trace_rule_chain_slice_c_rules_core_from_normalized(const NormalizedInput &ctx) {
  if (!is_pk_free_structure(ctx.db_full) && !is_h_type_structure(ctx.db_full)) {
    fail_invalid_input("rules_core slice-c trace requires pk_free or h_type structure");
  }
  const auto shared_trace = trace_rule_chain_slice_c_shared_from_normalized(ctx);
  const std::string tree_db =
      is_h_type_structure(ctx.db_full) ? normalize_h_type_brackets(ctx.db_full) : ctx.db_full;
  const int n = static_cast<int>(tree_db.size());
  sparse_tree tree(tree_db, n);
  ::cparty::scfg::detail::RuleCoreStubWContext wctx(n);
  ::cparty::scfg::detail::RuleCoreStubWIContext wictx;
  ::cparty::scfg::detail::RuleCoreStubVContext vctx;
  ::cparty::scfg::detail::RuleCoreStubVMContext vmctx;
  ::cparty::scfg::detail::RuleCoreStubWMvWMpContext wmvwmpctx;
  ::cparty::scfg::detail::RuleCoreStubWMContext wmctx;
  ::cparty::scfg::detail::RuleCoreStubWIPContext wipctx;
  ::cparty::scfg::detail::RuleCoreStubVPLContext vplctx;
  ::cparty::scfg::detail::RuleCoreStubVPRContext vprctx;
  ::cparty::scfg::detail::RuleCoreStubVPContext vpctx(ctx.seq);

  auto wmv_wmp_rules = [&](::scfg::NonTerminal target, cand_pos_t i, cand_pos_t j) {
    std::vector<::scfg::ApplicableRule> out;
    for (::scfg::RuleId rule : ::scfg::rules_for(target)) {
      const auto splits = ::scfg::enumerate_splits_wmv_wmp(rule, i, j, wmvwmpctx, tree.tree);
      for (const auto &split : splits) {
        out.push_back({rule, split});
      }
    }
    return out;
  };

  auto fallback_rule_name = [](::scfg::NonTerminal nt) {
    return ::scfg::rule_id_name(::scfg::rules_for(nt).front());
  };

  std::vector<internal::RuleTraceStep> out;
  out.reserve(shared_trace.size());
  for (const auto &step : shared_trace) {
    if (step.state == "W") {
      const auto applicable = ::scfg::applicable_rules_w(step.i, step.j, wctx, tree);
      out.push_back({step.state, step.i, step.j,
                     applicable.empty() ? fallback_rule_name(::scfg::NonTerminal::W)
                                        : ::scfg::rule_id_name(applicable.front().rule)});
      continue;
    }
    if (step.state == "WI") {
      const auto applicable = ::scfg::applicable_rules_wi(step.i, step.j, wictx, tree);
      out.push_back({step.state, step.i, step.j,
                     applicable.empty() ? fallback_rule_name(::scfg::NonTerminal::WI)
                                        : ::scfg::rule_id_name(applicable.front().rule)});
      continue;
    }
    if (step.state == "V") {
      if (step.i > step.j) {
        out.push_back({step.state, step.i, step.j, "V_EMPTY"});
        continue;
      }
      const auto applicable = ::scfg::applicable_rules_v(step.i, step.j, vctx, tree);
      out.push_back({step.state, step.i, step.j,
                     applicable.empty() ? fallback_rule_name(::scfg::NonTerminal::V)
                                        : ::scfg::rule_id_name(applicable.front().rule)});
      continue;
    }
    if (step.state == "VM") {
      auto applicable = ::scfg::applicable_rules_vm(step.i, step.j, vmctx, tree.up);
      if (applicable.empty()) {
        out.push_back({step.state, step.i, step.j, ::scfg::rule_id_name(::scfg::RuleId::VM_SCALE2)});
        continue;
      }
      out.push_back({step.state, step.i, step.j, ::scfg::rule_id_name(applicable.front().rule)});
      continue;
    }
    if (step.state == "WMv") {
      const auto applicable = wmv_wmp_rules(::scfg::NonTerminal::WMv, step.i, step.j);
      out.push_back({step.state, step.i, step.j,
                     applicable.empty() ? ::scfg::rule_id_name(::scfg::RuleId::WMv_EXTEND_UNPAIRED)
                                        : ::scfg::rule_id_name(applicable.front().rule)});
      continue;
    }
    if (step.state == "WMp") {
      const auto applicable = wmv_wmp_rules(::scfg::NonTerminal::WMp, step.i, step.j);
      out.push_back({step.state, step.i, step.j,
                     applicable.empty() ? ::scfg::rule_id_name(::scfg::RuleId::WMp_EXTEND_UNPAIRED)
                                        : ::scfg::rule_id_name(applicable.front().rule)});
      continue;
    }
    if (step.state == "WM") {
      const auto applicable = ::scfg::applicable_rules_wm(step.i, step.j, wmctx, tree);
      out.push_back({step.state, step.i, step.j,
                     applicable.empty() ? ::scfg::rule_id_name(::scfg::RuleId::WM_EXTEND_UNPAIRED)
                                        : ::scfg::rule_id_name(applicable.front().rule)});
      continue;
    }
    if (step.state == "WIP") {
      const auto applicable = ::scfg::applicable_rules_wip(step.i, step.j, wipctx, tree);
      out.push_back({step.state, step.i, step.j,
                     applicable.empty() ? fallback_rule_name(::scfg::NonTerminal::WIP)
                                        : ::scfg::rule_id_name(applicable.front().rule)});
      continue;
    }
    if (step.state == "VP") {
      const auto applicable = ::scfg::applicable_rules_vp(step.i, step.j, vpctx, tree);
      out.push_back({step.state, step.i, step.j,
                     applicable.empty() ? fallback_rule_name(::scfg::NonTerminal::VP)
                                        : ::scfg::rule_id_name(applicable.front().rule)});
      continue;
    }
    if (step.state == "VPL") {
      const auto applicable = ::scfg::applicable_rules_vpl(step.i, step.j, vplctx, tree);
      out.push_back({step.state, step.i, step.j,
                     applicable.empty() ? fallback_rule_name(::scfg::NonTerminal::VPL)
                                        : ::scfg::rule_id_name(applicable.front().rule)});
      continue;
    }
    if (step.state == "VPR") {
      const auto applicable = ::scfg::applicable_rules_vpr(step.i, step.j, vprctx, tree);
      out.push_back({step.state, step.i, step.j,
                     applicable.empty() ? fallback_rule_name(::scfg::NonTerminal::VPR)
                                        : ::scfg::rule_id_name(applicable.front().rule)});
      continue;
    }
    fail_invalid_input("rules_core slice-c trace hit unexpected state " + step.state);
  }
  return out;
}

std::vector<internal::RuleTraceStep> trace_rule_chain_slice_c_from_normalized(const NormalizedInput &ctx) {
  return trace_rule_chain_slice_c_rules_core_from_normalized(ctx);
}

std::vector<internal::RuleTraceStep> trace_rule_chain_slice_d_rules_core_from_normalized(const NormalizedInput &ctx) {
  if (!is_pk_free_structure(ctx.db_full) && !is_h_type_structure(ctx.db_full)) {
    fail_invalid_input("rules_core slice-d trace requires pk_free or h_type structure");
  }
  const auto shared_trace = trace_rule_chain_slice_d_shared_from_normalized(ctx);
  const std::string tree_db =
      is_h_type_structure(ctx.db_full) ? normalize_h_type_brackets(ctx.db_full) : ctx.db_full;
  const int n = static_cast<int>(tree_db.size());
  sparse_tree tree(tree_db, n);
  ::cparty::scfg::detail::RuleCoreStubWContext wctx(n);
  ::cparty::scfg::detail::RuleCoreStubWIContext wictx;
  ::cparty::scfg::detail::RuleCoreStubVContext vctx;
  ::cparty::scfg::detail::RuleCoreStubVMContext vmctx;
  ::cparty::scfg::detail::RuleCoreStubWMvWMpContext wmvwmpctx;
  ::cparty::scfg::detail::RuleCoreStubWMContext wmctx;
  ::cparty::scfg::detail::RuleCoreStubWIPContext wipctx;
  ::cparty::scfg::detail::RuleCoreStubVPLContext vplctx;
  ::cparty::scfg::detail::RuleCoreStubVPRContext vprctx;
  ::cparty::scfg::detail::RuleCoreStubVPContext vpctx(ctx.seq);
  ::cparty::scfg::detail::RuleCoreStubWMBWContext wmbwctx;
  ::cparty::scfg::detail::RuleCoreStubWMBPContext wmbpctx(n);
  ::cparty::scfg::detail::RuleCoreStubWMBContext wmbctx(n);
  ::cparty::scfg::detail::RuleCoreStubBEContext bectx(n);

  auto wmv_wmp_rules = [&](::scfg::NonTerminal target, cand_pos_t i, cand_pos_t j) {
    std::vector<::scfg::ApplicableRule> out;
    for (::scfg::RuleId rule : ::scfg::rules_for(target)) {
      const auto splits = ::scfg::enumerate_splits_wmv_wmp(rule, i, j, wmvwmpctx, tree.tree);
      for (const auto &split : splits) {
        out.push_back({rule, split});
      }
    }
    return out;
  };

  auto fallback_rule_name = [](::scfg::NonTerminal nt) {
    return ::scfg::rule_id_name(::scfg::rules_for(nt).front());
  };

  std::vector<internal::RuleTraceStep> out;
  out.reserve(shared_trace.size());
  for (const auto &step : shared_trace) {
    if (step.state == "W") {
      const auto applicable = ::scfg::applicable_rules_w(step.i, step.j, wctx, tree);
      out.push_back({step.state, step.i, step.j,
                     applicable.empty() ? fallback_rule_name(::scfg::NonTerminal::W)
                                        : ::scfg::rule_id_name(applicable.front().rule)});
      continue;
    }
    if (step.state == "WI") {
      const auto applicable = ::scfg::applicable_rules_wi(step.i, step.j, wictx, tree);
      out.push_back({step.state, step.i, step.j,
                     applicable.empty() ? fallback_rule_name(::scfg::NonTerminal::WI)
                                        : ::scfg::rule_id_name(applicable.front().rule)});
      continue;
    }
    if (step.state == "V") {
      if (step.i > step.j) {
        out.push_back({step.state, step.i, step.j, "V_EMPTY"});
        continue;
      }
      const auto applicable = ::scfg::applicable_rules_v(step.i, step.j, vctx, tree);
      out.push_back({step.state, step.i, step.j,
                     applicable.empty() ? fallback_rule_name(::scfg::NonTerminal::V)
                                        : ::scfg::rule_id_name(applicable.front().rule)});
      continue;
    }
    if (step.state == "VM") {
      auto applicable = ::scfg::applicable_rules_vm(step.i, step.j, vmctx, tree.up);
      if (applicable.empty()) {
        out.push_back({step.state, step.i, step.j, ::scfg::rule_id_name(::scfg::RuleId::VM_SCALE2)});
        continue;
      }
      out.push_back({step.state, step.i, step.j, ::scfg::rule_id_name(applicable.front().rule)});
      continue;
    }
    if (step.state == "WMv") {
      const auto applicable = wmv_wmp_rules(::scfg::NonTerminal::WMv, step.i, step.j);
      out.push_back({step.state, step.i, step.j,
                     applicable.empty() ? ::scfg::rule_id_name(::scfg::RuleId::WMv_EXTEND_UNPAIRED)
                                        : ::scfg::rule_id_name(applicable.front().rule)});
      continue;
    }
    if (step.state == "WMp") {
      const auto applicable = wmv_wmp_rules(::scfg::NonTerminal::WMp, step.i, step.j);
      out.push_back({step.state, step.i, step.j,
                     applicable.empty() ? ::scfg::rule_id_name(::scfg::RuleId::WMp_EXTEND_UNPAIRED)
                                        : ::scfg::rule_id_name(applicable.front().rule)});
      continue;
    }
    if (step.state == "WM") {
      const auto applicable = ::scfg::applicable_rules_wm(step.i, step.j, wmctx, tree);
      out.push_back({step.state, step.i, step.j,
                     applicable.empty() ? ::scfg::rule_id_name(::scfg::RuleId::WM_EXTEND_UNPAIRED)
                                        : ::scfg::rule_id_name(applicable.front().rule)});
      continue;
    }
    if (step.state == "WIP") {
      const auto applicable = ::scfg::applicable_rules_wip(step.i, step.j, wipctx, tree);
      out.push_back({step.state, step.i, step.j,
                     applicable.empty() ? fallback_rule_name(::scfg::NonTerminal::WIP)
                                        : ::scfg::rule_id_name(applicable.front().rule)});
      continue;
    }
    if (step.state == "VP") {
      const auto applicable = ::scfg::applicable_rules_vp(step.i, step.j, vpctx, tree);
      out.push_back({step.state, step.i, step.j,
                     applicable.empty() ? fallback_rule_name(::scfg::NonTerminal::VP)
                                        : ::scfg::rule_id_name(applicable.front().rule)});
      continue;
    }
    if (step.state == "VPL") {
      const auto applicable = ::scfg::applicable_rules_vpl(step.i, step.j, vplctx, tree);
      out.push_back({step.state, step.i, step.j,
                     applicable.empty() ? fallback_rule_name(::scfg::NonTerminal::VPL)
                                        : ::scfg::rule_id_name(applicable.front().rule)});
      continue;
    }
    if (step.state == "VPR") {
      const auto applicable = ::scfg::applicable_rules_vpr(step.i, step.j, vprctx, tree);
      out.push_back({step.state, step.i, step.j,
                     applicable.empty() ? fallback_rule_name(::scfg::NonTerminal::VPR)
                                        : ::scfg::rule_id_name(applicable.front().rule)});
      continue;
    }
    if (step.state == "WMBW") {
      const auto applicable = ::scfg::applicable_rules_wmbw(step.i, step.j, wmbwctx, tree);
      out.push_back({step.state, step.i, step.j,
                     applicable.empty() ? fallback_rule_name(::scfg::NonTerminal::WMBW)
                                        : ::scfg::rule_id_name(applicable.front().rule)});
      continue;
    }
    if (step.state == "WMBP") {
      const auto applicable = ::scfg::applicable_rules_wmbp(step.i, step.j, wmbpctx, tree);
      out.push_back({step.state, step.i, step.j,
                     applicable.empty() ? fallback_rule_name(::scfg::NonTerminal::WMBP)
                                        : ::scfg::rule_id_name(applicable.front().rule)});
      continue;
    }
    if (step.state == "WMB") {
      const auto applicable = ::scfg::applicable_rules_wmb(step.i, step.j, wmbctx, tree);
      out.push_back({step.state, step.i, step.j,
                     applicable.empty() ? fallback_rule_name(::scfg::NonTerminal::WMB)
                                        : ::scfg::rule_id_name(applicable.front().rule)});
      continue;
    }
    if (step.state == "BE") {
      const auto applicable = ::scfg::applicable_rules_be(step.i, step.j, step.i, step.j, bectx, tree);
      out.push_back({step.state, step.i, step.j,
                     applicable.empty() ? fallback_rule_name(::scfg::NonTerminal::BE)
                                        : ::scfg::rule_id_name(applicable.front().rule)});
      continue;
    }
    fail_invalid_input("rules_core slice-d trace hit unexpected state " + step.state);
  }
  return out;
}

EnergyBreakdown structure_energy_breakdown_from_normalized(const NormalizedInput &ctx) {
  auto trace_is_pair_wrapped = [&](const internal::RuleTraceStep &step) {
    if (step.state != "V") {
      return false;
    }
    if (step.i <= 0 || step.j <= 0 || step.i > step.j ||
        static_cast<size_t>(step.i) > ctx.db_full.size()) {
      return false;
    }
    const char left = ctx.db_full[static_cast<size_t>(step.i - 1)];
    return left == '(' || left == '[';
  };

  auto trace_is_unpaired = [&](const internal::RuleTraceStep &step) {
    if (step.state != "V") {
      return false;
    }
    if (step.i <= 0 || step.j <= 0 || step.i > step.j ||
        static_cast<size_t>(step.i) > ctx.db_full.size()) {
      return false;
    }
    return ctx.db_full[static_cast<size_t>(step.i - 1)] == '.';
  };

  EnergyBreakdown breakdown;
  breakdown.topology_family = topology_family_for_structure(ctx.db_full);

  const char *real_score_env = std::getenv("CPARTY_FIXED_ENERGY_REAL_SCORE");
  const bool use_real_score = (real_score_env && *real_score_env != '\0' &&
                               std::string(real_score_env) != "0");

  auto compute_real_score = [&](const std::string &energy_db) -> double {
    ensure_vienna_params_loaded(ctx.seq);
    make_pair_matrix();
    std::unique_ptr<vrna_param_t, void (*)(void *)> params(scale_parameters(), free);
    params->model_details.dangles = 2;
    std::unique_ptr<short, void (*)(void *)> S(encode_sequence(ctx.seq.c_str(), 0), free);
    std::unique_ptr<short, void (*)(void *)> S1(encode_sequence(ctx.seq.c_str(), 1), free);

    const int n = static_cast<int>(energy_db.size());
    sparse_tree tree(energy_db, n);
    const int n1 = n + 1;
    std::vector<double> v_cache(static_cast<size_t>(n1 * n1), INF);
    std::vector<bool> v_done(static_cast<size_t>(n1 * n1), false);
    auto v_index = [&](int i, int j) {
      return static_cast<size_t>(i * n1 + j);
    };

    auto v_energy = [&](auto &&self, int i, int j) -> double {
      if (i >= j || i < 1 || j > n) {
        return INF;
      }
      if (tree.tree[i].pair != j) {
        return INF;
      }
      const size_t idx = v_index(i, j);
      if (v_done[idx]) {
        return v_cache[idx];
      }
      v_done[idx] = true;

      std::vector<std::pair<int, int>> children;
      for (int child : tree.tree[i].children) {
        const int partner = tree.tree[child].pair;
        if (partner > 0 && partner < j) {
          children.emplace_back(child, partner);
        }
      }
      std::sort(children.begin(), children.end());

      const int ptype_closing = pair[S.get()[i]][S.get()[j]];
      if (ptype_closing == 0) {
        v_cache[idx] = INF;
        return v_cache[idx];
      }

      if (children.empty()) {
        v_cache[idx] = E_Hairpin(j - i - 1, ptype_closing, S1.get()[i + 1], S1.get()[j - 1],
                                 &ctx.seq.c_str()[i - 1], params.get());
        return v_cache[idx];
      }

      if (children.size() == 1) {
        const int k = children.front().first;
        const int l = children.front().second;
        const int ptype_inner = pair[S.get()[k]][S.get()[l]];
        const double loop = E_IntLoop(k - i - 1, j - l - 1, ptype_closing, rtype[ptype_inner],
                                      S1.get()[i + 1], S1.get()[j - 1],
                                      S1.get()[k - 1], S1.get()[l + 1], params.get());
        v_cache[idx] = loop + self(self, k, l);
        return v_cache[idx];
      }

      auto ml_stem_energy = [&](int a, int b) -> double {
        const double vij = self(self, a, b);
        const double vi1j = self(self, a + 1, b);
        const double vij1 = self(self, a, b - 1);
        const double vi1j1 = self(self, a + 1, b - 1);

        double e = INF;
        double en = INF;
        pair_type type = pair[S.get()[a]][S.get()[b]];
        if ((tree.tree[a].pair < -1 && tree.tree[b].pair < -1) || (tree.tree[a].pair == b)) {
          en = vij;
          if (en != INF) {
            if (params->model_details.dangles == 2) {
              base_type mm5 = a > 1 ? S.get()[a - 1] : -1;
              base_type mm3 = b < n ? S.get()[b + 1] : -1;
              en += E_MLstem(type, mm5, mm3, params.get());
            } else {
              en += E_MLstem(type, -1, -1, params.get());
            }
            e = std::min(e, en);
          }
        }
        if (params->model_details.dangles == 1) {
          const base_type mm5 = S.get()[a];
          const base_type mm3 = S.get()[b];

          if (((tree.tree[a + 1].pair < -1 && tree.tree[b].pair < -1) || (tree.tree[a + 1].pair == b)) &&
              tree.tree[a].pair < 0) {
            en = (b - a - 1 > TURN) ? vi1j : INF;
            if (en != INF) {
              en += params->MLbase;
              type = pair[S.get()[a + 1]][S.get()[b]];
              en += E_MLstem(type, mm5, -1, params.get());
              e = std::min(e, en);
            }
          }

          if (((tree.tree[a].pair < -1 && tree.tree[b - 1].pair < -1) || (tree.tree[a].pair == b - 1)) &&
              tree.tree[b].pair < 0) {
            en = (b - 1 - a > TURN) ? vij1 : INF;
            if (en != INF) {
              en += params->MLbase;
              type = pair[S.get()[a]][S.get()[b - 1]];
              en += E_MLstem(type, -1, mm3, params.get());
              e = std::min(e, en);
            }
          }

          if (((tree.tree[a + 1].pair < -1 && tree.tree[b - 1].pair < -1) || (tree.tree[a + 1].pair == b - 1)) &&
              tree.tree[a].pair < 0 && tree.tree[b].pair < 0) {
            en = (b - a - 2 > TURN) ? vi1j1 : INF;
            if (en != INF) {
              en += 2 * params->MLbase;
              type = pair[S.get()[a + 1]][S.get()[b - 1]];
              en += E_MLstem(type, mm5, mm3, params.get());
              e = std::min(e, en);
            }
          }
        }

        return e;
      };

      int unpaired = 0;
      int prev = i;
      double total = params->MLclosing;
      for (const auto &child : children) {
        const int k = child.first;
        const int l = child.second;
        unpaired += (k - prev - 1);
        prev = l;
        total += ml_stem_energy(k, l);
        total += self(self, k, l);
      }
      unpaired += (j - prev - 1);
      total += params->MLbase * unpaired;
      v_cache[idx] = total;
      return v_cache[idx];
    };

    auto ext_stem_energy = [&](int i, int j) -> double {
      double e = INF;
      double en = INF;
      pair_type tt = pair[S.get()[i]][S.get()[j]];
      if ((tree.tree[i].pair < -1 && tree.tree[j].pair < -1) || (tree.tree[i].pair == j && tree.tree[j].pair == i)) {
        en = v_energy(v_energy, i, j);
        if (en != INF) {
          if (params->model_details.dangles == 2) {
            base_type si1 = i > 1 ? S.get()[i - 1] : -1;
            base_type sj1 = j < n ? S.get()[j + 1] : -1;
            en += vrna_E_ext_stem(tt, si1, sj1, params.get());
          } else {
            en += vrna_E_ext_stem(tt, -1, -1, params.get());
          }
          e = std::min(e, en);
        }
      }

      if (params->model_details.dangles == 1) {
        tt = pair[S.get()[i + 1]][S.get()[j]];
        if (((tree.tree[i + 1].pair < -1 && tree.tree[j].pair < -1) || (tree.tree[i + 1].pair == j)) &&
            tree.tree[i].pair < 0) {
          en = (j - i - 1 > TURN) ? v_energy(v_energy, i + 1, j) : INF;
          if (en != INF) {
            base_type si1 = S.get()[i];
            en += vrna_E_ext_stem(tt, si1, -1, params.get());
          }
          e = std::min(e, en);
        }

        tt = pair[S.get()[i]][S.get()[j - 1]];
        if (((tree.tree[i].pair < -1 && tree.tree[j - 1].pair < -1) || (tree.tree[i].pair == j - 1)) &&
            tree.tree[j].pair < 0) {
          en = (j - i - 1 > TURN) ? v_energy(v_energy, i, j - 1) : INF;
          if (en != INF) {
            base_type sj1 = S.get()[j];
            en += vrna_E_ext_stem(tt, -1, sj1, params.get());
          }
          e = std::min(e, en);
        }

        tt = pair[S.get()[i + 1]][S.get()[j - 1]];
        if (((tree.tree[i + 1].pair < -1 && tree.tree[j - 1].pair < -1) || (tree.tree[i + 1].pair == j - 1)) &&
            tree.tree[i].pair < 0 && tree.tree[j].pair < 0) {
          en = (j - i - 2 > TURN) ? v_energy(v_energy, i + 1, j - 1) : INF;
          if (en != INF) {
            base_type si1 = S.get()[i];
            base_type sj1 = S.get()[j];
            en += vrna_E_ext_stem(tt, si1, sj1, params.get());
          }
          e = std::min(e, en);
        }
      }
      return e;
    };

    double total_energy = 0.0;
    std::vector<int> roots = tree.tree[0].children;
    std::sort(roots.begin(), roots.end());
    for (int k : roots) {
      const int l = tree.tree[k].pair;
      if (l > 0) {
        total_energy += ext_stem_energy(k, l);
      }
    }
    return total_energy;
  };

  if (use_real_score) {
    if (is_pk_free_structure(ctx.db_full)) {
      breakdown.total_energy = compute_real_score(ctx.db_full);
    } else if (is_h_type_structure(ctx.db_full)) {
      breakdown.total_energy = compute_real_score(normalize_h_type_brackets(ctx.db_full));
    } else {
      std::string round_only = ctx.db_full;
      std::string square_only = ctx.db_full;
      for (size_t i = 0; i < ctx.db_full.size(); ++i) {
        const char c = ctx.db_full[i];
        if (c == '[' || c == ']') {
          round_only[i] = '.';
        } else if (c == '(' || c == ')') {
          square_only[i] = '.';
        }
      }
      breakdown.total_energy =
          compute_real_score(round_only) + compute_real_score(normalize_h_type_brackets(square_only));
    }
  }

  const auto trace = (is_pk_free_structure(ctx.db_full) || is_h_type_structure(ctx.db_full))
                         ? trace_rule_chain_slice_d_rules_core_from_normalized(ctx)
                         : trace_rule_chain_slice_d_shared_from_normalized(ctx);
  for (const auto &step : trace) {
    ++breakdown.rule_evaluated_count;
    if (step.state == "V" && step.i > step.j) {
      ++breakdown.empty_rule_count;
    } else if (trace_is_unpaired(step)) {
      ++breakdown.unpaired_rule_count;
    } else if (trace_is_pair_wrapped(step)) {
      ++breakdown.pair_wrapped_rule_count;
    } else {
      ++breakdown.transition_rule_count;
    }

    if (breakdown.topology_family == "pk_free") {
      ++breakdown.family_pk_free_rules;
    } else if (breakdown.topology_family == "h_type") {
      ++breakdown.family_h_type_rules;
    } else if (breakdown.topology_family == "k_type") {
      ++breakdown.family_k_type_rules;
    }

    if (!use_real_score && trace_is_pair_wrapped(step)) {
      breakdown.total_energy += shared_rule_score(SharedRuleKind::kPairWrapped);
    }
  }

  return breakdown;
}

std::vector<internal::RuleTraceStep> trace_rule_chain_slice_d_from_normalized(const NormalizedInput &ctx) {
  return trace_rule_chain_slice_d_rules_core_from_normalized(ctx);
}

std::vector<internal::RuleTraceStep> trace_rule_chain_zw_only_from_normalized(const NormalizedInput &ctx) {
  std::vector<internal::RuleTraceStep> out;
  const auto slice_a_trace = trace_rule_chain_slice_a_shared_from_normalized(ctx);
  out.reserve(slice_a_trace.size());
  for (const auto &step : slice_a_trace) {
    if (step.state == "V") {
      internal::RuleTraceStep mapped = step;
      mapped.state = "ZW";
      if (step.rule == "V_EMPTY") {
        mapped.rule = "ZW_EMPTY";
      } else if (step.rule == "V_UNPAIRED") {
        mapped.rule = "ZW_UNPAIRED";
      } else if (step.rule == "V_PAIR_WRAPPED") {
        mapped.rule = "ZW_PAIR_WRAPPED";
      } else {
        continue;
      }
      out.push_back(mapped);
    }
  }
  return out;
}

}  // namespace

double get_structure_energy(const std::string &seq, const std::string &db_full) {
  const NormalizedInput normalized = normalize_input(seq, db_full);
  return structure_energy_breakdown_from_normalized(normalized).total_energy;
}

double get_structure_energy_union(const std::string &seq,
                                  const std::string &structure_g,
                                  const std::string &structure_gprime) {
  const NormalizedInput normalized = normalize_union_input(seq, structure_g, structure_gprime);
  return structure_energy_breakdown_from_normalized(normalized).total_energy;
}

namespace internal {

std::vector<RuleTraceStep> trace_rule_chain_zw_only(const std::string &seq,
                                                    const std::string &db_full) {
  return trace_rule_chain_zw_only_from_normalized(normalize_input(seq, db_full));
}

std::vector<RuleTraceStep> trace_rule_chain_slice_a(const std::string &seq,
                                                    const std::string &db_full) {
  return trace_rule_chain_slice_a_from_normalized(normalize_input(seq, db_full));
}

std::vector<RuleTraceStep> trace_rule_chain_slice_a_rules_core(const std::string &seq,
                                                               const std::string &db_full) {
  return trace_rule_chain_slice_a_rules_core_from_normalized(normalize_input(seq, db_full));
}

std::vector<RuleTraceStep> trace_rule_chain_slice_b(const std::string &seq,
                                                    const std::string &db_full) {
  return trace_rule_chain_slice_b_from_normalized(normalize_input(seq, db_full));
}

std::vector<RuleTraceStep> trace_rule_chain_slice_b_rules_core(const std::string &seq,
                                                               const std::string &db_full) {
  return trace_rule_chain_slice_b_rules_core_from_normalized(normalize_input(seq, db_full));
}

std::vector<RuleTraceStep> trace_rule_chain_slice_c(const std::string &seq,
                                                    const std::string &db_full) {
  return trace_rule_chain_slice_c_from_normalized(normalize_input(seq, db_full));
}

std::vector<RuleTraceStep> trace_rule_chain_slice_c_rules_core(const std::string &seq,
                                                               const std::string &db_full) {
  return trace_rule_chain_slice_c_rules_core_from_normalized(normalize_input(seq, db_full));
}

std::vector<RuleTraceStep> trace_rule_chain_slice_d(const std::string &seq,
                                                    const std::string &db_full) {
  return trace_rule_chain_slice_d_from_normalized(normalize_input(seq, db_full));
}

std::vector<RuleTraceStep> trace_rule_chain_slice_d_rules_core(const std::string &seq,
                                                               const std::string &db_full) {
  return trace_rule_chain_slice_d_rules_core_from_normalized(normalize_input(seq, db_full));
}

EnergyBreakdown get_structure_energy_breakdown(const std::string &seq,
                                               const std::string &db_full) {
  return structure_energy_breakdown_from_normalized(normalize_input(seq, db_full));
}

const std::vector<std::string> &fixed_energy_target_states() {
  static const std::vector<std::string> kStates = {
      "W", "WI", "V", "VM", "WM", "WMv", "WMp", "WIP",
      "VP", "VPL", "VPR", "WMB", "WMBP", "WMBW", "BE", "ZW",
  };
  return kStates;
}

const std::vector<RolloutStatePlanEntry> &fixed_energy_rollout_plan() {
  static const std::vector<RolloutStatePlanEntry> kPlan = {
      {"W", "014"},   {"WI", "014"},  {"V", "014"},    {"VM", "015"},
      {"WM", "015"},  {"WMv", "015"}, {"WMp", "015"},  {"WIP", "016"},
      {"VP", "016"},  {"VPL", "016"}, {"VPR", "016"},  {"WMB", "017"},
      {"WMBP", "017"},{"WMBW", "017"},{"BE", "017"},   {"ZW", "013"},
  };
  return kPlan;
}

}  // namespace internal

}  // namespace cparty
