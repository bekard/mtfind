#pragma once

#include <cstddef>
#include <optional>
#include <vector>

#include "results.hpp"

// Chunks are processed by a pool of threads; each thread holds one chunk
// (plus lookahead) in memory at a time.
inline constexpr size_t default_chunk_size = 16 * 1024 * 1024;

// A byte range of the stream processed by a single worker. The worker may read
// up to mask_size - 1 bytes past the end to finish matches that start inside.
struct chunk_range {
  size_t offset;
  size_t size;

  bool operator==(const chunk_range &another) const = default;
};

// Splits a stream into consecutive chunks of chunk_size bytes; the last chunk
// takes the remainder. The chunk size is raised to at least mask_size, so a
// match never spans more than two chunks.
std::vector<chunk_range> split_into_chunks(size_t stream_size,
                                           size_t chunk_size,
                                           size_t mask_size);

// What a worker knows after scanning its range, before the line numbers of the
// previous chunks are known.
//
// Entries are relative to the chunk:
// - line is the number of '\n' in the chunk before the match (0-based);
// - pos is 0-based, counted from the line start, or from the chunk start
//   if line == 0, because that line may have started in a previous chunk;
// - offset is already absolute.
// The merge step turns them into final entries.
struct chunk_result {
  size_t newline_count = 0;
  std::optional<size_t> last_newline_offset; // absolute
  std::vector<entry> entries;

  bool operator==(const chunk_result &another) const = default;
};
