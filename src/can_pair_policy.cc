#include "can_pair_policy.hh"

#include <cctype>

namespace cparty {
namespace can_pair_policy {

namespace {

char to_upper_base(char c) noexcept {
    return static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
}

} // namespace

bool is_allowed_base_pair(char left, char right) noexcept {
    left = to_upper_base(left);
    right = to_upper_base(right);
    return (left == 'A' && right == 'U') || (left == 'U' && right == 'A') || (left == 'C' && right == 'G') || (left == 'G' && right == 'C') ||
           (left == 'G' && right == 'U') || (left == 'U' && right == 'G');
}

bool is_tree_up_pairable(int tree_up_value, int span) noexcept { return tree_up_value >= span; }

} // namespace can_pair_policy
} // namespace cparty
