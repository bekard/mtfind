#include <algorithm>

#include "chunk.hpp"
#include "mtfind_assert.hpp"

std::vector<chunk_range> split_into_chunks(size_t stream_size,
                                           size_t chunk_size,
                                           size_t mask_size) {
  chunk_size = std::max(chunk_size, mask_size);
  mtfind_assert(chunk_size > 0);

  std::vector<chunk_range> chunks;
  chunks.reserve((stream_size + chunk_size - 1) / chunk_size);

  for (size_t offset = 0; offset < stream_size; offset += chunk_size) {
    chunks.push_back({offset, std::min(chunk_size, stream_size - offset)});
  }

  return chunks;
}
