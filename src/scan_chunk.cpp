#include "scan_chunk.hpp"
#include "mtfind_assert.hpp"

bool match_at(const char *text, std::string_view mask) {
  for (size_t i = 0; i < mask.size(); ++i) {
    if (mask[i] != '?' && mask[i] != text[i]) {
      return false;
    }
  }
  return true;
}

chunk_result scan_chunk(std::string_view data, size_t owned_size,
                        std::string_view mask, size_t base_offset) {
  mtfind_assert(!mask.empty());
  mtfind_assert(mask.find('\n') == std::string_view::npos);
  mtfind_assert(owned_size <= data.size());

  chunk_result result;
  size_t line_start = 0;

  while (line_start < owned_size) {
    const size_t newline = data.find('\n', line_start);
    const size_t line_end =
        newline == std::string_view::npos ? data.size() : newline;

    for (size_t i = line_start; i < owned_size && i + mask.size() <= line_end;) {
      if (match_at(data.data() + i, mask)) {
        result.entries.push_back(
            {result.newline_count, i - line_start, base_offset + i});
        i += mask.size();
      } else {
        ++i;
      }
    }

    if (newline == std::string_view::npos || newline >= owned_size) {
      break;
    }

    ++result.newline_count;
    result.last_newline_offset = base_offset + newline;
    line_start = newline + 1;
  }

  return result;
}
