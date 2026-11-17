const std = @import("std");

/// 简单的整数加法
export fn zm_add(a: i32, b: i32) callconv(.c) i32 {
    return a + b;
}

/// 计算 i32 数组的和
export fn zm_sum(items: [*]const i32, len: usize) callconv(.c) i32 {
    var total: i32 = 0;
    for (items[0..len]) |v| total += v;
    return total;
}

/// 返回一个由 Zig 分配的字符串，调用方必须用 zm_string_free 释放
export fn zm_greet(name: [*:0]const u8) callconv(.c) ?[*:0]u8 {
    const slice = std.mem.span(name);
    const z = std.fmt.allocPrintSentinel(
        std.heap.c_allocator,
        "Hello, {s}! This string came from Zig.",
        .{slice},
        0,
    ) catch return null;
    return z.ptr;
}

/// 释放 zm_greet 返回的字符串
export fn zm_string_free(ptr: ?[*:0]u8) callconv(.c) void {
    if (ptr) |p| {
        const s = std.mem.span(p);
        std.heap.c_allocator.free(s);
    }
}

/// 返回库版本号
export fn zm_version() callconv(.c) u32 {
    return 1;
}

test "zm_add" {
    try std.testing.expectEqual(@as(i32, 30), zm_add(10, 20));
}
