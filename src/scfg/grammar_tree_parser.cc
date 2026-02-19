#include "scfg/grammar_tree_parser.hh"

#include "fixed_energy_api.hh"
#include "fixed_energy_input.hh"
#include "scfg/rules_core.hh"
#include "scfg/rules_core_stub_contexts.hh"
#include "sparse_tree.hh"

#include <stdexcept>
#include <vector>

namespace cparty::scfg {
namespace {

std::string nonterminal_name(const ::scfg::NonTerminal nonterminal) {
  switch (nonterminal) {
    case ::scfg::NonTerminal::W:
      return "W";
    case ::scfg::NonTerminal::WI:
      return "WI";
    case ::scfg::NonTerminal::V:
      return "V";
    case ::scfg::NonTerminal::VM:
      return "VM";
    case ::scfg::NonTerminal::WM:
      return "WM";
    case ::scfg::NonTerminal::WMv:
      return "WMv";
    case ::scfg::NonTerminal::WMp:
      return "WMp";
    case ::scfg::NonTerminal::WIP:
      return "WIP";
    case ::scfg::NonTerminal::VP:
      return "VP";
    case ::scfg::NonTerminal::VPL:
      return "VPL";
    case ::scfg::NonTerminal::VPR:
      return "VPR";
    case ::scfg::NonTerminal::WMB:
      return "WMB";
    case ::scfg::NonTerminal::WMBP:
      return "WMBP";
    case ::scfg::NonTerminal::WMBW:
      return "WMBW";
    case ::scfg::NonTerminal::BE:
      return "BE";
    default:
      break;
  }
  return "UNKNOWN";
}

std::vector<RuleTraceStep> trace_rules_core_from_normalized(const NormalizedInput &ctx) {
  if (!is_pk_free_structure(ctx.db_full) && !is_h_type_structure(ctx.db_full)) {
    fail_invalid_input("rules_core parse requires pk_free or h_type structure");
  }
  const std::string tree_db =
      is_h_type_structure(ctx.db_full) ? normalize_h_type_brackets(ctx.db_full) : ctx.db_full;
  const int n = static_cast<int>(tree_db.size());
  sparse_tree tree(tree_db, n);

  detail::RuleCoreStubWContext wctx(n);
  detail::RuleCoreStubWIContext wictx;
  detail::RuleCoreStubVContext vctx;
  detail::RuleCoreStubVMContext vmctx;
  detail::RuleCoreStubWMvWMpContext wmvwmpctx;
  detail::RuleCoreStubWMContext wmctx;
  detail::RuleCoreStubWIPContext wipctx;
  detail::RuleCoreStubVPLContext vplctx;
  detail::RuleCoreStubVPRContext vprctx;
  detail::RuleCoreStubVPContext vpctx(ctx.seq);
  detail::RuleCoreStubWMBWContext wmbwctx;
  detail::RuleCoreStubWMBPContext wmbpctx(n);
  detail::RuleCoreStubWMBContext wmbctx(n);
  detail::RuleCoreStubBEContext bectx(n);

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

  struct ParseItem {
    ::scfg::NonTerminal nt;
    int i = 0;
    int j = 0;
  };

  std::vector<ParseItem> stack;
  stack.push_back(ParseItem{::scfg::NonTerminal::W, 1, n});
  std::vector<RuleTraceStep> trace;

  while (!stack.empty()) {
    const ParseItem cur = stack.back();
    stack.pop_back();
    if (cur.i > cur.j) {
      continue;
    }

    std::vector<::scfg::ApplicableRule> applicable;
    switch (cur.nt) {
      case ::scfg::NonTerminal::W:
        applicable = ::scfg::applicable_rules_w(cur.i, cur.j, wctx, tree);
        break;
      case ::scfg::NonTerminal::WI:
        applicable = ::scfg::applicable_rules_wi(cur.i, cur.j, wictx, tree);
        break;
      case ::scfg::NonTerminal::V:
        applicable = ::scfg::applicable_rules_v(cur.i, cur.j, vctx, tree);
        break;
      case ::scfg::NonTerminal::VM:
        applicable = ::scfg::applicable_rules_vm(cur.i, cur.j, vmctx, tree.up);
        break;
      case ::scfg::NonTerminal::WMv:
        applicable = wmv_wmp_rules(::scfg::NonTerminal::WMv, cur.i, cur.j);
        break;
      case ::scfg::NonTerminal::WMp:
        applicable = wmv_wmp_rules(::scfg::NonTerminal::WMp, cur.i, cur.j);
        break;
      case ::scfg::NonTerminal::WM:
        applicable = ::scfg::applicable_rules_wm(cur.i, cur.j, wmctx, tree);
        break;
      case ::scfg::NonTerminal::WIP:
        applicable = ::scfg::applicable_rules_wip(cur.i, cur.j, wipctx, tree);
        break;
      case ::scfg::NonTerminal::VPL:
        applicable = ::scfg::applicable_rules_vpl(cur.i, cur.j, vplctx, tree);
        break;
      case ::scfg::NonTerminal::VPR:
        applicable = ::scfg::applicable_rules_vpr(cur.i, cur.j, vprctx, tree);
        break;
      case ::scfg::NonTerminal::VP:
        applicable = ::scfg::applicable_rules_vp(cur.i, cur.j, vpctx, tree);
        break;
      case ::scfg::NonTerminal::WMBW:
        applicable = ::scfg::applicable_rules_wmbw(cur.i, cur.j, wmbwctx, tree);
        break;
      case ::scfg::NonTerminal::WMBP:
        applicable = ::scfg::applicable_rules_wmbp(cur.i, cur.j, wmbpctx, tree);
        break;
      case ::scfg::NonTerminal::WMB:
        applicable = ::scfg::applicable_rules_wmb(cur.i, cur.j, wmbctx, tree);
        break;
      case ::scfg::NonTerminal::BE:
        applicable = ::scfg::applicable_rules_be(cur.i, cur.j, cur.i, cur.j, bectx, tree);
        break;
      default:
        fail_invalid_input("rules_core parse hit unexpected nonterminal " + nonterminal_name(cur.nt));
    }

    if (applicable.size() != 1) {
      fail_invalid_input("rules_core parse could not select unique rule for " + nonterminal_name(cur.nt) + "[" +
                         std::to_string(cur.i) + "," + std::to_string(cur.j) + "] with candidates=" +
                         std::to_string(applicable.size()));
    }

    const ::scfg::ApplicableRule selected = applicable.front();
    trace.push_back(RuleTraceStep{nonterminal_name(cur.nt), cur.i, cur.j, ::scfg::rule_id_name(selected.rule)});

    std::vector<::scfg::RuleChild> children;
    switch (cur.nt) {
      case ::scfg::NonTerminal::W:
        children = ::scfg::expand_w(selected.rule, cur.i, cur.j, selected.split);
        break;
      case ::scfg::NonTerminal::WI:
        children = ::scfg::expand_wi(selected.rule, cur.i, cur.j, selected.split);
        break;
      case ::scfg::NonTerminal::V:
        children = ::scfg::expand_v(selected.rule, cur.i, cur.j, selected.split);
        break;
      case ::scfg::NonTerminal::VM:
        children = ::scfg::expand_vm(selected.rule, cur.i, cur.j, selected.split);
        break;
      case ::scfg::NonTerminal::WMv:
      case ::scfg::NonTerminal::WMp:
        children = ::scfg::expand_wmv_wmp(selected.rule, cur.i, cur.j, selected.split);
        break;
      case ::scfg::NonTerminal::WM:
        children = ::scfg::expand_wm(selected.rule, cur.i, cur.j, selected.split);
        break;
      case ::scfg::NonTerminal::WIP:
        children = ::scfg::expand_wip(selected.rule, cur.i, cur.j, selected.split);
        break;
      case ::scfg::NonTerminal::VPL:
        children = ::scfg::expand_vpl(selected.rule, cur.i, cur.j, selected.split);
        break;
      case ::scfg::NonTerminal::VPR:
        children = ::scfg::expand_vpr(selected.rule, cur.i, cur.j, selected.split);
        break;
      case ::scfg::NonTerminal::VP:
        children = ::scfg::expand_vp(selected.rule, cur.i, cur.j, selected.split);
        break;
      case ::scfg::NonTerminal::WMBW:
        children = ::scfg::expand_wmbw(selected.rule, cur.i, cur.j, selected.split);
        break;
      case ::scfg::NonTerminal::WMBP:
        children = ::scfg::expand_wmbp(selected.rule, cur.i, cur.j, selected.split);
        break;
      case ::scfg::NonTerminal::WMB:
        children = ::scfg::expand_wmb(selected.rule, cur.i, cur.j, selected.split);
        break;
      case ::scfg::NonTerminal::BE:
        children = ::scfg::expand_be(selected.rule, cur.i, cur.j, cur.i, cur.j, selected.split);
        break;
      default:
        break;
    }

    for (auto it = children.rbegin(); it != children.rend(); ++it) {
      if (it->i > it->j) {
        continue;
      }
      stack.push_back(ParseItem{it->nonterminal, it->i, it->j});
    }
  }

  return trace;
}

}  // namespace

ParseResult parse_fixed_energy(const std::string &seq,
                               const std::string &structure_g,
                               const std::string &structure_gprime,
                               const ParseOptions &options) {
  ParseResult result{};
  try {
    const NormalizedInput normalized = normalize_union_input(seq, structure_g, structure_gprime);
    result.total_energy = get_structure_energy_union(seq, structure_g, structure_gprime);
    if (options.return_trace) {
      if (options.require_rules_core) {
        result.trace = trace_rules_core_from_normalized(normalized);
      }
    }
  } catch (const std::exception &err) {
    result.ok = false;
    result.error = err.what();
  }
  return result;
}

}  // namespace cparty::scfg
