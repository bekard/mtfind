#pragma once

#include <ostream>
#include <string>
#include <vector>

#include "results.hpp"

inline std::ostream &operator<<(std::ostream &os, const entry &e) {
  return os << "{" << e.line << ", " << e.pos << ", " << e.offset << "}";
}

// Straightforward single-threaded search, independent from scan_chunk.
inline std::vector<entry> reference_find(const std::string &text,
                                         const std::string &mask) {
  std::vector<entry> result;
  size_t line = 1;
  size_t line_start = 0;

  for (size_t i = 0; i < text.size();) {
    if (text[i] == '\n') {
      ++line;
      line_start = ++i;
      continue;
    }

    bool matches = i + mask.size() <= text.size();
    for (size_t j = 0; matches && j < mask.size(); ++j) {
      const char c = text[i + j];
      matches = c != '\n' && (mask[j] == '?' || mask[j] == c);
    }

    if (matches) {
      result.push_back({line, i - line_start + 1, i});
      i += mask.size();
    } else {
      ++i;
    }
  }

  return result;
}
