#pragma once

#include <cstdio>
#include <cstdlib>
#include <source_location>

inline void
mtfind_assert(bool result,
              std::source_location loc = std::source_location::current()) {
  if (!result) {
    std::fprintf(stderr, "assert\n%s:%d in %s\n", loc.file_name(), loc.line(),
                loc.function_name());
    std::abort();
  }
}