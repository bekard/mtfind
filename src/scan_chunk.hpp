#pragma once

#include <string_view>

#include "chunk.hpp"

// Returns true if `mask` matches the text starting at `text`. '?' in the mask
// matches any character. The caller guarantees that at least mask.size() bytes
// are readable and that they don't contain '\n'.
bool match_at(const char *text, std::string_view mask);

// Finds non-overlapping matches of `mask` in `data`, greedily from left to
// right, line by line.
//
// Only data[0, owned_size) belongs to the chunk: matches must start there and
// only newlines there are counted. The rest of `data` is lookahead that lets
// matches starting near the end of the chunk complete.
//
// `base_offset` is the offset of data[0] in the stream. See chunk_result for
// the meaning of the returned entries.
chunk_result scan_chunk(std::string_view data, size_t owned_size,
                        std::string_view mask, size_t base_offset);
