#ifndef SCFG_RULES_DEBUG_HH_
#define SCFG_RULES_DEBUG_HH_

#include "scfg/rules_api.hh"

namespace scfg {

bool rules_debug_enabled();
void record_rule_hit(RuleId rule);

} // namespace scfg

#endif
