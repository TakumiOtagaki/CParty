#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

struct Entry {
  int row = 0;
  std::string id;
  std::string family;
  std::string expected;
  std::string reason;
  std::string sequence;
  std::string structure;
};

std::string trim(const std::string &s) {
  const auto start = s.find_first_not_of(" \t\r\n");
  if (start == std::string::npos) {
    return "";
  }
  const auto end = s.find_last_not_of(" \t\r\n");
  return s.substr(start, end - start + 1);
}

std::vector<std::string> split(const std::string &s, const char delimiter) {
  std::vector<std::string> out;
  std::stringstream ss(s);
  std::string item;
  while (std::getline(ss, item, delimiter)) {
    out.push_back(item);
  }
  return out;
}

bool is_blank(const std::string &s) { return trim(s).empty(); }

Entry parse_header(const std::string &line, int row) {
  Entry entry;
  entry.row = row;
  const auto parts = split(line, '|');
  if (parts.empty()) {
    throw std::runtime_error("empty header");
  }

  const std::string id_part = trim(parts[0]);
  if (id_part.empty() || id_part[0] != '>') {
    throw std::runtime_error("header must start with >");
  }
  entry.id = trim(id_part.substr(1));
  if (entry.id.empty()) {
    throw std::runtime_error("header id is empty");
  }

  for (size_t i = 1; i < parts.size(); ++i) {
    const std::string kv = trim(parts[i]);
    if (kv.empty()) {
      continue;
    }
    const auto eq_pos = kv.find('=');
    if (eq_pos == std::string::npos) {
      throw std::runtime_error("metadata token must be key=value: " + kv);
    }
    const std::string key = trim(kv.substr(0, eq_pos));
    const std::string value = trim(kv.substr(eq_pos + 1));
    if (key == "family") {
      entry.family = value;
    } else if (key == "expected") {
      entry.expected = value;
    } else if (key == "reason") {
      entry.reason = value;
    }
  }

  if (entry.family.empty() || entry.expected.empty()) {
    throw std::runtime_error("required metadata missing (family/expected)");
  }

  return entry;
}

std::vector<Entry> parse_multi_secstruct(const std::string &path) {
  std::ifstream in(path);
  if (!in) {
    throw std::runtime_error("failed to open: " + path);
  }

  std::vector<std::string> lines;
  std::string line;
  while (std::getline(in, line)) {
    lines.push_back(line);
  }

  std::vector<Entry> entries;
  size_t i = 0;
  while (i < lines.size()) {
    while (i < lines.size() && is_blank(lines[i])) {
      ++i;
    }
    if (i >= lines.size()) {
      break;
    }

    if (trim(lines[i]).empty() || trim(lines[i])[0] != '>') {
      throw std::runtime_error("record must start with header at source line " + std::to_string(i + 1));
    }

    Entry entry = parse_header(lines[i], static_cast<int>(entries.size() + 1));
    ++i;

    while (i < lines.size() && is_blank(lines[i])) {
      ++i;
    }
    if (i >= lines.size()) {
      throw std::runtime_error("missing sequence for row " + std::to_string(entry.row));
    }
    entry.sequence = trim(lines[i]);
    ++i;

    while (i < lines.size() && is_blank(lines[i])) {
      ++i;
    }
    if (i >= lines.size()) {
      throw std::runtime_error("missing structure for row " + std::to_string(entry.row));
    }
    entry.structure = trim(lines[i]);
    ++i;

    if (entry.sequence.empty() || entry.structure.empty()) {
      throw std::runtime_error("empty sequence/structure at row " + std::to_string(entry.row));
    }

    entries.push_back(entry);
  }

  if (entries.empty()) {
    throw std::runtime_error("no entries parsed");
  }

  return entries;
}

std::vector<Entry> load_fixture(const std::string &path) {
  std::ifstream in(path);
  if (!in) {
    throw std::runtime_error("failed to open fixture: " + path);
  }

  std::string header;
  if (!std::getline(in, header)) {
    throw std::runtime_error("fixture is empty");
  }

  std::vector<Entry> rows;
  std::string line;
  while (std::getline(in, line)) {
    if (trim(line).empty()) {
      continue;
    }
    const auto cols = split(line, '\t');
    if (cols.size() != 7) {
      throw std::runtime_error("fixture must have 7 columns, got " + std::to_string(cols.size()));
    }
    Entry e;
    e.row = std::stoi(cols[0]);
    e.id = cols[1];
    e.family = cols[2];
    e.expected = cols[3];
    e.reason = cols[4];
    e.sequence = cols[5];
    e.structure = cols[6];
    rows.push_back(e);
  }

  if (rows.empty()) {
    throw std::runtime_error("fixture has no data rows");
  }

  return rows;
}

void compare(const std::vector<Entry> &actual, const std::vector<Entry> &expected) {
  if (actual.size() != expected.size()) {
    throw std::runtime_error("row count drift: actual=" + std::to_string(actual.size()) + " expected=" +
                             std::to_string(expected.size()));
  }

  for (size_t i = 0; i < expected.size(); ++i) {
    const Entry &a = actual[i];
    const Entry &e = expected[i];
    if (a.row != e.row || a.id != e.id || a.family != e.family || a.expected != e.expected ||
        a.reason != e.reason || a.sequence != e.sequence || a.structure != e.structure) {
      std::ostringstream oss;
      oss << "fixture mismatch at row " << (i + 1) << ":\n"
          << "  actual   = [" << a.row << ", " << a.id << ", " << a.family << ", " << a.expected << ", "
          << a.reason << ", " << a.sequence << ", " << a.structure << "]\n"
          << "  expected = [" << e.row << ", " << e.id << ", " << e.family << ", " << e.expected << ", "
          << e.reason << ", " << e.sequence << ", " << e.structure << "]";
      throw std::runtime_error(oss.str());
    }
  }
}

}  // namespace

int main(int argc, char **argv) {
  if (argc != 3) {
    std::cerr << "usage: multi_secstruct_schema_test <multi.secstruct> <fixture.tsv>\n";
    return EXIT_FAILURE;
  }

  try {
    const auto actual = parse_multi_secstruct(argv[1]);
    const auto expected = load_fixture(argv[2]);
    compare(actual, expected);
    std::cout << "multi_secstruct_rows=" << actual.size() << "\n";
    return EXIT_SUCCESS;
  } catch (const std::exception &e) {
    std::cerr << "multi_secstruct_schema_test failed: " << e.what() << "\n";
    return EXIT_FAILURE;
  }
}
