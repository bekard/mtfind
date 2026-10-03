#include <algorithm>

#include "merge.hpp"

results merge_chunks(std::vector<chunk_result> chunks, chunk_worker &worker) {
  const size_t mask_size = worker.mask_size();

  results res{.mask_size = mask_size, .entries = {}};

  size_t line_base = 1;  // line number of the current chunk's first line
  size_t line_start = 0; // offset where that line starts

  // Every match start before this offset is decided: either accepted, or
  // rejected. It is never before the end of the last accepted entry.
  size_t checked = 0;

  for (chunk_result &chunk : chunks) {
    for (entry e : chunk.entries) {
      if (e.line == 0) {
        e.pos = e.offset - line_start;
      }
      e.line += line_base;
      e.pos += 1;

      if (e.offset >= checked) {
        res.entries.push_back(e);
        checked = e.offset + mask_size;
        continue;
      }

      // The entry overlaps an accepted one, so drop it. Its chunk skipped
      // the starts in (e.offset, e.offset + mask_size) because of it; rescan
      // those that are not decided yet. They lie within the dropped entry,
      // so on the same line.
      const size_t shadow_end = e.offset + mask_size;
      if (checked >= shadow_end) {
        continue;
      }

      for (const entry &found :
           worker.process({checked, shadow_end - checked}).entries) {
        res.entries.push_back(
            {e.line, e.pos + (found.offset - e.offset), found.offset});
        checked = found.offset + mask_size;
      }
      checked = std::max(checked, shadow_end);
    }

    line_base += chunk.newline_count;
    if (chunk.last_newline_offset) {
      line_start = *chunk.last_newline_offset + 1;
    }

    chunk.entries = {}; // free memory early
  }

  return res;
}
