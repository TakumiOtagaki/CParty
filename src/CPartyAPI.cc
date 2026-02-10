#include "CPartyAPI.hh"

#include "W_final.hh"
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
