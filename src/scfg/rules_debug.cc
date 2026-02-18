#include "scfg/rules_debug.hh"

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <string>

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

bool is_truthy_value(const char *env_value) {
    if (!env_value) return false;
    std::string value = trim_copy(env_value);
    return value == "1" || value == "true" || value == "TRUE" || value == "rules" || value == "RULES";
}

std::map<RuleId, long long> &rule_hit_counts() {
    // Heap-allocate to avoid static destruction ordering issues with atexit.
    static auto *counts = new std::map<RuleId, long long>();
    return *counts;
}

void dump_rule_hit_counts() {
    if (!rules_debug_enabled()) return;
    const auto &counts = rule_hit_counts();
    if (counts.empty()) return;
    std::fprintf(stderr, "SCFG_RULES_HITS_BEGIN\n");
    for (const auto &entry : counts) {
        std::fprintf(stderr, "%s=%lld\n", rule_id_name(entry.first), entry.second);
    }
    std::fprintf(stderr, "SCFG_RULES_HITS_END\n");
}

void ensure_rule_debug_atexit() {
    static bool registered = false;
    if (!registered) {
        registered = true;
        std::atexit(dump_rule_hit_counts);
    }
}

} // namespace

bool rules_debug_enabled() {
    return is_truthy_value(std::getenv("SCFG_RULES_DEBUG"));
}

void record_rule_hit(RuleId rule) {
    if (!rules_debug_enabled()) return;
    ensure_rule_debug_atexit();
    rule_hit_counts()[rule] += 1;
}

} // namespace scfg
