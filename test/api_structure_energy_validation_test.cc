#include "fixed_energy_api.hh"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

void expect_invalid_argument(const std::string &seq, const std::string &db_full) {
  bool threw = false;
  try {
    (void)cparty::get_structure_energy(seq, db_full);
  } catch (const std::invalid_argument &) {
    threw = true;
  } catch (const std::exception &e) {
    std::cerr << "expected std::invalid_argument, got different exception: " << e.what() << "\n";
    std::exit(EXIT_FAILURE);
  }

  if (!threw) {
    std::cerr << "expected invalid_argument for seq='" << seq << "' db='" << db_full << "'\n";
    std::exit(EXIT_FAILURE);
  }
}

}  // namespace

int main() {
  const double energy = cparty::get_structure_energy("AUGC", "(())");
  if (!std::isfinite(energy)) {
    std::cerr << "expected finite energy for valid input\n";
    return EXIT_FAILURE;
  }
  if (energy != -2.0) {
    std::cerr << "unexpected deterministic score for valid input: " << energy << "\n";
    return EXIT_FAILURE;
  }

  expect_invalid_argument("AUTG", "(())");
  expect_invalid_argument("AUGC", "(()");
  expect_invalid_argument("AUGC", "(())..");
  expect_invalid_argument("AUGX", "(())");

  std::cout << "api_structure_energy_validation=ok\n";
  return EXIT_SUCCESS;
}
