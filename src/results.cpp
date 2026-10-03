#include <stdexcept>
#include <string>

#include "results.hpp"

void print_results(const results &res, std::istream &source,
                   std::ostream &out) {
  out << res.entries.size() << '\n';

  std::string text(res.mask_size, '\0');

  for (const entry &e : res.entries) {
    source.seekg(static_cast<std::streamoff>(e.offset));
    source.read(text.data(), static_cast<std::streamsize>(text.size()));
    if (!source) {
      throw std::runtime_error("failed to read the match at offset " +
                               std::to_string(e.offset));
    }

    out << e.line << ' ' << e.pos << ' ' << text << '\n';
  }
}
