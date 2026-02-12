#include "CPartyAPI.hh"

#include <string>
#include <type_traits>

int main() {
    using expected_cond_signature = double (*)(const std::string &, const std::string &);
    using expected_structure_signature = double (*)(const std::string &, const std::string &);
    using expected_structure_with_options_signature =
        double (*)(const std::string &, const std::string &, const cparty::EnergyEvalOptions &);

    constexpr expected_structure_signature structure_sig =
        static_cast<expected_structure_signature>(&get_structure_energy);
    constexpr expected_structure_with_options_signature structure_with_options_sig =
        static_cast<expected_structure_with_options_signature>(&get_structure_energy);
    (void)structure_sig;
    (void)structure_with_options_sig;

    static_assert(std::is_same<decltype(&get_cond_log_prob), expected_cond_signature>::value,
                  "get_cond_log_prob signature changed unexpectedly");
    return 0;
}
