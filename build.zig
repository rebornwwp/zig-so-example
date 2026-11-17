const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const lib = b.addLibrary(.{
        .name = "zigmath",
        .root_module = b.createModule(.{
            .root_source_file = b.path("src/zigmath.zig"),
            .target = target,
            .optimize = optimize,
            .link_libc = true,
        }),
        .linkage = .dynamic,
    });
    b.installArtifact(lib);

    // 头文件为手写，随构建一起安装
    b.installFile("include/zigmath.h", "include/zigmath.h");
}
