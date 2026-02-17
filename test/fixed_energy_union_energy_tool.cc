#include "fixed_energy_api.hh"

#include <iomanip>
#include <iostream>
#include <stdexcept>

int main(int argc, char **argv) {
  if (argc != 4) {
    std::cerr << "usage: fixed_energy_union_energy_tool <seq> <G> <G'>\n";
    return 2;
  }

  try {
    const std::string seq = argv[1];
    const std::string g = argv[2];
    const std::string gprime = argv[3];
    const double energy = cparty::get_structure_energy_union(seq, g, gprime);
    std::cout << std::setprecision(17) << energy << "\n";
    return 0;
  } catch (const std::exception &e) {
    std::cerr << "fixed_energy_union_energy_tool failed: " << e.what() << "\n";
    return 1;
  }
}
