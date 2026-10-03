#include <gtest/gtest.h>

#include <ostream>
#include <random>
#include <string>

#include "chunk_worker.hpp"
#include "merge.hpp"
#include "test_utils.hpp"

namespace {

results find_in_chunks(const std::string &text, const std::string &mask,
                       size_t chunk_size, size_t threads_count = 2) {
  const istringstream_provider provider(text);
  chunk_worker worker(provider, mask);

  return merge_chunks(
      process_chunks(provider, mask,
                     split_into_chunks(text.size(), chunk_size, mask.size()),
                     threads_count),
      worker);
}

std::string random_string(std::mt19937 &rng, std::string_view alphabet,
                          size_t size) {
  std::uniform_int_distribution<size_t> pick(0, alphabet.size() - 1);
  std::string result(size, ' ');
  for (char &c : result) {
    c = alphabet[pick(rng)];
  }
  return result;
}

} // namespace

TEST(merge_tests, no_chunks) {
  const istringstream_provider provider("");
  chunk_worker worker(provider, "?ad");

  EXPECT_EQ(merge_chunks({}, worker), (results{.mask_size = 3, .entries = {}}));
}

TEST(merge_tests, spec_example) {
  const std::string text = "I've paid my dues\n"
                           "Time after time.\n"
                           "I've done my sentence\n"
                           "But committed no crime.\n"
                           "And bad mistakes ?\n"
                           "I've made a few.\n"
                           "I've had my share of sand kicked in my face\n"
                           "But I've come through.\n";
  const std::vector<entry> expected = {
      {5, 5, text.find("bad")},
      {6, 6, text.find("mad")},
      {7, 6, text.find("had")},
  };

  for (size_t chunk_size : {3, 7, 16, 1000}) {
    EXPECT_EQ(find_in_chunks(text, "?ad", chunk_size).entries, expected)
        << "chunk_size: " << chunk_size;
  }
}

TEST(merge_tests, line_spans_many_chunks) {
  const results res = find_in_chunks("a\nxxxxxxxxxxbad", "?ad", 3);

  EXPECT_EQ(res.entries, (std::vector<entry>{{2, 11, 12}}));
}

TEST(merge_tests, overlap_at_boundary_is_dropped) {
  // Chunks: "aaa|aa". The first chunk takes [0, 2) and [2, 4) using its
  // lookahead; the second chunk's [3, 5) overlaps and must be dropped.
  const results res = find_in_chunks("aaaaa", "aa", 3);

  EXPECT_EQ(res.entries, (std::vector<entry>{{1, 1, 0}, {1, 3, 2}}));
}

TEST(merge_tests, shadowed_range_is_rescanned) {
  // Chunks: "aaaa|aaaa|a". The second chunk finds [4, 7), which overlaps
  // [3, 6) from the first chunk. Dropping it uncovers the match at 6.
  const results res = find_in_chunks("aaaaaaaaa", "aaa", 4);

  EXPECT_EQ(res.entries,
            (std::vector<entry>{{1, 1, 0}, {1, 4, 3}, {1, 7, 6}}));
}

TEST(merge_tests, same_as_reference_on_random_input) {
  std::mt19937 rng(42);

  for (int iteration = 0; iteration < 300; ++iteration) {
    const std::string text =
        random_string(rng, "aaab\n", std::uniform_int_distribution<size_t>(
                                         0, 60)(rng));
    const std::string mask = random_string(
        rng, "aab?", std::uniform_int_distribution<size_t>(1, 5)(rng));
    const std::vector<entry> expected = reference_find(text, mask);

    for (size_t chunk_size = 1; chunk_size <= text.size() + 1; ++chunk_size) {
      ASSERT_EQ(find_in_chunks(text, mask, chunk_size).entries, expected)
          << "text: \"" << text << "\", mask: \"" << mask
          << "\", chunk_size: " << chunk_size;
    }
  }
}
