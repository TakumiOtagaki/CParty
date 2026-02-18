#include "scfg/rules_config.hh"

#include "scfg/rules_debug.hh"

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <map>
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

std::vector<std::string> split_csv_debug(const std::string &value) {
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

bool is_truthy_value(const char *env_value) {
    if (!env_value) return false;
    std::string value = trim_copy(env_value);
    return value == "1" || value == "true" || value == "TRUE" || value == "rules" || value == "RULES";
}

bool is_truthy_or_default_true(const char *env_value) {
    if (!env_value) return true;
    return is_truthy_value(env_value);
}

bool is_rules_mode_enabled(const char *env_value) {
    return is_truthy_value(env_value);
}

void load_rule_id_list(const char *env_value, std::unordered_set<RuleId> *out) {
    if (!env_value) return;
    std::string raw(env_value);
    for (const auto &token : split_csv_debug(raw)) {
        std::string trimmed = trim_copy(token);
        if (trimmed.empty()) continue;
        RuleId id;
        if (parse_rule_id(trimmed, &id)) {
            out->insert(id);
        }
    }
}

void maybe_print_rules_config(const RulesConfig &config) {
    if (!rules_debug_enabled()) return;
    static bool printed = false;
    if (printed) return;
    printed = true;

    std::fprintf(stderr, "SCFG_RULES_MODE=%s\n", config.use_rules ? "rules" : "legacy");
    std::fprintf(stderr, "SCFG_RULES_APPLICABLE=%d\n", config.use_applicable ? 1 : 0);
    std::fprintf(stderr, "SCFG_INSIDE_CORE=%d\n", config.use_inside_core ? 1 : 0);
    std::fprintf(stderr, "SCFG_DENSITY2_VIEW=%d\n", config.use_density2_view ? 1 : 0);
    if (config.use_only_list) {
        std::fprintf(stderr, "SCFG_RULES_ONLY=");
        bool first = true;
        for (const auto &rule : config.allowed_rules) {
            if (!first) std::fprintf(stderr, ",");
            first = false;
            std::fprintf(stderr, "%s", rule_id_name(rule));
        }
        std::fprintf(stderr, "\n");
    }
    if (!config.disabled_rules.empty()) {
        std::fprintf(stderr, "SCFG_RULES_DISABLE=");
        bool first = true;
        for (const auto &rule : config.disabled_rules) {
            if (!first) std::fprintf(stderr, ",");
            first = false;
            std::fprintf(stderr, "%s", rule_id_name(rule));
        }
        std::fprintf(stderr, "\n");
    }
}

} // namespace

RulesConfig load_rules_config_from_env() {
    RulesConfig config;
    config.use_rules = is_rules_mode_enabled(std::getenv("SCFG_RULES_MODE"));
    config.use_applicable = is_truthy_value(std::getenv("SCFG_RULES_APPLICABLE"));
    config.use_inside_core = is_truthy_value(std::getenv("SCFG_INSIDE_CORE"));
    config.use_density2_view = is_truthy_or_default_true(std::getenv("SCFG_DENSITY2_VIEW"));

    load_rule_id_list(std::getenv("SCFG_RULES_DISABLE"), &config.disabled_rules);

    const char *only_env = std::getenv("SCFG_RULES_ONLY");
    if (only_env) {
        std::string trimmed_only = trim_copy(only_env);
        if (trimmed_only.empty()) {
            return config;
        }
        std::unordered_set<RuleId> parsed_only;
        load_rule_id_list(trimmed_only.c_str(), &parsed_only);
        if (parsed_only.empty()) {
            if (rules_debug_enabled()) {
                std::fprintf(stderr, "SCFG_RULES_ONLY_IGNORED=1\n");
            }
            return config;
        }
        config.use_only_list = true;
        config.allowed_rules = std::move(parsed_only);
    }
    return config;
}

bool is_rule_enabled(const RulesConfig &config, RuleId rule) {
    if (config.use_only_list && config.allowed_rules.find(rule) == config.allowed_rules.end()) {
        return false;
    }
    if (config.disabled_rules.find(rule) != config.disabled_rules.end()) {
        return false;
    }
    return true;
}

const RulesConfig &get_rules_config() {
    static const RulesConfig config = load_rules_config_from_env();
    maybe_print_rules_config(config);
    return config;
}

} // namespace scfg
