#include "scfg/rules_core.hh"

#include "scfg/structure_view.hh"
#include "sparse_tree.hh"

namespace scfg {

bool predicate_allows(const RuleSpec &spec, const RuleSpanContext &ctx, const sparse_tree &tree) {
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
    default:
        return true;
    }
}

} // namespace scfg
