cmake_minimum_required(VERSION 3.14)
project(mtfind)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
  set(MTFIND_WARNINGS /W4)
else()
  set(MTFIND_WARNINGS -Wall -Wextra)
endif()

file(GLOB_RECURSE MTFIND_SOURCES CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp
)

set(MTFIND_MAIN ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)
list(REMOVE_ITEM MTFIND_SOURCES ${MTFIND_MAIN})

add_library(mtfind_lib ${MTFIND_SOURCES})
target_include_directories(mtfind_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_options(mtfind_lib PRIVATE ${MTFIND_WARNINGS})

add_executable(mtfind ${MTFIND_MAIN})
target_link_libraries(mtfind PRIVATE mtfind_lib)
target_compile_options(mtfind PRIVATE ${MTFIND_WARNINGS})

# ---- Tests ----
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

file(GLOB_RECURSE MTFIND_TESTS CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp
)

add_executable(mtfind_test ${MTFIND_TESTS})
target_link_libraries(mtfind_test PRIVATE mtfind_lib GTest::gtest_main)
target_compile_options(mtfind_test PRIVATE ${MTFIND_WARNINGS})

include(GoogleTest)
gtest_discover_tests(mtfind_test)

