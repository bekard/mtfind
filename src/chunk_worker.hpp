#pragma once

#include <string>
#include <vector>

#include "chunk.hpp"
#include "stream_provider.hpp"

// Reads chunks of a stream and scans them for the mask. Owns its own stream
// and a buffer that is reused between chunks, so it is meant to live in a
// single thread and process many chunks.
class chunk_worker {
public:
  chunk_worker(const stream_provider_iface &stream_provider,
               const std::string &mask);

  // Reads the range plus up to mask_size - 1 bytes of lookahead and scans it.
  // Throws std::runtime_error if the stream can't be read.
  chunk_result process(const chunk_range &range);

  size_t mask_size() const { return mask.size(); }

private:
  stream_ptr stream;
  size_t stream_size;
  std::string mask;
  std::vector<char> buffer;
};

// Processes the chunks on a pool of threads_count threads. Each thread takes
// the next unprocessed chunk until none are left. The results are in the
// same order as the chunks.
std::vector<chunk_result>
process_chunks(const stream_provider_iface &stream_provider,
               const std::string &mask, const std::vector<chunk_range> &chunks,
               size_t threads_count);
