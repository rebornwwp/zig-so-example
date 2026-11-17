# Zig Shared Library (.so) Called from C — Example

[中文文档](README.md)

## Directory structure

```
zig-so-example/
├── build.zig              # Zig build script: builds the shared library and installs the header
├── build.zig.zon
├── src/zigmath.zig        # Zig library source, exporting functions with the C ABI
├── include/zigmath.h      # Hand-written C header
├── CMakeLists.txt         # Wraps the C program: triggers `zig build` and links the .so
├── c/main.c               # Example C caller
├── c/main.cpp             # Example C++ caller (verifies the extern "C" header)
├── cmake-build/           # CMake build directory
└── zig-out/               # Zig build output
    ├── include/zigmath.h
    └── lib/libzigmath.so
```

## Build

```bash
cd zig-so-example
zig build -Doptimize=ReleaseSafe
```

Outputs:

- `zig-out/lib/libzigmath.so`
- `zig-out/include/zigmath.h`

## Build the C program with CMake (recommended)

`CMakeLists.txt` automatically runs `zig build` to produce the `.so`, links it into the C program as an IMPORTED target, and sets up the RPATH:

```bash
cmake -S . -B cmake-build -DCMAKE_BUILD_TYPE=Release
cmake --build cmake-build -j
./cmake-build/zigmath_demo
./cmake-build/zigmath_demo_cpp   # C++ caller, same output except "C++ caller"
```

Install (optional):

```bash
cmake --install cmake-build --prefix /path/to/prefix
/path/to/prefix/bin/zigmath_demo
/path/to/prefix/bin/zigmath_demo_cpp
```

The installed files are `bin/zigmath_demo`, `bin/zigmath_demo_cpp`, `lib/libzigmath.so` and `include/zigmath.h`.

## Or compile and run the C program directly with gcc

```bash
gcc c/main.c \
  -Izig-out/include \
  -Lzig-out/lib \
  -lzigmath \
  -Wl,-rpath,'$ORIGIN/../zig-out/lib' \
  -o c/main

./c/main
```

Expected output:

```
zigmath version: 1
10 + 20 = 30
sum([1..5]) = 15
Hello, C caller! This string came from Zig.
```

You can also set the runtime library path explicitly:

```bash
LD_LIBRARY_PATH=zig-out/lib ./c/main
```

## Calling from C++

The header wraps its declarations in `#ifdef __cplusplus extern "C" { ... }`, so a C++ program can include it and link against this C-ABI shared library directly:

```bash
g++ c/main.cpp \
  -Izig-out/include \
  -Lzig-out/lib \
  -lzigmath \
  -Wl,-rpath,'$ORIGIN/../zig-out/lib' \
  -o c/main_cpp

./c/main_cpp
```

Expected output (only the last line differs, with `C++ caller`):

```
zigmath version: 1
10 + 20 = 30
sum([1..5]) = 15
Hello, C++ caller! This string came from Zig.
```

The string returned by `zm_greet` is allocated by Zig's `c_allocator`; the example uses `std::unique_ptr` with `zm_string_free` as a custom deleter so it is always released.

## Or compile from the command line without build.zig

```bash
mkdir -p out
zig build-lib -dynamic -lc -O ReleaseSafe \
  -femit-bin=out/libzigmath.so src/zigmath.zig
gcc c/main.c -Iinclude -Lout -lzigmath -Wl,-rpath,'$ORIGIN' -o out/main
./out/main
```

## About the header file

The header `include/zigmath.h` is **maintained by hand**; `zig build` only copies it to `zig-out/include/`.

Although Zig 0.17 still has the `-femit-h` flag, it is currently disabled upstream (`error: -femit-h is currently broken, see https://github.com/ziglang/zig/issues/9698`) and there is no corresponding API in `build.zig`, so this example does not rely on header generation. When adding or changing exported functions, remember to update the header accordingly.

## Key points

1. **Exporting functions**: use `export fn xxx(...) callconv(.c) ...` in Zig. The symbol name stays `xxx` and can be linked directly from C.
2. **C type mapping**:
   - Zig `i32` <-> C `int32_t`
   - Zig `u32` <-> C `uint32_t`
   - Zig `[*:0]const u8` <-> C `const char *` (NUL-terminated string)
   - Zig `[*]const i32` + `usize` <-> C `const int32_t *` + `size_t`
   - Zig `?[*:0]u8` <-> C `char *` (may be NULL)
3. **Memory ownership**: `zm_greet` allocates a C string with `std.heap.c_allocator`, so the caller must free it with `zm_string_free` (do not mix with `free` unless the allocators are known to match; here `c_allocator` uses C's malloc/free, so it is consistent).
4. **Linking libc**: because `std.heap.c_allocator` is used, the build needs `link_libc = true` (or `-lc` on the command line).
5. **Inspecting exported symbols**: `nm -D zig-out/lib/libzigmath.so | grep zm_`
