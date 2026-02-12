#include "CPartyAPI.hh"

#include <string>
#include <type_traits>

int main() {
    using expected_signature = double (*)(const std::string &, const std::string &);
    static_assert(std::is_same<decltype(&get_structure_energy), expected_signature>::value,
                  "get_structure_energy signature mismatch");
    static_assert(std::is_same<decltype(&get_cond_log_prob), expected_signature>::value,
                  "get_cond_log_prob signature changed unexpectedly");
    return 0;
}
