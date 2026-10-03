# mtfind

Multithreaded search for a mask in a text file. In the mask, `?` matches any
single character.

## Usage

```
mtfind <file> "<mask>"
```

The program prints the number of matches, then one line per match: line number,
position in the line, and the matched text. Numbering starts at 1, matches don't
overlap, and they are printed in file order.

```
$ mtfind input.txt "?ad"
3
5 5 bad
6 6 mad
7 6 had
```

The search is case-sensitive, and a match never spans more than one line. The
mask must not be empty or contain a newline, and can be up to 100,000
characters long.

## How it works

1. The file is split into 16 MB chunks by size, not by lines, so threads get
   equal work whatever the line lengths are.
2. A pool of threads (one per CPU core) scans the chunks. Each thread reads its
   chunk plus `mask length - 1` bytes past its end, so matches that cross a
   chunk boundary are found. For each chunk it records the newline count and
   the matches.
3. The chunk results are merged in file order. Newline counts give each chunk's
   first line number. Matches that overlap the previous chunk's last match are
   dropped, and the few bytes they hid are scanned again, so the result is the
   same as a single-threaded search.
4. The text of each match is read back from the file when printing, so only
   offsets are kept in memory.

## Building

Requirements: CMake 3.14+ and a C++20 compiler. GoogleTest is downloaded by
CMake during configuration.

```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Running

```
./build/mtfind input.txt "?ad"
```

Tests:

```
./build/mtfind_test
```
