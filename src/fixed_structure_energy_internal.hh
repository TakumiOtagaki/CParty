#ifndef FIXED_STRUCTURE_ENERGY_INTERNAL_HH
#define FIXED_STRUCTURE_ENERGY_INTERNAL_HH

#include <string>

namespace cparty {
namespace internal {

double evaluate_fixed_structure_energy_kcal(const std::string &seq, const std::string &db_full) noexcept;

} // namespace internal
} // namespace cparty

#endif
