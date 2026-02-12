#ifndef FIXED_STRUCTURE_ENERGY_INTERNAL_HH
#define FIXED_STRUCTURE_ENERGY_INTERNAL_HH

#include "energy_eval_context.hh"

#include <string>

namespace cparty {
namespace internal {

struct EnergyBreakdown {
    double pk_free_core_kcal;
    double pk_penalties_kcal;
    double band_scaled_terms_kcal;
    double total_kcal;
};

bool build_energy_eval_context(const std::string &seq,
                               const std::string &db_full,
                               const cparty::EnergyEvalOptions &options,
                               cparty::EnergyEvalContext &context) noexcept;

double score_fixed_structure_energy_kcal(const cparty::EnergyEvalContext &context) noexcept;
double evaluate_fixed_structure_energy_kcal(const cparty::EnergyEvalContext &context) noexcept;
double evaluate_fixed_structure_energy_kcal(const std::string &seq, const std::string &db_full) noexcept;
EnergyBreakdown evaluate_fixed_structure_energy_breakdown_kcal(const std::string &seq, const std::string &db_full) noexcept;

} // namespace internal
} // namespace cparty

#endif
