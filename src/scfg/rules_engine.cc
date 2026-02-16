#include "scfg/rules_engine.hh"

#include <cctype>
#include <cstdlib>
#include <string>
#include <vector>

namespace scfg {
namespace {

std::string trim_copy(const std::string &value) {
    size_t start = 0;
    while (start < value.size() && std::isspace(static_cast<unsigned char>(value[start]))) {
        ++start;
    }
    size_t end = value.size();
    while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1]))) {
        --end;
    }
    return value.substr(start, end - start);
}

std::vector<std::string> split_csv(const std::string &value) {
    std::vector<std::string> parts;
    size_t start = 0;
    while (start <= value.size()) {
        size_t comma = value.find(',', start);
        if (comma == std::string::npos) comma = value.size();
        parts.emplace_back(value.substr(start, comma - start));
        start = comma + 1;
    }
    return parts;
}

bool is_rules_mode_enabled(const char *env_value) {
    if (!env_value) return false;
    std::string value = trim_copy(env_value);
    return value == "rules" || value == "RULES" || value == "1" || value == "true";
}

} // namespace

RulesConfig load_rules_config_from_env() {
    RulesConfig config;
    config.use_rules = is_rules_mode_enabled(std::getenv("SCFG_RULES_MODE"));

    const char *disable_env = std::getenv("SCFG_RULES_DISABLE");
    if (!disable_env) return config;

    std::string raw(disable_env);
    for (const auto &token : split_csv(raw)) {
        std::string trimmed = trim_copy(token);
        if (trimmed.empty()) continue;
        RuleId id;
        if (parse_rule_id(trimmed, &id)) {
            config.disabled_rules.insert(id);
        }
    }
    return config;
}

bool is_rule_enabled(const RulesConfig &config, RuleId rule) {
    if (config.disabled_rules.find(rule) != config.disabled_rules.end()) {
        return false;
    }
    return true;
}

} // namespace scfg
