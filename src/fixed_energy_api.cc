#include "fixed_energy_api.hh"

#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

namespace cparty {
namespace {

[[noreturn]] void fail_invalid_input(const std::string &reason) {
  throw std::invalid_argument("invalid fixed-structure input: " + reason);
}

void validate_sequence(const std::string &seq) {
  if (seq.empty()) {
    fail_invalid_input("sequence is empty");
  }

  for (size_t i = 0; i < seq.size(); ++i) {
    const char c = seq[i];
    if (c == 'A' || c == 'U' || c == 'G' || c == 'C') {
      continue;
    }
    if (c == 'T') {
      fail_invalid_input("sequence contains T at position " + std::to_string(i + 1));
    }
    fail_invalid_input("sequence contains non-AUGC base at position " + std::to_string(i + 1));
  }
}

void validate_structure(const std::string &db_full, const size_t expected_length) {
  if (db_full.empty()) {
    fail_invalid_input("structure is empty");
  }
  if (db_full.size() != expected_length) {
    fail_invalid_input("sequence/structure length mismatch");
  }

  std::vector<size_t> stack;
  stack.reserve(db_full.size());

  for (size_t i = 0; i < db_full.size(); ++i) {
    const char c = db_full[i];
    if (c == '.') {
      continue;
    }
    if (c == '(') {
      stack.push_back(i);
      continue;
    }
    if (c == ')') {
      if (stack.empty()) {
        fail_invalid_input("unbalanced structure: closing bracket without opener");
      }
      stack.pop_back();
      continue;
    }
    fail_invalid_input("structure contains unsupported symbol at position " + std::to_string(i + 1));
  }

  if (!stack.empty()) {
    fail_invalid_input("unbalanced structure: missing closing bracket");
  }
}

}  // namespace

double get_structure_energy(const std::string &seq, const std::string &db_full) {
  validate_sequence(seq);
  validate_structure(db_full, seq.size());

  // Placeholder deterministic score for story 011; parser/rule scoring is added in later stories.
  int pairs = 0;
  for (char c : db_full) {
    if (c == '(') {
      ++pairs;
    }
  }
  return -1.0 * static_cast<double>(pairs);
}

}  // namespace cparty
