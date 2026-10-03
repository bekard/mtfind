#include <algorithm>
#include <thread>

#include "async_find.hpp"
#include "chunk_worker.hpp"
#include "merge.hpp"
#include "mtfind_assert.hpp"

results async_find(const std::string &mask,
                   const stream_provider_ptr &stream_provider,
                   size_t chunk_size, size_t threads_count) {
  mtfind_assert(stream_provider != nullptr);

  if (threads_count == 0) {
    threads_count = std::max(1u, std::thread::hardware_concurrency());
  }

  const std::vector<chunk_range> chunks = split_into_chunks(
      stream_provider->get_stream_size(), chunk_size, mask.size());

  std::vector<chunk_result> chunk_results =
      process_chunks(*stream_provider, mask, chunks, threads_count);

  chunk_worker rescan_worker(*stream_provider, mask);
  return merge_chunks(std::move(chunk_results), rescan_worker);
}
