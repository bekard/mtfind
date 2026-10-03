#include <algorithm>
#include <atomic>
#include <future>
#include <stdexcept>

#include "chunk_worker.hpp"
#include "mtfind_assert.hpp"
#include "scan_chunk.hpp"

chunk_worker::chunk_worker(const stream_provider_iface &stream_provider,
                           const std::string &the_mask)
    : stream(stream_provider.create_stream()),
      stream_size(stream_provider.get_stream_size()), mask(the_mask) {
  mtfind_assert(stream != nullptr);
  mtfind_assert(!mask.empty());
}

chunk_result chunk_worker::process(const chunk_range &range) {
  mtfind_assert(range.offset + range.size <= stream_size);

  const size_t chunk_end = range.offset + range.size;
  const size_t lookahead = std::min(mask.size() - 1, stream_size - chunk_end);
  const size_t read_size = range.size + lookahead;

  buffer.resize(read_size);

  stream->clear();
  stream->seekg(static_cast<std::streamoff>(range.offset));
  stream->read(buffer.data(), static_cast<std::streamsize>(read_size));

  if (static_cast<size_t>(stream->gcount()) != read_size) {
    throw std::runtime_error("failed to read " + std::to_string(read_size) +
                             " bytes at offset " +
                             std::to_string(range.offset));
  }

  return scan_chunk(std::string_view(buffer.data(), read_size), range.size,
                    mask, range.offset);
}

std::vector<chunk_result>
process_chunks(const stream_provider_iface &stream_provider,
               const std::string &mask, const std::vector<chunk_range> &chunks,
               size_t threads_count) {
  mtfind_assert(threads_count > 0);

  std::vector<chunk_result> results(chunks.size());
  std::atomic<size_t> next_chunk = 0;

  auto work = [&] {
    chunk_worker worker(stream_provider, mask);
    for (size_t i = next_chunk++; i < chunks.size(); i = next_chunk++) {
      results[i] = worker.process(chunks[i]);
    }
  };

  std::vector<std::future<void>> futures;
  for (size_t i = 0; i < std::min(threads_count, chunks.size()); ++i) {
    futures.push_back(std::async(std::launch::async, work));
  }

  // Wait for all threads before rethrowing: they reference local variables.
  for (std::future<void> &future : futures) {
    future.wait();
  }
  for (std::future<void> &future : futures) {
    future.get();
  }

  return results;
}
