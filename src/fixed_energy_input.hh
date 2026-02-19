#ifndef CPARTY_FIXED_ENERGY_INPUT_HH
#define CPARTY_FIXED_ENERGY_INPUT_HH

#include <string>
#include <vector>

namespace cparty {

struct NormalizedInput {
  std::string seq;
  std::string db_full;
  std::vector<int> pair_map;
};

[[noreturn]] void fail_invalid_input(const std::string &reason);

NormalizedInput normalize_input(const std::string &seq, const std::string &db_full);
NormalizedInput normalize_union_input(const std::string &seq,
                                      const std::string &structure_g,
                                      const std::string &structure_gprime);

bool is_pk_free_structure(const std::string &db_full);
bool is_h_type_structure(const std::string &db_full);
std::string normalize_h_type_brackets(const std::string &db_full);

}  // namespace cparty

#endif  // CPARTY_FIXED_ENERGY_INPUT_HH
