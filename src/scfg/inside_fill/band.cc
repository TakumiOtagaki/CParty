#include "scfg/inside_fill.hh"

#include "scfg/constraint_oracle.hh"
#include "scfg/legacy_adapter.hh"
#include "scfg/rules_core.hh"
#include "scfg/rules_part_func.hh"
#include "scfg/structure_view.hh"

#include <ViennaRNA/params/constants.h>

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <iostream>

namespace scfg {

void compute_WIP_restricted_rules(PartFuncWIPContext &ctx, cand_pos_t i, cand_pos_t j, sparse_tree &tree, const RulesConfig &config) {
    const cand_pos_t ij = ctx.index_of(i, j);
    pf_t contributions = 0;
    if (config.use_applicable) {
        const auto applicable = applicable_rules_wip(i, j, ctx, tree);
        for (const auto &entry : applicable) {
            if (!is_rule_enabled(config, entry.rule)) continue;
            record_rule_hit(entry.rule);
            pf_t coeff = rule_score_wip(entry.rule, i, j, entry.split, ctx);
            pf_t term = 1;
            const auto children = expand_wip(entry.rule, i, j, entry.split);
            for (const auto &child : children) {
                if (child.nonterminal == NonTerminal::WIP) {
                    term *= ctx.get_energy_WIP(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::V) {
                    term *= ctx.get_energy(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::WMB) {
                    term *= ctx.get_energy_WMB(child.i, child.j);
                }
            }
            contributions += term * coeff;
        }
        ctx.set_WIP(ij, contributions);
        return;
    }
    for (RuleId rule : rules_for(NonTerminal::WIP)) {
        if (!is_rule_enabled(config, rule)) continue;
        const auto splits = enumerate_splits_wip(rule, i, j, ctx, tree);
        for (const auto &split : splits) {
            record_rule_hit(rule);
            pf_t coeff = rule_score_wip(rule, i, j, split, ctx);
            pf_t term = 1;
            const auto children = expand_wip(rule, i, j, split);
            for (const auto &child : children) {
                if (child.nonterminal == NonTerminal::WIP) {
                    term *= ctx.get_energy_WIP(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::V) {
                    term *= ctx.get_energy(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::WMB) {
                    term *= ctx.get_energy_WMB(child.i, child.j);
                }
            }
            contributions += term * coeff;
        }
    }
    ctx.set_WIP(ij, contributions);
}

void compute_WIP_restricted_rules(PartFuncWIPContext &ctx,
                                  cand_pos_t i,
                                  cand_pos_t j,
                                  const StructureView &view,
                                  const RulesConfig &config) {
    const cand_pos_t ij = ctx.index_of(i, j);
    pf_t contributions = 0;
    if (config.use_applicable) {
        const auto applicable = applicable_rules_wip(i, j, ctx, view);
        for (const auto &entry : applicable) {
            if (!is_rule_enabled(config, entry.rule)) continue;
            record_rule_hit(entry.rule);
            pf_t coeff = rule_score_wip(entry.rule, i, j, entry.split, ctx);
            pf_t term = 1;
            const auto children = expand_wip(entry.rule, i, j, entry.split);
            for (const auto &child : children) {
                if (child.nonterminal == NonTerminal::WIP) {
                    term *= ctx.get_energy_WIP(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::V) {
                    term *= ctx.get_energy(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::WMB) {
                    term *= ctx.get_energy_WMB(child.i, child.j);
                }
            }
            contributions += term * coeff;
        }
        ctx.set_WIP(ij, contributions);
        return;
    }
    for (RuleId rule : rules_for(NonTerminal::WIP)) {
        if (!is_rule_enabled(config, rule)) continue;
        const auto splits = enumerate_splits_wip(rule, i, j, ctx, view);
        for (const auto &split : splits) {
            record_rule_hit(rule);
            pf_t coeff = rule_score_wip(rule, i, j, split, ctx);
            pf_t term = 1;
            const auto children = expand_wip(rule, i, j, split);
            for (const auto &child : children) {
                if (child.nonterminal == NonTerminal::WIP) {
                    term *= ctx.get_energy_WIP(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::V) {
                    term *= ctx.get_energy(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::WMB) {
                    term *= ctx.get_energy_WMB(child.i, child.j);
                }
            }
            contributions += term * coeff;
        }
    }
    ctx.set_WIP(ij, contributions);
}

void compute_VPL_restricted_rules(PartFuncVPLContext &ctx, cand_pos_t i, cand_pos_t j, sparse_tree &tree, const RulesConfig &config) {
    const cand_pos_t ij = ctx.index_of(i, j);
    pf_t contributions = 0;
    if (config.use_applicable) {
        const auto applicable = applicable_rules_vpl(i, j, ctx, tree);
        for (const auto &entry : applicable) {
            if (!is_rule_enabled(config, entry.rule)) continue;
            record_rule_hit(entry.rule);
            pf_t coeff = rule_score_vpl(entry.rule, i, j, entry.split, ctx);
            pf_t term = 1;
            const auto children = expand_vpl(entry.rule, i, j, entry.split);
            for (const auto &child : children) {
                if (child.nonterminal == NonTerminal::VP) {
                    term *= ctx.get_energy_VP(child.i, child.j);
                }
            }
            contributions += term * coeff;
        }
        ctx.set_VPL(ij, contributions);
        return;
    }
    for (RuleId rule : rules_for(NonTerminal::VPL)) {
        if (!is_rule_enabled(config, rule)) continue;
        const auto splits = enumerate_splits_vpl(rule, i, j, ctx, tree);
        for (const auto &split : splits) {
            record_rule_hit(rule);
            pf_t coeff = rule_score_vpl(rule, i, j, split, ctx);
            pf_t term = 1;
            const auto children = expand_vpl(rule, i, j, split);
            for (const auto &child : children) {
                if (child.nonterminal == NonTerminal::VP) {
                    term *= ctx.get_energy_VP(child.i, child.j);
                }
            }
            contributions += term * coeff;
        }
    }
    ctx.set_VPL(ij, contributions);
}

void compute_VPL_restricted_rules(PartFuncVPLContext &ctx,
                                  cand_pos_t i,
                                  cand_pos_t j,
                                  const StructureView &view,
                                  const RulesConfig &config) {
    const cand_pos_t ij = ctx.index_of(i, j);
    pf_t contributions = 0;
    if (config.use_applicable) {
        const auto applicable = applicable_rules_vpl(i, j, ctx, view);
        for (const auto &entry : applicable) {
            if (!is_rule_enabled(config, entry.rule)) continue;
            record_rule_hit(entry.rule);
            pf_t coeff = rule_score_vpl(entry.rule, i, j, entry.split, ctx);
            pf_t term = 1;
            const auto children = expand_vpl(entry.rule, i, j, entry.split);
            for (const auto &child : children) {
                if (child.nonterminal == NonTerminal::VP) {
                    term *= ctx.get_energy_VP(child.i, child.j);
                }
            }
            contributions += term * coeff;
        }
        ctx.set_VPL(ij, contributions);
        return;
    }
    for (RuleId rule : rules_for(NonTerminal::VPL)) {
        if (!is_rule_enabled(config, rule)) continue;
        const auto splits = enumerate_splits_vpl(rule, i, j, ctx, view);
        for (const auto &split : splits) {
            record_rule_hit(rule);
            pf_t coeff = rule_score_vpl(rule, i, j, split, ctx);
            pf_t term = 1;
            const auto children = expand_vpl(rule, i, j, split);
            for (const auto &child : children) {
                if (child.nonterminal == NonTerminal::VP) {
                    term *= ctx.get_energy_VP(child.i, child.j);
                }
            }
            contributions += term * coeff;
        }
    }
    ctx.set_VPL(ij, contributions);
}

void compute_VPR_restricted_rules(PartFuncVPRContext &ctx, cand_pos_t i, cand_pos_t j, sparse_tree &tree, const RulesConfig &config) {
    const cand_pos_t ij = ctx.index_of(i, j);
    pf_t contributions = 0;
    if (config.use_applicable) {
        const auto applicable = applicable_rules_vpr(i, j, ctx, tree);
        for (const auto &entry : applicable) {
            if (!is_rule_enabled(config, entry.rule)) continue;
            record_rule_hit(entry.rule);
            pf_t coeff = rule_score_vpr(entry.rule, i, j, entry.split, ctx);
            pf_t term = 1;
            const auto children = expand_vpr(entry.rule, i, j, entry.split);
            for (const auto &child : children) {
                if (child.nonterminal == NonTerminal::VP) {
                    term *= ctx.get_energy_VP(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::WIP) {
                    term *= ctx.get_energy_WIP(child.i, child.j);
                }
            }
            contributions += term * coeff;
        }
        ctx.set_VPR(ij, contributions);
        return;
    }
    for (RuleId rule : rules_for(NonTerminal::VPR)) {
        if (!is_rule_enabled(config, rule)) continue;
        const auto splits = enumerate_splits_vpr(rule, i, j, ctx, tree);
        for (const auto &split : splits) {
            record_rule_hit(rule);
            pf_t coeff = rule_score_vpr(rule, i, j, split, ctx);
            pf_t term = 1;
            const auto children = expand_vpr(rule, i, j, split);
            for (const auto &child : children) {
                if (child.nonterminal == NonTerminal::VP) {
                    term *= ctx.get_energy_VP(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::WIP) {
                    term *= ctx.get_energy_WIP(child.i, child.j);
                }
            }
            contributions += term * coeff;
        }
    }
    ctx.set_VPR(ij, contributions);
}

void compute_VPR_restricted_rules(PartFuncVPRContext &ctx,
                                  cand_pos_t i,
                                  cand_pos_t j,
                                  const StructureView &view,
                                  const RulesConfig &config) {
    const cand_pos_t ij = ctx.index_of(i, j);
    pf_t contributions = 0;
    if (config.use_applicable) {
        const auto applicable = applicable_rules_vpr(i, j, ctx, view);
        for (const auto &entry : applicable) {
            if (!is_rule_enabled(config, entry.rule)) continue;
            record_rule_hit(entry.rule);
            pf_t coeff = rule_score_vpr(entry.rule, i, j, entry.split, ctx);
            pf_t term = 1;
            const auto children = expand_vpr(entry.rule, i, j, entry.split);
            for (const auto &child : children) {
                if (child.nonterminal == NonTerminal::VP) {
                    term *= ctx.get_energy_VP(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::WIP) {
                    term *= ctx.get_energy_WIP(child.i, child.j);
                }
            }
            contributions += term * coeff;
        }
        ctx.set_VPR(ij, contributions);
        return;
    }
    for (RuleId rule : rules_for(NonTerminal::VPR)) {
        if (!is_rule_enabled(config, rule)) continue;
        const auto splits = enumerate_splits_vpr(rule, i, j, ctx, view);
        for (const auto &split : splits) {
            record_rule_hit(rule);
            pf_t coeff = rule_score_vpr(rule, i, j, split, ctx);
            pf_t term = 1;
            const auto children = expand_vpr(rule, i, j, split);
            for (const auto &child : children) {
                if (child.nonterminal == NonTerminal::VP) {
                    term *= ctx.get_energy_VP(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::WIP) {
                    term *= ctx.get_energy_WIP(child.i, child.j);
                }
            }
            contributions += term * coeff;
        }
    }
    ctx.set_VPR(ij, contributions);
}

void compute_VP_restricted_rules(PartFuncVPContext &ctx, cand_pos_t i, cand_pos_t j, sparse_tree &tree, const RulesConfig &config) {
    const cand_pos_t ij = ctx.index_of(i, j);
    pf_t contributions = 0;
    if (config.use_applicable) {
        const auto applicable = applicable_rules_vp(i, j, ctx, tree);
        for (const auto &entry : applicable) {
            if (!is_rule_enabled(config, entry.rule)) continue;
            record_rule_hit(entry.rule);
            pf_t coeff = rule_score_vp(entry.rule, i, j, entry.split, ctx, tree);
            pf_t term = 1;
            const auto children = expand_vp(entry.rule, i, j, entry.split);
            for (const auto &child : children) {
                if (child.nonterminal == NonTerminal::VP) {
                    term *= ctx.get_energy_VP(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::WIP) {
                    term *= ctx.get_energy_WIP(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::VPR) {
                    term *= ctx.get_energy_VPR(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::VPL) {
                    term *= ctx.get_energy_VPL(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::WI) {
                    term *= ctx.get_energy_WI(child.i, child.j);
                }
            }
            contributions += term * coeff;
        }
        ctx.set_VP(ij, contributions);
        return;
    }
    for (RuleId rule : rules_for(NonTerminal::VP)) {
        if (!is_rule_enabled(config, rule)) continue;
        const auto splits = enumerate_splits_vp(rule, i, j, ctx, tree);
        for (const auto &split : splits) {
            record_rule_hit(rule);
            pf_t coeff = rule_score_vp(rule, i, j, split, ctx, tree);
            pf_t term = 1;
            const auto children = expand_vp(rule, i, j, split);
            for (const auto &child : children) {
                if (child.nonterminal == NonTerminal::VP) {
                    term *= ctx.get_energy_VP(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::WIP) {
                    term *= ctx.get_energy_WIP(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::VPR) {
                    term *= ctx.get_energy_VPR(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::VPL) {
                    term *= ctx.get_energy_VPL(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::WI) {
                    term *= ctx.get_energy_WI(child.i, child.j);
                }
            }
            contributions += term * coeff;
        }
    }
    ctx.set_VP(ij, contributions);
}

void compute_VP_restricted_rules(PartFuncVPContext &ctx,
                                 cand_pos_t i,
                                 cand_pos_t j,
                                 const StructureView &view,
                                 sparse_tree &tree,
                                 const RulesConfig &config) {
    const cand_pos_t ij = ctx.index_of(i, j);
    pf_t contributions = 0;
    if (config.use_applicable) {
        const auto applicable = applicable_rules_vp(i, j, ctx, view);
        for (const auto &entry : applicable) {
            if (!is_rule_enabled(config, entry.rule)) continue;
            record_rule_hit(entry.rule);
            pf_t coeff = rule_score_vp(entry.rule, i, j, entry.split, ctx, tree);
            pf_t term = 1;
            const auto children = expand_vp(entry.rule, i, j, entry.split);
            for (const auto &child : children) {
                if (child.nonterminal == NonTerminal::VP) {
                    term *= ctx.get_energy_VP(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::WIP) {
                    term *= ctx.get_energy_WIP(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::VPR) {
                    term *= ctx.get_energy_VPR(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::VPL) {
                    term *= ctx.get_energy_VPL(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::WI) {
                    term *= ctx.get_energy_WI(child.i, child.j);
                }
            }
            contributions += term * coeff;
        }
        ctx.set_VP(ij, contributions);
        return;
    }
    for (RuleId rule : rules_for(NonTerminal::VP)) {
        if (!is_rule_enabled(config, rule)) continue;
        const auto splits = enumerate_splits_vp(rule, i, j, ctx, view);
        for (const auto &split : splits) {
            record_rule_hit(rule);
            pf_t coeff = rule_score_vp(rule, i, j, split, ctx, tree);
            pf_t term = 1;
            const auto children = expand_vp(rule, i, j, split);
            for (const auto &child : children) {
                if (child.nonterminal == NonTerminal::VP) {
                    term *= ctx.get_energy_VP(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::WIP) {
                    term *= ctx.get_energy_WIP(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::VPR) {
                    term *= ctx.get_energy_VPR(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::VPL) {
                    term *= ctx.get_energy_VPL(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::WI) {
                    term *= ctx.get_energy_WI(child.i, child.j);
                }
            }
            contributions += term * coeff;
        }
    }
    ctx.set_VP(ij, contributions);
}

void compute_WMBP_restricted_rules(PartFuncWMBPContext &ctx, cand_pos_t i, cand_pos_t j, sparse_tree &tree, const RulesConfig &config) {
    const cand_pos_t ij = ctx.index_of(i, j);
    pf_t contributions = 0;
    if (config.use_applicable) {
        const auto applicable = applicable_rules_wmbp(i, j, ctx, tree);
        for (const auto &entry : applicable) {
            if (!is_rule_enabled(config, entry.rule)) continue;
            record_rule_hit(entry.rule);
            pf_t coeff = rule_score_wmbp(entry.rule, i, j, entry.split, ctx, tree);
            pf_t term = 1;
            const auto children = expand_wmbp(entry.rule, i, j, entry.split);
            for (const auto &child : children) {
                if (child.nonterminal == NonTerminal::WMBP) {
                    term *= ctx.get_energy_WMBP(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::WMBW) {
                    term *= ctx.get_energy_WMBW(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::WI) {
                    term *= ctx.get_energy_WI(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::VP) {
                    term *= ctx.get_energy_VP(child.i, child.j);
                }
            }
            contributions += term * coeff;
        }
        ctx.set_WMBP(ij, contributions);
        return;
    }
    for (RuleId rule : rules_for(NonTerminal::WMBP)) {
        if (!is_rule_enabled(config, rule)) continue;
        const auto splits = enumerate_splits_wmbp(rule, i, j, ctx, tree);
        for (const auto &split : splits) {
            record_rule_hit(rule);
            pf_t coeff = rule_score_wmbp(rule, i, j, split, ctx, tree);
            pf_t term = 1;
            const auto children = expand_wmbp(rule, i, j, split);
            for (const auto &child : children) {
                if (child.nonterminal == NonTerminal::WMBP) {
                    term *= ctx.get_energy_WMBP(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::WMBW) {
                    term *= ctx.get_energy_WMBW(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::WI) {
                    term *= ctx.get_energy_WI(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::VP) {
                    term *= ctx.get_energy_VP(child.i, child.j);
                }
            }
            contributions += term * coeff;
        }
    }
    ctx.set_WMBP(ij, contributions);
}

void compute_WMBP_restricted_rules(PartFuncWMBPContext &ctx,
                                   cand_pos_t i,
                                   cand_pos_t j,
                                   const StructureView &view,
                                   sparse_tree &tree,
                                   const RulesConfig &config) {
    const cand_pos_t ij = ctx.index_of(i, j);
    pf_t contributions = 0;
    if (config.use_applicable) {
        const auto applicable = applicable_rules_wmbp(i, j, ctx, view, tree);
        for (const auto &entry : applicable) {
            if (!is_rule_enabled(config, entry.rule)) continue;
            record_rule_hit(entry.rule);
            pf_t coeff = rule_score_wmbp(entry.rule, i, j, entry.split, ctx, tree);
            pf_t term = 1;
            const auto children = expand_wmbp(entry.rule, i, j, entry.split);
            for (const auto &child : children) {
                if (child.nonterminal == NonTerminal::WMBP) {
                    term *= ctx.get_energy_WMBP(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::WMBW) {
                    term *= ctx.get_energy_WMBW(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::WI) {
                    term *= ctx.get_energy_WI(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::VP) {
                    term *= ctx.get_energy_VP(child.i, child.j);
                }
            }
            contributions += term * coeff;
        }
        ctx.set_WMBP(ij, contributions);
        return;
    }
    for (RuleId rule : rules_for(NonTerminal::WMBP)) {
        if (!is_rule_enabled(config, rule)) continue;
        const auto splits = enumerate_splits_wmbp(rule, i, j, ctx, view, tree);
        for (const auto &split : splits) {
            record_rule_hit(rule);
            pf_t coeff = rule_score_wmbp(rule, i, j, split, ctx, tree);
            pf_t term = 1;
            const auto children = expand_wmbp(rule, i, j, split);
            for (const auto &child : children) {
                if (child.nonterminal == NonTerminal::WMBP) {
                    term *= ctx.get_energy_WMBP(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::WMBW) {
                    term *= ctx.get_energy_WMBW(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::WI) {
                    term *= ctx.get_energy_WI(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::VP) {
                    term *= ctx.get_energy_VP(child.i, child.j);
                }
            }
            contributions += term * coeff;
        }
    }
    ctx.set_WMBP(ij, contributions);
}

void compute_WMBW_restricted_rules(PartFuncWMBWContext &ctx, cand_pos_t i, cand_pos_t j, sparse_tree &tree, const RulesConfig &config) {
    const cand_pos_t ij = ctx.index_of(i, j);
    pf_t contributions = 0;
    if (config.use_applicable) {
        const auto applicable = applicable_rules_wmbw(i, j, ctx, tree);
        for (const auto &entry : applicable) {
            if (!is_rule_enabled(config, entry.rule)) continue;
            record_rule_hit(entry.rule);
            pf_t coeff = rule_score_wmbw(entry.rule, i, j, entry.split, ctx);
            pf_t term = 1;
            const auto children = expand_wmbw(entry.rule, i, j, entry.split);
            for (const auto &child : children) {
                if (child.nonterminal == NonTerminal::WMBP) {
                    term *= ctx.get_energy_WMBP(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::WI) {
                    term *= ctx.get_energy_WI(child.i, child.j);
                }
            }
            contributions += term * coeff;
        }
        ctx.set_WMBW(ij, contributions);
        return;
    }
    for (RuleId rule : rules_for(NonTerminal::WMBW)) {
        if (!is_rule_enabled(config, rule)) continue;
        const auto splits = enumerate_splits_wmbw(rule, i, j, ctx, tree);
        for (const auto &split : splits) {
            record_rule_hit(rule);
            pf_t coeff = rule_score_wmbw(rule, i, j, split, ctx);
            pf_t term = 1;
            const auto children = expand_wmbw(rule, i, j, split);
            for (const auto &child : children) {
                if (child.nonterminal == NonTerminal::WMBP) {
                    term *= ctx.get_energy_WMBP(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::WI) {
                    term *= ctx.get_energy_WI(child.i, child.j);
                }
            }
            contributions += term * coeff;
        }
    }
    ctx.set_WMBW(ij, contributions);
}

void compute_WMBW_restricted_rules(PartFuncWMBWContext &ctx,
                                   cand_pos_t i,
                                   cand_pos_t j,
                                   const StructureView &view,
                                   const RulesConfig &config) {
    const cand_pos_t ij = ctx.index_of(i, j);
    pf_t contributions = 0;
    if (config.use_applicable) {
        const auto applicable = applicable_rules_wmbw(i, j, ctx, view);
        for (const auto &entry : applicable) {
            if (!is_rule_enabled(config, entry.rule)) continue;
            record_rule_hit(entry.rule);
            pf_t coeff = rule_score_wmbw(entry.rule, i, j, entry.split, ctx);
            pf_t term = 1;
            const auto children = expand_wmbw(entry.rule, i, j, entry.split);
            for (const auto &child : children) {
                if (child.nonterminal == NonTerminal::WMBP) {
                    term *= ctx.get_energy_WMBP(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::WI) {
                    term *= ctx.get_energy_WI(child.i, child.j);
                }
            }
            contributions += term * coeff;
        }
        ctx.set_WMBW(ij, contributions);
        return;
    }
    for (RuleId rule : rules_for(NonTerminal::WMBW)) {
        if (!is_rule_enabled(config, rule)) continue;
        const auto splits = enumerate_splits_wmbw(rule, i, j, ctx, view);
        for (const auto &split : splits) {
            record_rule_hit(rule);
            pf_t coeff = rule_score_wmbw(rule, i, j, split, ctx);
            pf_t term = 1;
            const auto children = expand_wmbw(rule, i, j, split);
            for (const auto &child : children) {
                if (child.nonterminal == NonTerminal::WMBP) {
                    term *= ctx.get_energy_WMBP(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::WI) {
                    term *= ctx.get_energy_WI(child.i, child.j);
                }
            }
            contributions += term * coeff;
        }
    }
    ctx.set_WMBW(ij, contributions);
}

void compute_WMB_restricted_rules(PartFuncWMBContext &ctx, cand_pos_t i, cand_pos_t j, sparse_tree &tree, const RulesConfig &config) {
    const cand_pos_t ij = ctx.index_of(i, j);
    pf_t contributions = 0;
    if (config.use_applicable) {
        const auto applicable = applicable_rules_wmb(i, j, ctx, tree);
        for (const auto &entry : applicable) {
            if (!is_rule_enabled(config, entry.rule)) continue;
            record_rule_hit(entry.rule);
            pf_t coeff = rule_score_wmb(entry.rule, i, j, entry.split, ctx, tree);
            pf_t term = 1;
            const auto children = expand_wmb(entry.rule, i, j, entry.split);
            for (const auto &child : children) {
                if (child.nonterminal == NonTerminal::WMBP) {
                    term *= ctx.get_energy_WMBP(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::WI) {
                    term *= ctx.get_energy_WI(child.i, child.j);
                }
            }
            contributions += term * coeff;
        }
        ctx.set_WMB(ij, contributions);
        return;
    }
    for (RuleId rule : rules_for(NonTerminal::WMB)) {
        if (!is_rule_enabled(config, rule)) continue;
        const auto splits = enumerate_splits_wmb(rule, i, j, ctx, tree);
        for (const auto &split : splits) {
            record_rule_hit(rule);
            pf_t coeff = rule_score_wmb(rule, i, j, split, ctx, tree);
            pf_t term = 1;
            const auto children = expand_wmb(rule, i, j, split);
            for (const auto &child : children) {
                if (child.nonterminal == NonTerminal::WMBP) {
                    term *= ctx.get_energy_WMBP(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::WI) {
                    term *= ctx.get_energy_WI(child.i, child.j);
                }
            }
            contributions += term * coeff;
        }
    }
    ctx.set_WMB(ij, contributions);
}

void compute_WMB_restricted_rules(PartFuncWMBContext &ctx,
                                  cand_pos_t i,
                                  cand_pos_t j,
                                  const StructureView &view,
                                  sparse_tree &tree,
                                  const RulesConfig &config) {
    const cand_pos_t ij = ctx.index_of(i, j);
    pf_t contributions = 0;
    if (config.use_applicable) {
        const auto applicable = applicable_rules_wmb(i, j, ctx, view);
        for (const auto &entry : applicable) {
            if (!is_rule_enabled(config, entry.rule)) continue;
            record_rule_hit(entry.rule);
            pf_t coeff = rule_score_wmb(entry.rule, i, j, entry.split, ctx, tree);
            pf_t term = 1;
            const auto children = expand_wmb(entry.rule, i, j, entry.split);
            for (const auto &child : children) {
                if (child.nonterminal == NonTerminal::WMBP) {
                    term *= ctx.get_energy_WMBP(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::WI) {
                    term *= ctx.get_energy_WI(child.i, child.j);
                }
            }
            contributions += term * coeff;
        }
        ctx.set_WMB(ij, contributions);
        return;
    }
    for (RuleId rule : rules_for(NonTerminal::WMB)) {
        if (!is_rule_enabled(config, rule)) continue;
        const auto splits = enumerate_splits_wmb(rule, i, j, ctx, view);
        for (const auto &split : splits) {
            record_rule_hit(rule);
            pf_t coeff = rule_score_wmb(rule, i, j, split, ctx, tree);
            pf_t term = 1;
            const auto children = expand_wmb(rule, i, j, split);
            for (const auto &child : children) {
                if (child.nonterminal == NonTerminal::WMBP) {
                    term *= ctx.get_energy_WMBP(child.i, child.j);
                } else if (child.nonterminal == NonTerminal::WI) {
                    term *= ctx.get_energy_WI(child.i, child.j);
                }
            }
            contributions += term * coeff;
        }
    }
    ctx.set_WMB(ij, contributions);
}

void compute_BE_restricted_rules(PartFuncBEContext &ctx,
                                 cand_pos_t i,
                                 cand_pos_t j,
                                 cand_pos_t ip,
                                 cand_pos_t jp,
                                 sparse_tree &tree,
                                 const RulesConfig &config) {
    if (!(i >= 1 && i <= ip && ip < jp && jp <= j && j <= ctx.n() && tree.tree[i].pair > 0 && tree.tree[j].pair > 0 &&
          tree.tree[ip].pair > 0 && tree.tree[jp].pair > 0 && tree.tree[i].pair == j && tree.tree[j].pair == i &&
          tree.tree[ip].pair == jp && tree.tree[jp].pair == ip)) {
        if (i >= 1 && i <= ctx.n() && ip >= i && ip <= ctx.n()) {
            cand_pos_t iip = ctx.index_of(i, ip);
            ctx.set_BE(iip, 0);
        }
        return;
    }

    cand_pos_t iip = ctx.index_of(i, ip);
    pf_t contributions = 0;
    if (config.use_applicable) {
        const auto applicable = applicable_rules_be(i, j, ip, jp, ctx, tree);
        for (const auto &entry : applicable) {
            if (!is_rule_enabled(config, entry.rule)) continue;
            record_rule_hit(entry.rule);
            pf_t coeff = rule_score_be(entry.rule, i, j, ip, jp, entry.split, ctx, tree);
            pf_t term = 1;
            const auto children = expand_be(entry.rule, i, j, ip, jp, entry.split);
            for (const auto &child : children) {
                if (child.nonterminal == NonTerminal::BE) {
                    term *= ctx.get_BE(child.i, child.j, ip, jp, tree);
                } else if (child.nonterminal == NonTerminal::WIP) {
                    term *= ctx.get_energy_WIP(child.i, child.j);
                }
            }
            contributions += term * coeff;
        }
        ctx.set_BE(iip, contributions);
        return;
    }
    for (RuleId rule : rules_for(NonTerminal::BE)) {
        if (!is_rule_enabled(config, rule)) continue;
        const auto splits = enumerate_splits_be(rule, i, j, ip, jp, ctx, tree);
        for (const auto &split : splits) {
            record_rule_hit(rule);
            pf_t coeff = rule_score_be(rule, i, j, ip, jp, split, ctx, tree);
            pf_t term = 1;
            const auto children = expand_be(rule, i, j, ip, jp, split);
            for (const auto &child : children) {
                if (child.nonterminal == NonTerminal::BE) {
                    term *= ctx.get_BE(child.i, child.j, ip, jp, tree);
                } else if (child.nonterminal == NonTerminal::WIP) {
                    term *= ctx.get_energy_WIP(child.i, child.j);
                }
            }
            contributions += term * coeff;
        }
    }
    ctx.set_BE(iip, contributions);
}

void compute_BE_restricted_rules(PartFuncBEContext &ctx,
                                 cand_pos_t i,
                                 cand_pos_t j,
                                 cand_pos_t ip,
                                 cand_pos_t jp,
                                 const StructureView &view,
                                 sparse_tree &tree,
                                 const RulesConfig &config) {
    if (!(i >= 1 && i <= ip && ip < jp && jp <= j && j <= ctx.n() && view.is_pair_square(i, j) &&
          view.is_pair_square(ip, jp))) {
        if (i >= 1 && i <= ctx.n() && ip >= i && ip <= ctx.n()) {
            cand_pos_t iip = ctx.index_of(i, ip);
            ctx.set_BE(iip, 0);
        }
        return;
    }

    cand_pos_t iip = ctx.index_of(i, ip);
    pf_t contributions = 0;
    if (config.use_applicable) {
        const auto applicable = applicable_rules_be(i, j, ip, jp, ctx, view);
        for (const auto &entry : applicable) {
            if (!is_rule_enabled(config, entry.rule)) continue;
            record_rule_hit(entry.rule);
            pf_t coeff = rule_score_be(entry.rule, i, j, ip, jp, entry.split, ctx, tree);
            pf_t term = 1;
            const auto children = expand_be(entry.rule, i, j, ip, jp, entry.split);
            for (const auto &child : children) {
                if (child.nonterminal == NonTerminal::BE) {
                    term *= ctx.get_BE(child.i, child.j, ip, jp, tree);
                } else if (child.nonterminal == NonTerminal::WIP) {
                    term *= ctx.get_energy_WIP(child.i, child.j);
                }
            }
            contributions += term * coeff;
        }
        ctx.set_BE(iip, contributions);
        return;
    }
    for (RuleId rule : rules_for(NonTerminal::BE)) {
        if (!is_rule_enabled(config, rule)) continue;
        const auto splits = enumerate_splits_be(rule, i, j, ip, jp, ctx, view);
        for (const auto &split : splits) {
            record_rule_hit(rule);
            pf_t coeff = rule_score_be(rule, i, j, ip, jp, split, ctx, tree);
            pf_t term = 1;
            const auto children = expand_be(rule, i, j, ip, jp, split);
            for (const auto &child : children) {
                if (child.nonterminal == NonTerminal::BE) {
                    term *= ctx.get_BE(child.i, child.j, ip, jp, tree);
                } else if (child.nonterminal == NonTerminal::WIP) {
                    term *= ctx.get_energy_WIP(child.i, child.j);
                }
            }
            contributions += term * coeff;
        }
    }
    ctx.set_BE(iip, contributions);
}

} // namespace scfg
