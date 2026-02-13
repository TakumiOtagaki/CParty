#include "scfg/constraint_oracle.hh"
#include "sparse_tree.hh"

#include <cstdlib>
#include <iostream>

int main() {
  {
    sparse_tree tree("......", 6);
    if (!scfg::is_empty_region(tree, 1, 6)) {
      std::cerr << "expected empty region (1,6) for unpaired structure\n";
      return EXIT_FAILURE;
    }
    if (!scfg::is_empty_region(tree, 3, 4)) {
      std::cerr << "expected empty single-gap region (3,4)\n";
      return EXIT_FAILURE;
    }
  }

  {
    sparse_tree tree("(..())", 6);
    if (scfg::is_empty_region(tree, 1, 6)) {
      std::cerr << "expected non-empty region (1,6) due to nested pair\n";
      return EXIT_FAILURE;
    }
    if (!scfg::is_unpaired_position(tree, 3)) {
      std::cerr << "expected position 3 to be unpaired\n";
      return EXIT_FAILURE;
    }
    if (scfg::is_unpaired_position(tree, 1)) {
      std::cerr << "expected position 1 to be paired\n";
      return EXIT_FAILURE;
    }
  }

  if (!scfg::is_pair_type_allowed(static_cast<pair_type>(1))) {
    std::cerr << "expected pair type 1 to be allowed\n";
    return EXIT_FAILURE;
  }
  if (scfg::is_pair_type_allowed(static_cast<pair_type>(0))) {
    std::cerr << "expected pair type 0 to be disallowed\n";
    return EXIT_FAILURE;
  }

  std::cout << "can_pair_policy_checks=ok\n";
  return EXIT_SUCCESS;
}
