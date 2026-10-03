#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <ostream>

#include "async_find.hpp"
#include "mtfind_assert.hpp"

namespace {

// An entry together with its matched text, as it is printed by mtfind.
struct found {
  size_t line;
  size_t pos;
  std::string str;

  bool operator==(const found &another) const = default;

  friend std::ostream &operator<<(std::ostream &os, const found &f) {
    return os << "{" << f.line << ", " << f.pos << ", \"" << f.str << "\"}";
  }
};

std::vector<found> to_found(const std::string &text, const results &res) {
  std::vector<found> result;
  for (const entry &e : res.entries) {
    result.push_back({e.line, e.pos, text.substr(e.offset, res.mask_size)});
  }
  return result;
}

} // namespace

TEST(mtfind_tests, empty_stream) {
  stream_provider_ptr stream_provider =
      std::make_shared<istringstream_provider>("");

  results res = async_find("mask", stream_provider);

  EXPECT_TRUE(res.entries.empty());
}

TEST(mtfind_tests, simple_stream) {
  std::stringstream ss;

  ss << "I've paid my dues\n";
  ss << "Time after time.\n";
  ss << "I've done my sentence.\n";
  ss << "But committed no crime.\n";
  ss << "And bad mistakes?\n";
  ss << "I've made a few.\n";
  ss << "I've had my share of sand kicked in my face.\n";
  ss << "But I've come through.\n";

  stream_provider_ptr stream_provider =
      std::make_shared<istringstream_provider>(ss.str());

  results res = async_find("?ad", stream_provider);

  const std::vector<found> expected = {
      {5, 5, "bad"},
      {6, 6, "mad"},
      {7, 6, "had"},
  };

  EXPECT_EQ(to_found(ss.str(), res), expected);
}

TEST(mtfind_tests, multiple_entries_in_line_stream) {
  std::stringstream ss;

  ss << "I've paid my dues\n";
  ss << "Time after time.\n";
  ss << "I've done my sentence.\n";
  ss << "But committed no crime.\n";
  ss << "And bad mistakes  ad trade?\n";
  ss << "I've made a few.\n";
  ss << "I've had my share of sand kicked in my face.\n";
  ss << "But I've come through.\n";

  stream_provider_ptr stream_provider =
      std::make_shared<istringstream_provider>(ss.str());

  results res = async_find("?ad", stream_provider);

  const std::vector<found> expected = {
      {5, 5, "bad"}, {5, 18, " ad"}, {5, 23, "rad"},
      {6, 6, "mad"}, {7, 6, "had"},
  };

  EXPECT_EQ(to_found(ss.str(), res), expected);
}

TEST(mtfind_tests, no_entries_stream) {
  std::stringstream ss;

  ss << "I've paid my dues\n";
  ss << "Time after time.\n";
  ss << "I've done my sentence.\n";
  ss << "But committed no crime.\n";
  ss << "But I've come through.\n";

  stream_provider_ptr stream_provider =
      std::make_shared<istringstream_provider>(ss.str());

  results res = async_find("?ad", stream_provider);

  EXPECT_TRUE(res.entries.empty());
}

TEST(mtfind_tests, same_for_any_chunk_size_and_threads_count) {
  const std::string text = "I've paid my dues\n"
                           "Time after time.\n"
                           "And bad mistakes  ad trade?\n"
                           "aaaaaaaaaaaaaaaaaaaaaaa\n"
                           "\n"
                           "I've had my share of sand kicked in my face";
  stream_provider_ptr stream_provider =
      std::make_shared<istringstream_provider>(text);

  for (const std::string mask : {"?ad", "aaa", "?", "I've", "??????????"}) {
    const results expected =
        async_find(mask, stream_provider, text.size(), 1);

    for (size_t chunk_size = 1; chunk_size <= text.size(); ++chunk_size) {
      for (size_t threads_count : {1, 2, 5}) {
        ASSERT_EQ(async_find(mask, stream_provider, chunk_size, threads_count),
                  expected)
            << "mask: " << mask << ", chunk_size: " << chunk_size
            << ", threads_count: " << threads_count;
      }
    }
  }
}

TEST(mtfind_tests, file_stream) {
  const std::string text = "I've paid my dues\n"
                           "And bad mistakes ?\n"
                           "I've made a few.\n";
  const std::filesystem::path path =
      std::filesystem::temp_directory_path() / "mtfind_test_input.txt";
  std::ofstream(path, std::ios::binary) << text;

  stream_provider_ptr stream_provider =
      std::make_shared<ifstream_provider>(path.string());

  const results res = async_find("?ad", stream_provider, 4, 3);
  std::filesystem::remove(path);

  const std::vector<found> expected = {{2, 5, "bad"}, {3, 6, "mad"}};
  EXPECT_EQ(to_found(text, res), expected);
}

TEST(mtfind_tests, print_results) {
  const std::string text = "I've paid my dues\n"
                           "And bad mistakes ?\n"
                           "I've made a few.\n";
  stream_provider_ptr stream_provider =
      std::make_shared<istringstream_provider>(text);

  const results res = async_find("?ad", stream_provider);

  std::istringstream source(text);
  std::ostringstream out;
  print_results(res, source, out);

  EXPECT_EQ(out.str(), "2\n"
                       "2 5 bad\n"
                       "3 6 mad\n");
}

TEST(mtfind_tests, print_no_results) {
  std::istringstream source("");
  std::ostringstream out;
  print_results(results{.mask_size = 3, .entries = {}}, source, out);

  EXPECT_EQ(out.str(), "0\n");
}
