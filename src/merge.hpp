#pragma once

#include <vector>

#include "chunk.hpp"
#include "chunk_worker.hpp"
#include "results.hpp"

// Combines the results of consecutive chunks (in stream order) into final
// entries with 1-based line numbers and positions.
//
// Chunks are scanned independently, so the first matches of a chunk may
// overlap the last match of the previous one. Such matches are dropped, and
// the part of the line they shadowed is rescanned with `worker`, so the result
// is the same as scanning the whole stream at once.
results merge_chunks(std::vector<chunk_result> chunks, chunk_worker &worker);
