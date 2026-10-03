#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <random>
#include <sstream>
#include <string>

#include "async_find.hpp"
#include "test_utils.hpp"

namespace {

// The data files are next to this source file.
const std::filesystem::path data_dir =
    std::filesystem::path(__FILE__).parent_path() / "data";

const std::vector<std::string> masks = {
    "?ad", "aaa", "a", "I've", "??", "? ?", "mistakes", std::string(30, '?'),
};

std::string read_file(const std::filesystem::path &path) {
  std::ifstream file(path, std::ios::binary);
  std::stringstream ss;
  ss << file.rdbuf();
  return ss.str();
}

// Runs async_find on a data file with various chunk sizes and thread counts
// and compares the result with the reference search.
void check_data_file(const std::string &name) {
  const std::filesystem::path path =
      data_dir / name;
  const std::string text = read_file(path);
  ASSERT_FALSE(text.empty()) << path;

  stream_provider_ptr stream_provider =
      std::make_shared<ifstream_provider>(path.string());

  for (const std::string &mask : masks) {
    const std::vector<entry> expected = reference_find(text, mask);

    for (size_t chunk_size : {default_chunk_size, size_t{4096}, size_t{1000},
                              size_t{100}}) {
      for (size_t threads_count : {0, 3}) {
        ASSERT_EQ(async_find(mask, stream_provider, chunk_size, threads_count)
                      .entries,
                  expected)
            << "file: " << name << ", mask: \"" << mask
            << "\", chunk_size: " << chunk_size
            << ", threads_count: " << threads_count;
      }
    }
  }
}

// Deletes the file when the test ends, even if it fails.
struct temp_file {
  std::filesystem::path path;
  ~temp_file() { std::filesystem::remove(path); }
};

} // namespace

TEST(big_stream_tests, text) { check_data_file("text.txt"); }

TEST(big_stream_tests, long_line) { check_data_file("long_line.txt"); }

TEST(big_stream_tests, short_lines) { check_data_file("short_lines.txt"); }

TEST(big_stream_tests, several_default_chunks) {
  // ~40 MB, so the default 16 MB chunk size gives three chunks.
  std::mt19937 rng(7);
  std::uniform_int_distribution<int> pick(0, 9);
  const std::string_view alphabet = "aaaabd \n\n?";

  std::string text(40 * 1024 * 1024, ' ');
  for (char &c : text) {
    c = alphabet[pick(rng)];
  }

  const temp_file file{std::filesystem::temp_directory_path() /
                       "mtfind_big_stream_test.txt"};
  std::ofstream(file.path, std::ios::binary) << text;

  stream_provider_ptr stream_provider =
      std::make_shared<ifstream_provider>(file.path.string());

  for (const std::string mask : {"?ad", "aaa", "a?a?a"}) {
    const std::vector<entry> expected = reference_find(text, mask);
    ASSERT_FALSE(expected.empty());

    EXPECT_EQ(async_find(mask, stream_provider).entries, expected)
        << "mask: \"" << mask << "\"";
  }
}
