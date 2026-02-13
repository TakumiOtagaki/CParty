#ifndef CPARTY_FIXED_ENERGY_API_HH
#define CPARTY_FIXED_ENERGY_API_HH

#include <string>

namespace cparty {

// Evaluates a fixed-structure energy path. Story 011 exposes the callable API
// and strict validation contract; scoring internals are expanded in later stories.
double get_structure_energy(const std::string &seq, const std::string &db_full);

}  // namespace cparty

#endif  // CPARTY_FIXED_ENERGY_API_HH
