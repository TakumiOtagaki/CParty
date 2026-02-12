#include "CPartyAPI.hh"

#include "W_final.hh"
#include "fixed_structure_energy_internal.hh"
#include "part_func.hh"
#include "sparse_tree.hh"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

extern "C" {
#include "ViennaRNA/params/io.h"
}

namespace {

constexpr double kRT = 0.61632;

std::string to_upper(std::string seq) {
    std::transform(seq.begin(), seq.end(), seq.begin(), [](unsigned char c) {
        return static_cast<char>(std::toupper(c));
    });
    return seq;
}

void seq_to_rna(std::string &sequence) {
    for (char &c : sequence) {
        if (c == 'T') c = 'U';
    }
}

bool validate_sequence(const std::string &sequence) {
    if (sequence.empty()) {
        std::cerr << "Error: sequence is missing" << std::endl;
        return false;
    }
    for (char c : sequence) {
        if (!(c == 'G' || c == 'C' || c == 'A' || c == 'U' || c == 'T' || c == 'N')) {
            std::cerr << "Error: sequence contains character " << c << " that is not N,G,C,A,U, or T." << std::endl;
            return false;
        }
    }
    return true;
}

bool validate_structure(const std::string &seq, const std::string &structure) {
    if (structure.empty()) {
        std::cerr << "Error: structure is missing" << std::endl;
        return false;
    }
    if (structure.size() != seq.size()) {
        std::cerr << "Error: structure length does not match sequence length" << std::endl;
        return false;
    }

    std::vector<int> stack;
    stack.reserve(structure.size());

    for (size_t j = 0; j < structure.size(); ++j) {
        const char ch = structure[j];
        if (ch == '(') {
            stack.push_back(static_cast<int>(j));
        } else if (ch == ')') {
            if (stack.empty()) {
                std::cerr << "Error: incorrect input structure, more right parentheses than left" << std::endl;
                return false;
            }
            int i = stack.back();
            stack.pop_back();
            const char left = seq[static_cast<size_t>(i)];
            const char right = seq[j];
            const bool can_pair = (left == 'A' && right == 'U') || (left == 'U' && right == 'A') || (left == 'C' && right == 'G') ||
                                  (left == 'G' && right == 'C') || (left == 'G' && right == 'U') || (left == 'U' && right == 'G');
            if (!can_pair) {
                std::cerr << "Error: invalid base pair " << left << "-" << right << " at positions " << i << " and " << j << std::endl;
                return false;
            }
        } else if (ch != '.') {
            std::cerr << "Error: invalid structure character " << ch << std::endl;
            return false;
        }
    }

    if (!stack.empty()) {
        std::cerr << "Error: incorrect input structure, more left parentheses than right" << std::endl;
        return false;
    }

    return true;
}

std::string normalize_api_sequence(std::string seq) {
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

bool is_valid_api_symbol(char c) { return c == 'A' || c == 'C' || c == 'G' || c == 'U'; }

bool has_pk_symbols(const std::string &db_full) {
    return db_full.find('[') != std::string::npos || db_full.find(']') != std::string::npos || db_full.find('{') != std::string::npos ||
           db_full.find('}') != std::string::npos || db_full.find('<') != std::string::npos || db_full.find('>') != std::string::npos;
}

bool validate_api_structure(const std::string &seq, const std::string &db_full) {
    if (seq.empty() || db_full.empty() || seq.size() != db_full.size()) {
        return false;
    }

    std::vector<size_t> canonical_stack;
    std::vector<size_t> square_stack;
    std::vector<size_t> brace_stack;
    std::vector<size_t> angle_stack;
    bool seen_square = false;
    bool seen_brace = false;
    bool seen_angle = false;

    for (size_t i = 0; i < db_full.size(); ++i) {
        const char ch = db_full[i];
        switch (ch) {
        case '.':
            break;
        case '(':
            canonical_stack.push_back(i);
            break;
        case ')':
            if (canonical_stack.empty()) {
                return false;
            }
            canonical_stack.pop_back();
            break;
        case '[':
            seen_square = true;
            square_stack.push_back(i);
            break;
        case ']':
            seen_square = true;
            if (square_stack.empty()) {
                return false;
            }
            square_stack.pop_back();
            break;
        case '{':
            seen_brace = true;
            brace_stack.push_back(i);
            break;
        case '}':
            seen_brace = true;
            if (brace_stack.empty()) {
                return false;
            }
            brace_stack.pop_back();
            break;
        case '<':
            seen_angle = true;
            angle_stack.push_back(i);
            break;
        case '>':
            seen_angle = true;
            if (angle_stack.empty()) {
                return false;
            }
            angle_stack.pop_back();
            break;
        default:
            return false;
        }
    }

    if (!canonical_stack.empty() || !square_stack.empty() || !brace_stack.empty() || !angle_stack.empty()) {
        return false;
    }

    const int pk_families = static_cast<int>(seen_square) + static_cast<int>(seen_brace) + static_cast<int>(seen_angle);
    return pk_families <= 1;
}

bool load_turner_params_once() {
    static bool loaded = false;
    static bool success = false;
    if (loaded) return success;
    loaded = true;

    std::filesystem::path param_path = std::filesystem::path(__FILE__).parent_path() / "../params/rna_Turner04.par";
    std::error_code ec;
    std::filesystem::path normalized = std::filesystem::weakly_canonical(param_path, ec);
    const std::string param_str = (ec ? param_path.string() : normalized.string());

    if (std::filesystem::exists(param_str)) {
        success = (vrna_params_load(param_str.c_str(), VRNA_PARAMETER_FORMAT_DEFAULT) != 0);
        if (!success) {
            std::cerr << "Error: failed to load energy parameters from " << param_str << std::endl;
        }
        return success;
    }

    success = (vrna_params_load_RNA_Turner2004() != 0);
    if (!success) {
        std::cerr << "Error: failed to load Turner 2004 parameters" << std::endl;
    }
    return success;
}

} // namespace

double get_cond_log_prob(const std::string &seq, const std::string &db_base) {
    if (!load_turner_params_once()) {
        return std::numeric_limits<double>::quiet_NaN();
    }

    std::string sequence = to_upper(seq);
    seq_to_rna(sequence);

    if (!validate_sequence(sequence)) {
        return std::numeric_limits<double>::quiet_NaN();
    }

    std::string structure = db_base.empty() ? std::string(sequence.size(), '.') : db_base;

    if (!validate_structure(sequence, structure)) {
        return std::numeric_limits<double>::quiet_NaN();
    }

    sparse_tree tree(structure, static_cast<int>(sequence.size()));

    constexpr bool pk_free = false;
    constexpr bool pk_only = false;
    constexpr bool fatgraph = false;
    constexpr int dangles = 2;
    constexpr int num_samples = 1000;
    constexpr bool psplot = false;

    W_final min_fold(sequence, structure, pk_free, pk_only, dangles);
    double mfe_energy = min_fold.hfold(tree);
    std::string mfe_structure = min_fold.structure;

    W_final_pf partition(sequence, mfe_structure, pk_free, pk_only, fatgraph, dangles, mfe_energy, num_samples, psplot);
    const double ensemble_energy = partition.hfold_pf(tree);

    return -ensemble_energy / kRT;
}

double get_structure_energy(const std::string &seq, const std::string &db_full) {
    const std::string normalized_seq = normalize_api_sequence(seq);
    if (normalized_seq.empty()) {
        return std::numeric_limits<double>::quiet_NaN();
    }
    for (char c : normalized_seq) {
        if (!is_valid_api_symbol(c)) {
            return std::numeric_limits<double>::quiet_NaN();
        }
    }
    if (!validate_api_structure(normalized_seq, db_full)) {
        return std::numeric_limits<double>::quiet_NaN();
    }

    // Stage 6c-1: wire PK-free path first. PK families are staged in later stories.
    if (has_pk_symbols(db_full)) {
        return std::numeric_limits<double>::quiet_NaN();
    }

    return cparty::internal::evaluate_fixed_structure_energy_kcal(normalized_seq, db_full);
}
