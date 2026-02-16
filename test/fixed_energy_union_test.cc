#include "fixed_energy_api.hh"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

void expect_invalid_union(const std::string &seq,
                          const std::string &g,
                          const std::string &gprime) {
  bool threw = false;
  try {
    (void)cparty::get_structure_energy_union(seq, g, gprime);
  } catch (const std::invalid_argument &) {
    threw = true;
  } catch (const std::exception &e) {
    std::cerr << "expected std::invalid_argument, got different exception: " << e.what() << "\n";
    std::exit(EXIT_FAILURE);
  }

  if (!threw) {
    std::cerr << "expected invalid_argument for seq='" << seq << "' G='" << g << "' G'='" << gprime << "'\n";
    std::exit(EXIT_FAILURE);
  }
}

}  // namespace

int main() {
  const std::string seq = "AUGCUA";
  const std::string g = "(....)";
  const std::string gprime = ".[..].";
  const std::string merged = "([..])";

  const double energy_union = cparty::get_structure_energy_union(seq, g, gprime);
  const double energy_merged = cparty::get_structure_energy(seq, merged);
  if (!std::isfinite(energy_union) || !std::isfinite(energy_merged)) {
    std::cerr << "expected finite energy for valid union input\n";
    return EXIT_FAILURE;
  }
  if (std::fabs(energy_union - energy_merged) > 1e-9) {
    std::cerr << "union energy mismatch: union=" << energy_union << " merged=" << energy_merged << "\n";
    return EXIT_FAILURE;
  }

  expect_invalid_union(seq, "(....)", "[....]");
  expect_invalid_union(seq, "(....)", "(....)");

  std::cout << "fixed_energy_union=ok\n";
  return EXIT_SUCCESS;
}
