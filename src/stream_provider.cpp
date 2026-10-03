#include <filesystem>

#include "mtfind_assert.hpp"
#include "stream_provider.hpp"

istringstream_provider::istringstream_provider(const std::string &the_text)
    : text(the_text), stream_size(the_text.size()) {}

stream_ptr istringstream_provider::create_stream() const {
  return std::make_unique<std::istringstream>(text);
}

size_t istringstream_provider::get_stream_size() const { return stream_size; }

ifstream_provider::ifstream_provider(const std::string &the_file_path)
    : file_path(the_file_path) {
  mtfind_assert(std::filesystem::exists(file_path));
  stream_size = std::filesystem::file_size(file_path);
}

stream_ptr ifstream_provider::create_stream() const {
  return std::make_unique<std::ifstream>(file_path, std::ios::binary);
}

size_t ifstream_provider::get_stream_size() const { return stream_size; }
