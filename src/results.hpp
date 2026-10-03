#pragma once

#include <cstddef>
#include <istream>
#include <ostream>
#include <vector>

// A single match. The matched text itself is not stored: it is always
// results::mask_size bytes starting at `offset` and is read back when printing.
struct entry {
  size_t line;   // 1-based line number
  size_t pos;    // 1-based position in the line
  size_t offset; // 0-based offset of the match in the stream

  bool operator==(const entry &another) const = default;
};

struct results {
  size_t mask_size = 0;
  std::vector<entry> entries;

  bool operator==(const results &another) const = default;
};

// Prints the number of entries, then "line pos text" for each entry. The text
// of each entry is read from `source`, the stream that was searched.
void print_results(const results &res, std::istream &source, std::ostream &out);
