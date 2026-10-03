#pragma once

#include <string>

#include "chunk.hpp"
#include "results.hpp"
#include "stream_provider.hpp"

// Finds non-overlapping matches of the mask in the stream. The stream is split
// into chunks of chunk_size bytes that are scanned on threads_count threads
// (0 means one per hardware thread).
results async_find(const std::string &mask,
                   const stream_provider_ptr &stream_provider,
                   size_t chunk_size = default_chunk_size,
                   size_t threads_count = 0);
