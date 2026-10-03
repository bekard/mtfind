#include <gtest/gtest.h>

#include <stdexcept>
#include <string>

#include "chunk_worker.hpp"
#include "scan_chunk.hpp"

namespace {

// Claims the stream is bigger than it is, so reads come up short.
class short_stream_provider final : public stream_provider_iface {
public:
  stream_ptr create_stream() const override {
    return std::make_unique<std::istringstream>("abc");
  }
  size_t get_stream_size() const override { return 10; }
};

} // namespace

TEST(chunk_worker_tests, whole_stream) {
  const istringstream_provider provider("x\nbad");
  chunk_worker worker(provider, "?ad");

  EXPECT_EQ(worker.process({0, 5}), scan_chunk("x\nbad", 5, "?ad", 0));
}

TEST(chunk_worker_tests, reads_lookahead) {
  // Owned: "xxba", the rest of the match is in the next chunk.
  const istringstream_provider provider("xxbad\nyy");
  chunk_worker worker(provider, "bad");

  const chunk_result res = worker.process({0, 4});

  EXPECT_EQ(res.entries, (std::vector<entry>{{0, 2, 2}}));
}

TEST(chunk_worker_tests, lookahead_is_clamped_at_stream_end) {
  const istringstream_provider provider("xxba");
  chunk_worker worker(provider, "bad");

  const chunk_result res = worker.process({2, 2});

  EXPECT_TRUE(res.entries.empty());
}

TEST(chunk_worker_tests, chunk_in_the_middle) {
  const istringstream_provider provider("aa\nbad\ncc");
  chunk_worker worker(provider, "?ad");

  const chunk_result res = worker.process({2, 4});

  EXPECT_EQ(res.newline_count, 1u);
  EXPECT_EQ(res.last_newline_offset, 2u);
  EXPECT_EQ(res.entries, (std::vector<entry>{{1, 0, 3}}));
}

TEST(chunk_worker_tests, reused_for_chunks_in_any_order) {
  const std::string text = "bad\nmad\nhad\n";
  const istringstream_provider provider(text);
  chunk_worker worker(provider, "?ad");

  for (const chunk_range range : {chunk_range{8, 4}, chunk_range{0, 4},
                                  chunk_range{4, 4}, chunk_range{0, 4}}) {
    EXPECT_EQ(worker.process(range),
              scan_chunk(std::string_view(text).substr(range.offset), range.size,
                         "?ad", range.offset));
  }
}

TEST(chunk_worker_tests, throws_on_short_read) {
  const short_stream_provider provider;
  chunk_worker worker(provider, "a");

  EXPECT_THROW(worker.process({0, 10}), std::runtime_error);
}

TEST(process_chunks_tests, no_chunks) {
  const istringstream_provider provider("");

  EXPECT_TRUE(process_chunks(provider, "?ad", {}, 4).empty());
}

TEST(process_chunks_tests, same_as_scanning_each_chunk) {
  const std::string text = "I've paid my dues\n"
                           "Time after time.\n"
                           "And bad mistakes ?\n"
                           "I've made a few.\n"
                           "I've had my share of sand kicked in my face\n"
                           "aaaaaaaaaaaaaaaaaaaaaaaaaa\n"
                           "\n\n"
                           "But I've come through.";
  const istringstream_provider provider(text);

  for (const std::string mask : {"?ad", "aa", "?", "I've", "????????"}) {
    for (size_t chunk_size = 1; chunk_size <= text.size(); ++chunk_size) {
      const std::vector<chunk_range> chunks = split_into_chunks(text.size(), chunk_size, mask.size());

      std::vector<chunk_result> expected;
      for (const chunk_range &range : chunks) {
        expected.push_back(scan_chunk(std::string_view(text).substr(range.offset),
                                      range.size, mask, range.offset));
      }

      for (size_t threads_count : {1, 2, 3, 8}) {
        EXPECT_EQ(process_chunks(provider, mask, chunks, threads_count),
                  expected)
            << "mask: " << mask << ", chunk_size: " << chunk_size
            << ", threads_count: " << threads_count;
      }
    }
  }
}

TEST(process_chunks_tests, rethrows_worker_error) {
  const short_stream_provider provider;

  EXPECT_THROW(process_chunks(provider, "a", {{0, 5}, {5, 5}}, 2),
               std::runtime_error);
}
