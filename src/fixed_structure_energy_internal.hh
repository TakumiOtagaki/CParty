#ifndef FIXED_STRUCTURE_ENERGY_INTERNAL_HH
#define FIXED_STRUCTURE_ENERGY_INTERNAL_HH

#include <string>

namespace cparty {
namespace internal {

struct EnergyBreakdown {
    double pk_free_core_kcal;
    double pk_penalties_kcal;
    double band_scaled_terms_kcal;
    double total_kcal;
};

double evaluate_fixed_structure_energy_kcal(const std::string &seq, const std::string &db_full) noexcept;
EnergyBreakdown evaluate_fixed_structure_energy_breakdown_kcal(const std::string &seq, const std::string &db_full) noexcept;

} // namespace internal
} // namespace cparty

#endif
