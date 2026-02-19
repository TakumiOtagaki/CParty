#include "fixed_energy_input.hh"

#include <stdexcept>
#include <vector>

namespace cparty {

[[noreturn]] void fail_invalid_input(const std::string &reason) {
  throw std::invalid_argument("invalid fixed-structure input: " + reason);
}

static void validate_sequence(const std::string &seq) {
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

static void validate_structure(const std::string &db_full, const size_t expected_length) {
  if (db_full.empty()) {
    fail_invalid_input("structure is empty");
  }
  if (db_full.size() != expected_length) {
    fail_invalid_input("sequence/structure length mismatch");
  }

  std::vector<size_t> round_stack;
  std::vector<size_t> square_stack;
  round_stack.reserve(db_full.size());
  square_stack.reserve(db_full.size());

  for (size_t i = 0; i < db_full.size(); ++i) {
    const char c = db_full[i];
    if (c == '.') {
      continue;
    }
    if (c == '(') {
      round_stack.push_back(i);
      continue;
    }
    if (c == '[') {
      square_stack.push_back(i);
      continue;
    }
    if (c == ')') {
      if (round_stack.empty()) {
        fail_invalid_input("unbalanced structure: closing bracket without opener");
      }
      round_stack.pop_back();
      continue;
    }
    if (c == ']') {
      if (square_stack.empty()) {
        fail_invalid_input("unbalanced structure: closing bracket without opener");
      }
      square_stack.pop_back();
      continue;
    }
    fail_invalid_input("structure contains unsupported symbol at position " + std::to_string(i + 1));
  }

  if (!round_stack.empty() || !square_stack.empty()) {
    fail_invalid_input("unbalanced structure: missing closing bracket");
  }
}

static void validate_structure_subset(const std::string &db,
                                      const size_t expected_length,
                                      const char open_bracket,
                                      const char close_bracket,
                                      const std::string &label) {
  if (db.empty()) {
    fail_invalid_input(label + " is empty");
  }
  if (db.size() != expected_length) {
    fail_invalid_input("sequence/" + label + " length mismatch");
  }

  std::vector<size_t> stack;
  stack.reserve(db.size());

  for (size_t i = 0; i < db.size(); ++i) {
    const char c = db[i];
    if (c == '.') {
      continue;
    }
    if (c == open_bracket) {
      stack.push_back(i);
      continue;
    }
    if (c == close_bracket) {
      if (stack.empty()) {
        fail_invalid_input("unbalanced " + label + ": closing bracket without opener");
      }
      stack.pop_back();
      continue;
    }
    fail_invalid_input(label + " contains unsupported symbol at position " + std::to_string(i + 1));
  }

  if (!stack.empty()) {
    fail_invalid_input("unbalanced " + label + ": missing closing bracket");
  }
}

NormalizedInput normalize_union_input(const std::string &seq,
                                      const std::string &structure_g,
                                      const std::string &structure_gprime) {
  validate_sequence(seq);
  validate_structure_subset(structure_g, seq.size(), '(', ')', "G");
  validate_structure_subset(structure_gprime, seq.size(), '[', ']', "G'");

  std::string merged;
  merged.resize(seq.size(), '.');
  for (size_t i = 0; i < seq.size(); ++i) {
    const char g = structure_g[i];
    const char gp = structure_gprime[i];
    if (g != '.' && gp != '.') {
      fail_invalid_input("G and G' overlap at position " + std::to_string(i + 1));
    }
    if (g != '.') {
      merged[i] = g;
    } else if (gp != '.') {
      merged[i] = gp;
    }
  }

  return normalize_input(seq, merged);
}

NormalizedInput normalize_input(const std::string &seq, const std::string &db_full) {
  validate_sequence(seq);
  validate_structure(db_full, seq.size());

  NormalizedInput out;
  out.seq = seq;
  out.db_full = db_full;
  out.pair_map.assign(static_cast<size_t>(db_full.size()), -1);

  std::vector<int> round_stack;
  std::vector<int> square_stack;
  round_stack.reserve(db_full.size());
  square_stack.reserve(db_full.size());
  for (size_t idx = 0; idx < db_full.size(); ++idx) {
    const char c = db_full[idx];
    if (c == '(') {
      round_stack.push_back(static_cast<int>(idx));
      continue;
    }
    if (c == '[') {
      square_stack.push_back(static_cast<int>(idx));
      continue;
    }
    if (c == ')') {
      const int left = round_stack.back();
      round_stack.pop_back();
      out.pair_map[static_cast<size_t>(left)] = static_cast<int>(idx);
      out.pair_map[idx] = left;
      continue;
    }
    if (c == ']') {
      const int left = square_stack.back();
      square_stack.pop_back();
      out.pair_map[static_cast<size_t>(left)] = static_cast<int>(idx);
      out.pair_map[idx] = left;
    }
  }
  return out;
}

bool is_pk_free_structure(const std::string &db_full) {
  return db_full.find('[') == std::string::npos && db_full.find(']') == std::string::npos;
}

bool is_h_type_structure(const std::string &db_full) {
  const bool has_round = db_full.find('(') != std::string::npos || db_full.find(')') != std::string::npos;
  const bool has_square = db_full.find('[') != std::string::npos || db_full.find(']') != std::string::npos;
  return (!has_round && has_square);
}

std::string normalize_h_type_brackets(const std::string &db_full) {
  std::string out = db_full;
  for (char &c : out) {
    if (c == '[') {
      c = '(';
    } else if (c == ']') {
      c = ')';
    }
  }
  return out;
}

}  // namespace cparty
