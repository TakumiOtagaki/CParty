#include "scfg/rules_core.hh"

#include "scfg/rules_part_helpers.hh"
#include "scfg/structure_view.hh"
#include "sparse_tree.hh"

namespace scfg {

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
    case RuleSpec::PredicateKind::WmbSplitBeWmbpWi:
        return tree.tree[ctx.j].pair >= 0 && ctx.j > tree.tree[ctx.j].pair && tree.tree[ctx.j].pair > ctx.i;
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
    case RuleSpec::PredicateKind::WmbSplitBeWmbpWi:
        return view.pair_square(ctx.j) >= 0 && ctx.j > view.pair_square(ctx.j) && view.pair_square(ctx.j) > ctx.i;
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

} // namespace scfg
