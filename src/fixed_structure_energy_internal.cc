#include "fixed_structure_energy_internal.hh"

#include "W_final.hh"
#include "sparse_tree.hh"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <filesystem>
#include <limits>
#include <string>
#include <utility>
#include <vector>

extern "C" {
#include "ViennaRNA/params/io.h"
}

namespace cparty {
namespace internal {
namespace {

constexpr char kSupportedPkOpen = '[';
constexpr char kSupportedPkClose = ']';

struct ParsedStructure {
    std::string tree_structure;
    bool uses_pk = false;
    std::vector<std::pair<size_t, size_t>> pairs;
};

double quiet_nan() { return std::numeric_limits<double>::quiet_NaN(); }

std::string normalize_sequence(std::string seq) {
    std::transform(seq.begin(), seq.end(), seq.begin(), [](unsigned char c) {
        return static_cast<char>(std::toupper(c));
    });
    for (char &c : seq) {
        if (c == 'T') {
            c = 'U';
        }
    }
    return seq;
}

bool is_valid_seq_symbol(char c) { return c == 'A' || c == 'C' || c == 'G' || c == 'U'; }

bool can_pair(char left, char right) {
    return (left == 'A' && right == 'U') || (left == 'U' && right == 'A') || (left == 'C' && right == 'G') || (left == 'G' && right == 'C') ||
           (left == 'G' && right == 'U') || (left == 'U' && right == 'G');
}

bool load_turner_params_once() {
    static bool loaded = false;
    static bool success = false;
    if (loaded) {
        return success;
    }
    loaded = true;

    std::filesystem::path param_path = std::filesystem::path(__FILE__).parent_path() / "../params/rna_Turner04.par";
    std::error_code ec;
    std::filesystem::path normalized = std::filesystem::weakly_canonical(param_path, ec);
    const std::string param_str = (ec ? param_path.string() : normalized.string());

    if (std::filesystem::exists(param_str)) {
        success = (vrna_params_load(param_str.c_str(), VRNA_PARAMETER_FORMAT_DEFAULT) != 0);
        return success;
    }

    success = (vrna_params_load_RNA_Turner2004() != 0);
    return success;
}

bool parse_structure(const std::string &seq, const std::string &db_full, ParsedStructure &parsed) {
    if (db_full.empty() || seq.empty() || seq.size() != db_full.size()) {
        return false;
    }

    std::vector<size_t> canonical_stack;
    std::vector<size_t> pk_stack;
    std::vector<size_t> brace_stack;
    std::vector<size_t> angle_stack;
    bool seen_square = false;
    bool seen_brace = false;
    bool seen_angle = false;

    parsed.tree_structure = db_full;
    parsed.pairs.clear();
    parsed.uses_pk = false;

    for (size_t i = 0; i < db_full.size(); ++i) {
        const char ch = db_full[i];
        switch (ch) {
        case '.':
            break;
        case '(':
            canonical_stack.push_back(i);
            break;
        case ')': {
            if (canonical_stack.empty()) {
                return false;
            }
            const size_t j = canonical_stack.back();
            canonical_stack.pop_back();
            parsed.pairs.emplace_back(j, i);
            break;
        }
        case '[':
            seen_square = true;
            parsed.uses_pk = true;
            pk_stack.push_back(i);
            parsed.tree_structure[i] = '.';
            break;
        case ']': {
            seen_square = true;
            parsed.uses_pk = true;
            if (pk_stack.empty()) {
                return false;
            }
            const size_t j = pk_stack.back();
            pk_stack.pop_back();
            parsed.pairs.emplace_back(j, i);
            parsed.tree_structure[i] = '.';
            break;
        }
        case '{':
            seen_brace = true;
            return false;
        case '}':
            seen_brace = true;
            return false;
        case '<':
            seen_angle = true;
            return false;
        case '>':
            seen_angle = true;
            return false;
        default:
            return false;
        }
    }

    if (!canonical_stack.empty() || !pk_stack.empty() || !brace_stack.empty() || !angle_stack.empty()) {
        return false;
    }

    const int pk_families = static_cast<int>(seen_square) + static_cast<int>(seen_brace) + static_cast<int>(seen_angle);
    if (pk_families > 1) {
        return false;
    }
    if ((seen_square && (kSupportedPkOpen != '[' || kSupportedPkClose != ']')) || seen_brace || seen_angle) {
        return false;
    }

    for (const auto &pair : parsed.pairs) {
        if (!can_pair(seq[pair.first], seq[pair.second])) {
            return false;
        }
    }

    return true;
}

bool validate_and_parse(const std::string &seq, const std::string &db_full, ParsedStructure &parsed, std::string &normalized_seq) {
    normalized_seq = normalize_sequence(seq);
    if (normalized_seq.empty()) {
        return false;
    }
    for (const char c : normalized_seq) {
        if (!is_valid_seq_symbol(c)) {
            return false;
        }
    }
    return parse_structure(normalized_seq, db_full, parsed);
}

double evaluate_from_parsed(const std::string &sequence, const std::string &db_full, const ParsedStructure &parsed) {
    sparse_tree tree(parsed.tree_structure, static_cast<int>(sequence.size()));

    constexpr bool pk_free = false;
    constexpr bool pk_only = false;
    constexpr int dangles = 2;

    W_final fold(sequence, db_full, pk_free, pk_only, dangles);
    return fold.hfold(tree);
}

} // namespace

double evaluate_fixed_structure_energy_kcal(const std::string &seq, const std::string &db_full) noexcept {
    if (!load_turner_params_once()) {
        return quiet_nan();
    }

    std::string sequence;
    ParsedStructure parsed;
    if (!validate_and_parse(seq, db_full, parsed, sequence)) {
        return quiet_nan();
    }

    return evaluate_from_parsed(sequence, db_full, parsed);
}

EnergyBreakdown evaluate_fixed_structure_energy_breakdown_kcal(const std::string &seq, const std::string &db_full) noexcept {
    EnergyBreakdown breakdown = {quiet_nan(), quiet_nan(), quiet_nan(), quiet_nan()};
    if (!load_turner_params_once()) {
        return breakdown;
    }

    std::string sequence;
    ParsedStructure parsed;
    if (!validate_and_parse(seq, db_full, parsed, sequence)) {
        return breakdown;
    }

    const double total = evaluate_from_parsed(sequence, db_full, parsed);
    if (!std::isfinite(total)) {
        return breakdown;
    }

    breakdown.total_kcal = total;
    breakdown.band_scaled_terms_kcal = 0.0;
    if (!parsed.uses_pk) {
        breakdown.pk_free_core_kcal = total;
        breakdown.pk_penalties_kcal = 0.0;
        return breakdown;
    }

    const ParsedStructure pk_free_parsed = {parsed.tree_structure, false, parsed.pairs};
    const double pk_free_core = evaluate_from_parsed(sequence, parsed.tree_structure, pk_free_parsed);
    if (!std::isfinite(pk_free_core)) {
        return {quiet_nan(), quiet_nan(), quiet_nan(), quiet_nan()};
    }

    breakdown.pk_free_core_kcal = pk_free_core;
    breakdown.pk_penalties_kcal = total - pk_free_core;
    return breakdown;
}

} // namespace internal
} // namespace cparty
