#include <gtest/gtest.h>

#include "chunk.hpp"

using chunks = std::vector<chunk_range>;

TEST(split_into_chunks_tests, empty_stream) {
  EXPECT_TRUE(split_into_chunks(0, 16, 3).empty());
}

TEST(split_into_chunks_tests, stream_smaller_than_chunk) {
  EXPECT_EQ(split_into_chunks(5, 16, 3), (chunks{{0, 5}}));
}

TEST(split_into_chunks_tests, exact_multiple) {
  EXPECT_EQ(split_into_chunks(12, 4, 3), (chunks{{0, 4}, {4, 4}, {8, 4}}));
}

TEST(split_into_chunks_tests, last_chunk_takes_remainder) {
  EXPECT_EQ(split_into_chunks(10, 4, 3), (chunks{{0, 4}, {4, 4}, {8, 2}}));
}

TEST(split_into_chunks_tests, chunk_size_is_at_least_mask_size) {
  EXPECT_EQ(split_into_chunks(10, 2, 4), (chunks{{0, 4}, {4, 4}, {8, 2}}));
}

TEST(split_into_chunks_tests, chunks_cover_stream_contiguously) {
  for (size_t stream_size = 0; stream_size <= 50; ++stream_size) {
    for (size_t chunk_size = 1; chunk_size <= 12; ++chunk_size) {
      const chunks res = split_into_chunks(stream_size, chunk_size, 1);

      size_t offset = 0;
      for (const chunk_range &range : res) {
        EXPECT_EQ(range.offset, offset);
        EXPECT_GT(range.size, 0u);
        EXPECT_LE(range.size, chunk_size);
        offset += range.size;
      }
      EXPECT_EQ(offset, stream_size);
    }
  }
}
