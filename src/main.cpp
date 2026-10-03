#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include "async_find.hpp"

namespace {

constexpr size_t max_mask_size = 100'000;

// Returns an error message, or an empty string if the arguments are valid.
std::string check_args(const std::string &file_path, const std::string &mask) {
  if (!std::filesystem::is_regular_file(file_path)) {
    return "file not found: " + file_path;
  }
  if (mask.empty()) {
    return "mask is empty";
  }
  if (mask.size() > max_mask_size) {
    return "mask is longer than " + std::to_string(max_mask_size) +
           " characters";
  }
  if (mask.find('\n') != std::string::npos) {
    return "mask must not contain a newline";
  }
  return {};
}

} // namespace

int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cerr << "usage: mtfind <file> \"<mask>\"\n";
    return 1;
  }

  const std::string file_path = argv[1];
  const std::string mask = argv[2];

  if (const std::string error = check_args(file_path, mask); !error.empty()) {
    std::cerr << "mtfind: " << error << '\n';
    return 1;
  }

  try {
    const results res =
        async_find(mask, std::make_shared<ifstream_provider>(file_path));

    std::ifstream source(file_path, std::ios::binary);
    std::ios::sync_with_stdio(false);
    print_results(res, source, std::cout);
  } catch (const std::exception &e) {
    std::cerr << "mtfind: " << e.what() << '\n';
    return 1;
  }

  return 0;
}
