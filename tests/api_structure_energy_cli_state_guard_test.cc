#include "CPartyAPI.hh"
#include "W_final.hh"
#include "part_func.hh"
#include "sparse_tree.hh"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>

namespace {

constexpr double kAbsTol = 1e-6;
constexpr double kRelTol = 1e-6;

bool approximately_equal(double a, double b) {
    const double scale = std::max(std::fabs(a), std::fabs(b));
    return std::fabs(a - b) <= std::max(kAbsTol, kRelTol * scale);
}

struct FoldSnapshot {
    std::string final_structure;
    std::string pf_structure;
    double mfe_energy;
    double pf_energy;
};

FoldSnapshot run_fold_pipeline(const std::string &seq,
                               const std::string &restricted,
                               const cparty::EnergyEvalOptions &options) {
    sparse_tree tree(restricted, static_cast<int>(seq.size()));
    W_final mfe(seq, restricted, options.pk_free, options.pk_only, options.dangles);
    const double mfe_energy = mfe.hfold(tree);
    const std::string final_structure = mfe.structure;

    constexpr bool kFatgraph = false;
    constexpr int kNumSamples = 1000;
    constexpr bool kPsPlot = false;
    std::string mutable_seq = seq;
    std::string mutable_final = final_structure;
    W_final_pf partition(mutable_seq,
                         mutable_final,
                         options.pk_free,
                         options.pk_only,
                         kFatgraph,
                         options.dangles,
                         mfe_energy,
                         kNumSamples,
                         kPsPlot);
    const double pf_energy = partition.hfold_pf(tree);
    return FoldSnapshot{final_structure, partition.structure, mfe_energy, pf_energy};
}

} // namespace

int main() {
    const std::string seq = "GGGAAAUCC";
    const std::string restricted = "(((...)))";
    const cparty::EnergyEvalOptions options{};

    const FoldSnapshot before = run_fold_pipeline(seq, restricted, options);
    if (!std::isfinite(before.mfe_energy) || !std::isfinite(before.pf_energy)) {
        std::cerr << "expected finite baseline fold outputs" << std::endl;
        return 1;
    }

    const double shared_energy = get_structure_energy(seq, before.final_structure, options);
    if (!std::isfinite(shared_energy)) {
        std::cerr << "expected finite shared fixed-structure energy" << std::endl;
        return 1;
    }

    const FoldSnapshot after = run_fold_pipeline(seq, restricted, options);
    if (before.final_structure != after.final_structure) {
        std::cerr << "mfe structure changed after shared evaluator call" << std::endl;
        return 1;
    }
    if (before.pf_structure != after.pf_structure) {
        std::cerr << "pf structure changed after shared evaluator call" << std::endl;
        return 1;
    }
    if (!approximately_equal(before.mfe_energy, after.mfe_energy)) {
        std::cerr << "mfe energy changed after shared evaluator call: before=" << before.mfe_energy
                  << " after=" << after.mfe_energy << std::endl;
        return 1;
    }
    if (!approximately_equal(before.pf_energy, after.pf_energy)) {
        std::cerr << "pf energy changed after shared evaluator call: before=" << before.pf_energy
                  << " after=" << after.pf_energy << std::endl;
        return 1;
    }

    return 0;
}
