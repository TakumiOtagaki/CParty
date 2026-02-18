#include "scfg/rules_part_helpers.hh"

#include "scfg/legacy_adapter.hh"
#include "scfg/constraint_oracle.hh"

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <ViennaRNA/params/constants.h>

namespace scfg {

PartFuncRuleHelpers::PartFuncRuleHelpers(sparse_tree &tree, const PartFuncModeConfig &config) : tree_(tree), config_(config) {}

cand_pos_t PartFuncRuleHelpers::border_b(cand_pos_t i, cand_pos_t j) { return tree_.b(i, j); }

cand_pos_t PartFuncRuleHelpers::border_bp(cand_pos_t i, cand_pos_t j) { return tree_.bp(i, j); }

cand_pos_t PartFuncRuleHelpers::border_B(cand_pos_t i, cand_pos_t j) { return tree_.B(i, j); }

cand_pos_t PartFuncRuleHelpers::border_Bp(cand_pos_t i, cand_pos_t j) { return tree_.Bp(i, j); }

cand_pos_t PartFuncRuleHelpers::pair_at(cand_pos_t pos) const { return tree_.tree[pos].pair; }

cand_pos_t PartFuncRuleHelpers::parent_index(cand_pos_t pos) const { return tree_.tree[pos].parent->index; }

bool PartFuncRuleHelpers::allow_exterior_split(cand_pos_t l, cand_pos_t j, cand_pos_t b_ij, int exterior_case) const {
    (void)j;
    return (b_ij > 0 && l < b_ij) || (b_ij < 0 && exterior_case == 0);
}

bool PartFuncRuleHelpers::has_valid_band_borders(cand_pos_t i, cand_pos_t l, cand_pos_t j) {
    const cand_pos_t bp_il = border_bp(i, l);
    const cand_pos_t Bp_lj = border_Bp(l, j);
    return bp_il >= 0 && l > bp_il && Bp_lj > 0 && l < Bp_lj;
}

bool PartFuncRuleHelpers::parent_within_interval_and_turn(cand_pos_t i, cand_pos_t l, cand_pos_t j) const {
    return i <= parent_index(l) && parent_index(l) < j && l + config_.turn <= j;
}

bool PartFuncRuleHelpers::has_valid_inner_arc_split(cand_pos_t i, cand_pos_t l, cand_pos_t j, cand_pos_t n_bound) {
    const cand_pos_t bp_il = border_bp(i, l);
    return bp_il >= 0 && bp_il < n_bound && l + config_.turn <= j;
}

pf_t PartFuncRuleHelpers::apply_double_pb_penalty(pf_t value) const { return value * std::pow(config_.exp_pb_penalty, 2); }

void PartFuncRuleHelpers::on_traceback_hook(cand_pos_t i, cand_pos_t j) const {
    (void)i;
    (void)j;
}

void PartFuncRuleHelpers::on_fixed_parse_hook(cand_pos_t i, cand_pos_t j) const {
    (void)i;
    (void)j;
}

PartFuncRuleHelpersView::PartFuncRuleHelpersView(const StructureView &view, const PartFuncModeConfig &config)
    : view_(view), config_(config) {}

cand_pos_t PartFuncRuleHelpersView::border_b(cand_pos_t i, cand_pos_t j) const { return view_.b(i, j); }

cand_pos_t PartFuncRuleHelpersView::border_bp(cand_pos_t i, cand_pos_t j) const { return view_.bp(i, j); }

cand_pos_t PartFuncRuleHelpersView::border_B(cand_pos_t i, cand_pos_t j) const { return view_.B(i, j); }

cand_pos_t PartFuncRuleHelpersView::border_Bp(cand_pos_t i, cand_pos_t j) const { return view_.Bp(i, j); }

cand_pos_t PartFuncRuleHelpersView::pair_at(cand_pos_t pos) const { return view_.pair_square(pos); }

cand_pos_t PartFuncRuleHelpersView::parent_index(cand_pos_t pos) const { return view_.parent_index(pos); }

bool PartFuncRuleHelpersView::allow_exterior_split(cand_pos_t l, cand_pos_t j, cand_pos_t b_ij, int exterior_case) const {
    (void)j;
    return (b_ij > 0 && l < b_ij) || (b_ij < 0 && exterior_case == 0);
}

bool PartFuncRuleHelpersView::has_valid_band_borders(cand_pos_t i, cand_pos_t l, cand_pos_t j) const {
    const cand_pos_t bp_il = border_bp(i, l);
    const cand_pos_t Bp_lj = border_Bp(l, j);
    return bp_il >= 0 && l > bp_il && Bp_lj > 0 && l < Bp_lj;
}

bool PartFuncRuleHelpersView::parent_within_interval_and_turn(cand_pos_t i, cand_pos_t l, cand_pos_t j) const {
    const cand_pos_t parent = parent_index(l);
    return i <= parent && parent < j && l + config_.turn <= j;
}

bool PartFuncRuleHelpersView::has_valid_inner_arc_split(cand_pos_t i, cand_pos_t l, cand_pos_t j, cand_pos_t n_bound) const {
    const cand_pos_t bp_il = border_bp(i, l);
    return bp_il >= 0 && bp_il < n_bound && l + config_.turn <= j;
}

pf_t PartFuncRuleHelpersView::apply_double_pb_penalty(pf_t value) const { return value * std::pow(config_.exp_pb_penalty, 2); }

void PartFuncRuleHelpersView::on_traceback_hook(cand_pos_t i, cand_pos_t j) const {
    (void)i;
    (void)j;
}

void PartFuncRuleHelpersView::on_fixed_parse_hook(cand_pos_t i, cand_pos_t j) const {
    (void)i;
    (void)j;
}


} // namespace scfg
