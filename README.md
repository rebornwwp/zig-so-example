# Zig 动态库（.so）供 C 调用示例

[English](README_EN.md)

## 目录结构

```
zig-so-example/
├── build.zig              # Zig 构建脚本：编译动态库并安装头文件
├── build.zig.zon
├── src/zigmath.zig        # Zig 库源码，导出 C ABI 函数
├── include/zigmath.h      # 手写的 C 头文件
├── CMakeLists.txt         # 包装 C 程序，自动触发 zig build 并链接 .so
├── c/main.c               # C 调用方示例
├── cmake-build/           # CMake 构建目录
└── zig-out/               # Zig 构建产物
    ├── include/zigmath.h
    └── lib/libzigmath.so
```

## 构建

```bash
cd zig-so-example
zig build -Doptimize=ReleaseSafe
```

产物：

- `zig-out/lib/libzigmath.so`
- `zig-out/include/zigmath.h`

## 用 CMake 构建 C 程序（推荐）

`CMakeLists.txt` 会自动调用 `zig build` 生成 `.so`，再把生成的 `.so` 作为 IMPORTED 目标链接到 C 程序，并设置好 RPATH：

```bash
cmake -S . -B cmake-build -DCMAKE_BUILD_TYPE=Release
cmake --build cmake-build -j
./cmake-build/zigmath_demo
```

安装（可选）：

```bash
cmake --install cmake-build --prefix /path/to/prefix
/path/to/prefix/bin/zigmath_demo
```

安装内容包括 `bin/zigmath_demo`、`lib/libzigmath.so`、`include/zigmath.h`。

## 或者直接用 gcc 编译并运行 C 程序

```bash
gcc c/main.c \
  -Izig-out/include \
  -Lzig-out/lib \
  -lzigmath \
  -Wl,-rpath,'$ORIGIN/../zig-out/lib' \
  -o c/main

./c/main
```

预期输出：

```
zigmath version: 1
10 + 20 = 30
sum([1..5]) = 15
Hello, C caller! This string came from Zig.
```

也可以直接设置运行时库路径：

```bash
LD_LIBRARY_PATH=zig-out/lib ./c/main
```

## 也可以不用 build.zig，直接命令行编译

```bash
mkdir -p out
zig build-lib -dynamic -lc -O ReleaseSafe \
  -femit-bin=out/libzigmath.so src/zigmath.zig
gcc c/main.c -Iinclude -Lout -lzigmath -Wl,-rpath,'$ORIGIN' -o out/main
./out/main
```

## 关于头文件

头文件 `include/zigmath.h` 是**手写维护**的，`zig build` 只是把它复制到 `zig-out/include/`。

Zig 0.17 虽然保留了 `-femit-h` 开关，但目前被官方禁用（`error: -femit-h is currently broken, see https://github.com/ziglang/zig/issues/9698`），`build.zig` 里也没有对应的 API，因此本示例不依赖自动生成。新增或修改导出函数时，记得同步更新头文件。

## 关键点说明

1. **导出函数**：在 Zig 中用 `export fn xxx(...) callconv(.c) ...` 导出，符号名保持为 `xxx`，可被 C 直接链接。
2. **C 类型映射**：
   - Zig `i32` <-> C `int32_t`
   - Zig `u32` <-> C `uint32_t`
   - Zig `[*:0]const u8` <-> C `const char *`（以 NUL 结尾的字符串）
   - Zig `[*]const i32` + `usize` <-> C `const int32_t *` + `size_t`
   - Zig `?[*:0]u8` <-> C `char *`（可为 NULL）
3. **内存所有权**：`zm_greet` 用 `std.heap.c_allocator` 分配 C 字符串，调用方必须用 `zm_string_free` 释放（不能直接混用 free，除非确认分配器一致；这里 c_allocator 底层就是 C 的 malloc/free，保持一致即可）。
4. **链接 libc**：因为用到了 `std.heap.c_allocator`，构建时需要 `link_libc = true`（命令行则加 `-lc`）。
5. **查看导出符号**：`nm -D zig-out/lib/libzigmath.so | grep zm_`
