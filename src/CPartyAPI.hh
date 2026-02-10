#ifndef CPARTY_API_HH
#define CPARTY_API_HH

#include <string>

// Return the conditional log probability ln P(G' | G, S) based on CParty's
// ensemble free energy for the structure G (db_base) on sequence S.
// On failure, returns NaN.
double get_cond_log_prob(const std::string &seq, const std::string &db_base);

#endif
