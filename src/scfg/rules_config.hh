#ifndef SCFG_RULES_CONFIG_HH_
#define SCFG_RULES_CONFIG_HH_

#include "scfg/rules_api.hh"

#include <unordered_set>

namespace scfg {

struct RulesConfig {
    bool use_rules = false;
    bool use_applicable = false;
    bool use_only_list = false;
    std::unordered_set<RuleId> disabled_rules;
    std::unordered_set<RuleId> allowed_rules;
};

RulesConfig load_rules_config_from_env();
bool is_rule_enabled(const RulesConfig &config, RuleId rule);
const RulesConfig &get_rules_config();

} // namespace scfg

#endif
