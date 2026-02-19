#include "scfg/rules_core.hh"

namespace scfg {

cand_pos_t resolve_endpoint(EndpointRef endpoint, const RuleSpanContext &ctx) {
    switch (endpoint.base) {
    case Endpoint::I:
        return ctx.i + endpoint.offset;
    case Endpoint::J:
        return ctx.j + endpoint.offset;
    case Endpoint::K:
        return ctx.split.k + endpoint.offset;
    case Endpoint::L:
        return ctx.split.l + endpoint.offset;
    case Endpoint::P:
        return ctx.split.p + endpoint.offset;
    case Endpoint::Q:
        return ctx.split.q + endpoint.offset;
    case Endpoint::IP:
        return ctx.ip + endpoint.offset;
    case Endpoint::JP:
        return ctx.jp + endpoint.offset;
    }
    return -1;
}

std::vector<RuleChild> expand_rule_rhs(const RuleSpec &spec, const RuleSpanContext &ctx) {
    std::vector<RuleChild> children;
    children.reserve(spec.rhs_len);
    for (size_t idx = 0; idx < spec.rhs_len; ++idx) {
        const RuleChildSpec &child_spec = spec.rhs[idx];
        const cand_pos_t left = resolve_endpoint(child_spec.span.left, ctx);
        const cand_pos_t right = resolve_endpoint(child_spec.span.right, ctx);
        if (!child_spec.allow_empty && left > right) continue;
        children.push_back({child_spec.nonterminal, left, right});
    }
    return children;
}

} // namespace scfg
