#ifndef ENERGY_EVAL_CONTEXT_HH
#define ENERGY_EVAL_CONTEXT_HH

#include <string>

namespace cparty {

struct EnergyEvalOptions {
    bool pk_free = false;
    bool pk_only = false;
    int dangles = 2;
};

struct EnergyEvalContext {
    std::string normalized_seq;
    std::string db_full;
    EnergyEvalOptions options;
};

} // namespace cparty

#endif
