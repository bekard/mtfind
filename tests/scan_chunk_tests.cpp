#include <gtest/gtest.h>

#include <ostream>
#include <string>

#include "scan_chunk.hpp"
#include "test_utils.hpp"

namespace {

// Scans the whole text as a single chunk starting at the stream beginning.
chunk_result scan_all(std::string_view text, std::string_view mask) {
  return scan_chunk(text, text.size(), mask, 0);
}

} // namespace

TEST(match_at_tests, exact) {
  EXPECT_TRUE(match_at("bad", "bad"));
  EXPECT_FALSE(match_at("bad", "bag"));
}

TEST(match_at_tests, wildcard) {
  EXPECT_TRUE(match_at("bad", "?ad"));
  EXPECT_TRUE(match_at("mad", "?ad"));
  EXPECT_TRUE(match_at("b d", "b?d"));
  EXPECT_TRUE(match_at("xyz", "???"));
  EXPECT_FALSE(match_at("bab", "?ad"));
}

TEST(match_at_tests, case_sensitive) { EXPECT_FALSE(match_at("BAD", "bad")); }

TEST(match_at_tests, question_mark_in_text) {
  EXPECT_TRUE(match_at("?", "?"));
  EXPECT_FALSE(match_at("?", "a"));
}

TEST(scan_chunk_tests, empty_data) {
  const chunk_result res = scan_all("", "?ad");

  EXPECT_EQ(res, chunk_result{});
}

TEST(scan_chunk_tests, spec_example) {
  const std::string text = "I've paid my dues\n"
                           "Time after time.\n"
                           "I've done my sentence\n"
                           "But committed no crime.\n"
                           "And bad mistakes ?\n"
                           "I've made a few.\n"
                           "I've had my share of sand kicked in my face\n"
                           "But I've come through.\n";

  const chunk_result res = scan_all(text, "?ad");

  EXPECT_EQ(res.newline_count, 8u);
  EXPECT_EQ(res.last_newline_offset, text.size() - 1);
  EXPECT_EQ(res.entries, (std::vector<entry>{
                             {4, 4, text.find("bad")},
                             {5, 5, text.find("mad")},
                             {6, 5, text.find("had")},
                         }));
}

TEST(scan_chunk_tests, multiple_entries_in_line) {
  const chunk_result res = scan_all("bad  ad trade", "?ad");

  EXPECT_EQ(res.entries, (std::vector<entry>{
                             {0, 0, 0},
                             {0, 4, 4},
                             {0, 9, 9},
                         }));
}

TEST(scan_chunk_tests, overlapping_entries) {
  EXPECT_EQ(scan_all("aaaa", "aa").entries,
            (std::vector<entry>{{0, 0, 0}, {0, 2, 2}}));
  EXPECT_EQ(scan_all("aaaaa", "aa").entries,
            (std::vector<entry>{{0, 0, 0}, {0, 2, 2}}));
  EXPECT_EQ(scan_all("aaaaa", "aaa").entries, (std::vector<entry>{{0, 0, 0}}));
  EXPECT_EQ(scan_all("abab", "?b?").entries, (std::vector<entry>{{0, 0, 0}}));
}

TEST(scan_chunk_tests, overlapping_resets_at_newline) {
  // The match at 0 would overlap the one at 1, but the next line starts fresh.
  EXPECT_EQ(scan_all("aaa\naa", "aa").entries,
            (std::vector<entry>{{0, 0, 0}, {1, 0, 4}}));
}

TEST(scan_chunk_tests, match_does_not_cross_newline) {
  EXPECT_TRUE(scan_all("ab\nc", "abc").entries.empty());
  EXPECT_TRUE(scan_all("ab\nc", "ab?c").entries.empty());
  EXPECT_TRUE(scan_all("a\nb", "???").entries.empty());
  EXPECT_TRUE(scan_all("a\nb", "a?").entries.empty());
}

TEST(scan_chunk_tests, mask_longer_than_line) {
  const chunk_result res = scan_all("ab\nabc\nab", "abc");

  EXPECT_EQ(res.newline_count, 2u);
  EXPECT_EQ(res.entries, (std::vector<entry>{{1, 0, 3}}));
}

TEST(scan_chunk_tests, no_final_newline) {
  const chunk_result res = scan_all("x\nbad", "?ad");

  EXPECT_EQ(res.newline_count, 1u);
  EXPECT_EQ(res.last_newline_offset, 1u);
  EXPECT_EQ(res.entries, (std::vector<entry>{{1, 0, 2}}));
}

TEST(scan_chunk_tests, empty_lines) {
  const chunk_result res = scan_all("\n\nbad\n\n", "?ad");

  EXPECT_EQ(res.newline_count, 4u);
  EXPECT_EQ(res.last_newline_offset, 6u);
  EXPECT_EQ(res.entries, (std::vector<entry>{{2, 0, 2}}));
}

TEST(scan_chunk_tests, no_newlines) {
  const chunk_result res = scan_all("bad", "?ad");

  EXPECT_EQ(res.newline_count, 0u);
  EXPECT_EQ(res.last_newline_offset, std::nullopt);
  EXPECT_EQ(res.entries, (std::vector<entry>{{0, 0, 0}}));
}

TEST(scan_chunk_tests, base_offset) {
  const chunk_result res = scan_chunk("x\nbad", 5, "?ad", 100);

  EXPECT_EQ(res.last_newline_offset, 101u);
  EXPECT_EQ(res.entries, (std::vector<entry>{{1, 0, 102}}));
}

TEST(scan_chunk_tests, first_line_pos_is_relative_to_chunk_start) {
  // The chunk starts in the middle of a line that began earlier.
  const chunk_result res = scan_chunk("xxbad\nbad", 9, "?ad", 10);

  EXPECT_EQ(res.entries, (std::vector<entry>{{0, 2, 12}, {1, 0, 16}}));
}

TEST(scan_chunk_tests, lookahead_completes_match) {
  // Owned: "xxba", lookahead: "d". The match starts in the owned part.
  const chunk_result res = scan_chunk("xxbad", 4, "bad", 0);

  EXPECT_EQ(res.entries, (std::vector<entry>{{0, 2, 2}}));
}

TEST(scan_chunk_tests, lookahead_match_is_not_owned) {
  // Owned: "xx", lookahead: "bad". The match belongs to the next chunk.
  const chunk_result res = scan_chunk("xxbad", 2, "bad", 0);

  EXPECT_TRUE(res.entries.empty());
}

TEST(scan_chunk_tests, lookahead_newlines_are_not_counted) {
  const chunk_result res = scan_chunk("a\nb\nc", 2, "?", 0);

  EXPECT_EQ(res.newline_count, 1u);
  EXPECT_EQ(res.last_newline_offset, 1u);
  EXPECT_EQ(res.entries, (std::vector<entry>{{0, 0, 0}}));
}

TEST(scan_chunk_tests, lookahead_is_cut_by_newline) {
  // "ba" is owned but the match would have to cross '\n'.
  const chunk_result res = scan_chunk("ba\nd", 2, "ba?", 0);

  EXPECT_TRUE(res.entries.empty());
}
