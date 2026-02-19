#include "scfg/rules_core.hh"

#include "scfg/constraint_oracle.hh"
#include "scfg/legacy_adapter.hh"
#include "scfg/rules_part_helpers.hh"
#include "scfg/structure_view.hh"
#include "sparse_tree.hh"

namespace scfg {

bool span_predicate_allows(const RuleSpec &spec, const RuleSpanContext &ctx, sparse_tree &tree) {
    switch (spec.span_predicate) {
    case RuleSpec::SpanPredicateKind::None:
        return true;
    case RuleSpec::SpanPredicateKind::BeSpanValid:
        return ctx.i >= 1 && ctx.i <= ctx.ip && ctx.ip < ctx.jp && ctx.jp <= ctx.j && ctx.j <= tree.n &&
               tree.tree[ctx.i].pair > 0 && tree.tree[ctx.j].pair > 0 &&
               tree.tree[ctx.ip].pair > 0 && tree.tree[ctx.jp].pair > 0 &&
               tree.tree[ctx.i].pair == ctx.j && tree.tree[ctx.j].pair == ctx.i &&
               tree.tree[ctx.ip].pair == ctx.jp && tree.tree[ctx.jp].pair == ctx.ip;
    default:
        return true;
    }
}

bool span_predicate_allows(const RuleSpec &spec, const RuleSpanContext &ctx, const StructureView &view) {
    switch (spec.span_predicate) {
    case RuleSpec::SpanPredicateKind::None:
        return true;
    case RuleSpec::SpanPredicateKind::BeSpanValid:
        return ctx.i >= 1 && ctx.i <= ctx.ip && ctx.ip < ctx.jp && ctx.jp <= ctx.j && ctx.j <= view.n() &&
               view.is_pair_square(ctx.i, ctx.j) && view.is_pair_square(ctx.ip, ctx.jp);
    default:
        return true;
    }
}

bool predicate_allows(const RuleSpec &spec, const RuleSpanContext &ctx, sparse_tree &tree) {
    switch (spec.predicate) {
    case RuleSpec::PredicateKind::None:
        return true;
    case RuleSpec::PredicateKind::UnpairedAtJ:
        return tree.tree[ctx.j].pair < 0;
    case RuleSpec::PredicateKind::UnpairedAtJMinus1:
        return ctx.j > 0 && tree.tree[ctx.j - 1].pair < 0;
    case RuleSpec::PredicateKind::VPairingState: {
        const bool unpaired = (tree.tree[ctx.i].pair < -1 && tree.tree[ctx.j].pair < -1);
        const bool paired = (tree.tree[ctx.i].pair == ctx.j && tree.tree[ctx.j].pair == ctx.i);
        return paired || unpaired;
    }
    case RuleSpec::PredicateKind::VPairingStateAndHairpinMinLoop: {
        const bool unpaired = (tree.tree[ctx.i].pair < -1 && tree.tree[ctx.j].pair < -1);
        const bool paired = (tree.tree[ctx.i].pair == ctx.j && tree.tree[ctx.j].pair == ctx.i);
        const bool canH = !(tree.up[ctx.j - 1] < (ctx.j - ctx.i - 1));
        return (paired || unpaired) && canH;
    }
    case RuleSpec::PredicateKind::WSpanWeaklyClosed:
        return tree.weakly_closed(1, ctx.j);
    case RuleSpec::PredicateKind::WmbSplitBeWmbpWi:
        return tree.tree[ctx.j].pair >= 0 && ctx.j > tree.tree[ctx.j].pair && tree.tree[ctx.j].pair > ctx.i;
    case RuleSpec::PredicateKind::WmbwSplitWmbpWi: {
        if (!(tree.tree[ctx.j].pair < ctx.j)) return false;
        const Node *parent_j = tree.tree[ctx.j].parent;
        const Node *parent_l = tree.tree[ctx.split.k].parent;
        if (!parent_j || !parent_l) return false;
        return tree.tree[ctx.split.k].pair < 0 && parent_j->index > -1 && parent_j->index == parent_l->index;
    }
    case RuleSpec::PredicateKind::BeBaseSamePair:
        return ctx.i == ctx.ip && ctx.j == ctx.jp && ctx.i < ctx.j;
    case RuleSpec::PredicateKind::BeStackPairing:
        return tree.tree[ctx.i + 1].pair == ctx.j - 1;
    case RuleSpec::PredicateKind::BeInternalLoop:
        return scfg::is_empty_region(tree, ctx.i, ctx.split.k) &&
               scfg::is_empty_region(tree, ctx.split.l, ctx.j);
    case RuleSpec::PredicateKind::BeWipWip:
        return tree.weakly_closed(ctx.i + 1, ctx.split.k - 1) &&
               tree.weakly_closed(ctx.split.l + 1, ctx.j - 1);
    case RuleSpec::PredicateKind::BeWipBasepair:
        return tree.weakly_closed(ctx.i + 1, ctx.split.k - 1) &&
               scfg::is_empty_region(tree, ctx.split.l, ctx.j);
    case RuleSpec::PredicateKind::BeBasepairWip:
        return scfg::is_empty_region(tree, ctx.i, ctx.split.k) &&
               tree.weakly_closed(ctx.split.l + 1, ctx.j - 1);
    case RuleSpec::PredicateKind::VprBasepair:
        return scfg::can_pair_right_span(tree, ctx.split.k, ctx.j);
    case RuleSpec::PredicateKind::VpWiCase1:
        return tree.tree[ctx.i].parent->index > 0 && tree.tree[ctx.j].parent->index < tree.tree[ctx.i].parent->index &&
               tree.Bp(ctx.i, ctx.j) >= 0 && tree.B(ctx.i, ctx.j) >= 0 && tree.bp(ctx.i, ctx.j) < 0;
    case RuleSpec::PredicateKind::VpWiCase2:
        return tree.tree[ctx.i].parent->index < tree.tree[ctx.j].parent->index && tree.tree[ctx.j].parent->index > 0 &&
               tree.b(ctx.i, ctx.j) >= 0 && tree.bp(ctx.i, ctx.j) >= 0 && tree.Bp(ctx.i, ctx.j) < 0;
    case RuleSpec::PredicateKind::VpWiCase3:
        return tree.tree[ctx.i].parent->index > 0 && tree.tree[ctx.j].parent->index > 0 &&
               tree.Bp(ctx.i, ctx.j) >= 0 && tree.B(ctx.i, ctx.j) >= 0 && tree.b(ctx.i, ctx.j) >= 0 &&
               tree.bp(ctx.i, ctx.j) >= 0;
    case RuleSpec::PredicateKind::VpStackPairing:
        return tree.tree[ctx.i + 1].pair < -1 && tree.tree[ctx.j - 1].pair < -1 &&
               scfg::is_pair_type_allowed(ctx.pair_type_ip1jm1);
    case RuleSpec::PredicateKind::VpInternalLoopSplit:
        if (ctx.split.k == ctx.i + 1 && ctx.split.l == ctx.j - 1) return false;
        return scfg::is_pair_type_allowed(ctx.pair_type_kl) &&
               scfg::is_unpaired_position(tree, ctx.split.k) &&
               scfg::is_unpaired_position(tree, ctx.split.l) &&
               scfg::is_empty_region(tree, ctx.i, ctx.split.k) &&
               scfg::is_empty_region(tree, ctx.split.l, ctx.j);
    default:
        return true;
    }
}

bool predicate_allows(const RuleSpec &spec, const RuleSpanContext &ctx, const StructureView &view) {
    switch (spec.predicate) {
    case RuleSpec::PredicateKind::None:
        return true;
    case RuleSpec::PredicateKind::UnpairedAtJ:
        return view.is_unpaired(ctx.j);
    case RuleSpec::PredicateKind::UnpairedAtJMinus1:
        return ctx.j > 0 && view.is_unpaired(ctx.j - 1);
    case RuleSpec::PredicateKind::VPairingState:
        return true;
    case RuleSpec::PredicateKind::VPairingStateAndHairpinMinLoop:
        return view.unpaired_prefix(ctx.j - 1) >= (ctx.j - ctx.i - 1);
    case RuleSpec::PredicateKind::WmbSplitBeWmbpWi:
        return view.pair_square(ctx.j) >= 0 && ctx.j > view.pair_square(ctx.j) && view.pair_square(ctx.j) > ctx.i;
    case RuleSpec::PredicateKind::WmbwSplitWmbpWi:
        return view.pair_square(ctx.j) < ctx.j && view.is_unpaired(ctx.split.k) && view.parent_index(ctx.j) > -1 &&
               view.parent_index(ctx.j) == view.parent_index(ctx.split.k);
    case RuleSpec::PredicateKind::BeBaseSamePair:
        return ctx.i == ctx.ip && ctx.j == ctx.jp && ctx.i < ctx.j;
    case RuleSpec::PredicateKind::BeStackPairing:
        return view.is_pair_square(ctx.i + 1, ctx.j - 1);
    case RuleSpec::PredicateKind::BeInternalLoop:
        return view.is_empty_region(ctx.i, ctx.split.k) && view.is_empty_region(ctx.split.l, ctx.j);
    case RuleSpec::PredicateKind::BeWipWip:
        return view.weakly_closed(ctx.i + 1, ctx.split.k - 1) &&
               view.weakly_closed(ctx.split.l + 1, ctx.j - 1);
    case RuleSpec::PredicateKind::BeWipBasepair:
        return view.weakly_closed(ctx.i + 1, ctx.split.k - 1) && view.is_empty_region(ctx.split.l, ctx.j);
    case RuleSpec::PredicateKind::BeBasepairWip:
        return view.is_empty_region(ctx.i, ctx.split.k) && view.weakly_closed(ctx.split.l + 1, ctx.j - 1);
    case RuleSpec::PredicateKind::VprBasepair:
        return view.can_pair_right_span(ctx.split.k, ctx.j);
    case RuleSpec::PredicateKind::VpWiCase1:
        return view.parent_index(ctx.i) > 0 && view.parent_index(ctx.j) < view.parent_index(ctx.i) &&
               view.Bp(ctx.i, ctx.j) >= 0 && view.B(ctx.i, ctx.j) >= 0 && view.bp(ctx.i, ctx.j) < 0;
    case RuleSpec::PredicateKind::VpWiCase2:
        return view.parent_index(ctx.i) < view.parent_index(ctx.j) && view.parent_index(ctx.j) > 0 &&
               view.b(ctx.i, ctx.j) >= 0 && view.bp(ctx.i, ctx.j) >= 0 && view.Bp(ctx.i, ctx.j) < 0;
    case RuleSpec::PredicateKind::VpWiCase3:
        return view.parent_index(ctx.i) > 0 && view.parent_index(ctx.j) > 0 && view.Bp(ctx.i, ctx.j) >= 0 &&
               view.B(ctx.i, ctx.j) >= 0 && view.b(ctx.i, ctx.j) >= 0 && view.bp(ctx.i, ctx.j) >= 0;
    case RuleSpec::PredicateKind::VpStackPairing:
        return view.is_unpaired(ctx.i + 1) && view.is_unpaired(ctx.j - 1) &&
               scfg::is_pair_type_allowed(ctx.pair_type_ip1jm1);
    case RuleSpec::PredicateKind::VpInternalLoopSplit:
        if (ctx.split.k == ctx.i + 1 && ctx.split.l == ctx.j - 1) return false;
        return scfg::is_pair_type_allowed(ctx.pair_type_kl) &&
               view.is_unpaired(ctx.split.k) &&
               view.is_unpaired(ctx.split.l) &&
               view.is_empty_region(ctx.i, ctx.split.k) &&
               view.is_empty_region(ctx.split.l, ctx.j);
    default:
        return true;
    }
}

bool predicate_allows(const RuleSpec &spec, const RuleSpanContext &ctx, const PartFuncRuleHelpers &rules) {
    switch (spec.predicate) {
    case RuleSpec::PredicateKind::None:
        return true;
    case RuleSpec::PredicateKind::WmbpJUnpaired:
        return rules.pair_at(ctx.j) < 0;
    case RuleSpec::PredicateKind::WmbpJUnpairedIpaired:
        return rules.pair_at(ctx.j) < 0 && rules.pair_at(ctx.i) >= 0;
    default:
        return true;
    }
}

bool predicate_allows(const RuleSpec &spec, const RuleSpanContext &ctx, const PartFuncRuleHelpersView &rules) {
    switch (spec.predicate) {
    case RuleSpec::PredicateKind::None:
        return true;
    case RuleSpec::PredicateKind::WmbpJUnpaired:
        return rules.pair_at(ctx.j) < 0;
    case RuleSpec::PredicateKind::WmbpJUnpairedIpaired:
        return rules.pair_at(ctx.j) < 0 && rules.pair_at(ctx.i) >= 0;
    default:
        return true;
    }
}

bool predicate_allows(const RuleSpec &spec, const RuleSpanContext &ctx, const std::vector<Node> &tree) {
    switch (spec.predicate) {
    case RuleSpec::PredicateKind::None:
        return true;
    case RuleSpec::PredicateKind::UnpairedAtJ:
        return tree[ctx.j].pair < 0;
    case RuleSpec::PredicateKind::UnpairedAtJMinus1:
        return ctx.j > 0 && tree[ctx.j - 1].pair < 0;
    case RuleSpec::PredicateKind::WSpanWeaklyClosed:
        return true;
    default:
        return true;
    }
}

} // namespace scfg
