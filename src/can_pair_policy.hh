#ifndef CAN_PAIR_POLICY_HH
#define CAN_PAIR_POLICY_HH

namespace cparty {
namespace can_pair_policy {

// Shared hard-constraint policy for canonical/wobble RNA base pairing.
bool is_allowed_base_pair(char left, char right) noexcept;

// Shared helper for recurrence guards that rely on tree.up span coverage.
bool is_tree_up_pairable(int tree_up_value, int span) noexcept;

} // namespace can_pair_policy
} // namespace cparty

#endif
