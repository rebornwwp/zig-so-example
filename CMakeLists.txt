cmake_minimum_required(VERSION 3.16)

project(zigmath_demo
    VERSION 0.1.0
    LANGUAGES C CXX
)

# ---------------------------------------------------------------------------
# 1) 自动调用 `zig build` 生成 libzigmath.so（头文件为手写）
# ---------------------------------------------------------------------------
find_program(ZIG_EXECUTABLE zig REQUIRED)

set(ZIG_PROJECT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(ZIG_OUTPUT_DIR "${ZIG_PROJECT_DIR}/zig-out")
set(ZIG_LIB_PATH "${ZIG_OUTPUT_DIR}/lib/libzigmath.so")
set(ZIG_INCLUDE_DIR "${ZIG_OUTPUT_DIR}/include")

# IMPORTED 目标的 INTERFACE_INCLUDE_DIRECTORIES 在 configure 阶段必须已存在，
# 这里先建好空目录，内容由后续的 `zig build` 填充。
file(MAKE_DIRECTORY "${ZIG_INCLUDE_DIR}")
file(MAKE_DIRECTORY "${ZIG_OUTPUT_DIR}/lib")

add_custom_target(zig_library ALL
    COMMAND ${ZIG_EXECUTABLE} build -Doptimize=ReleaseSafe
    WORKING_DIRECTORY "${ZIG_PROJECT_DIR}"
    COMMENT "Building Zig shared library: libzigmath.so"
    VERBATIM
)

# ---------------------------------------------------------------------------
# 2) 把 Zig 生成的 .so 作为 IMPORTED 目标引入
# ---------------------------------------------------------------------------
add_library(zigmath SHARED IMPORTED GLOBAL)
set_target_properties(zigmath PROPERTIES
    IMPORTED_LOCATION "${ZIG_LIB_PATH}"
    INTERFACE_INCLUDE_DIRECTORIES "${ZIG_INCLUDE_DIR}"
)

# ---------------------------------------------------------------------------
# 3) C / C++ 可执行程序
# ---------------------------------------------------------------------------
add_executable(zigmath_demo c/main.c)
add_executable(zigmath_demo_cpp c/main.cpp)
add_dependencies(zigmath_demo zig_library)
add_dependencies(zigmath_demo_cpp zig_library)

foreach(target zigmath_demo zigmath_demo_cpp)
    target_link_libraries(${target} PRIVATE zigmath)
    target_include_directories(${target} PRIVATE "${ZIG_INCLUDE_DIR}")

    # 运行时能找到 .so：构建/安装时都把 zig-out/lib 写进 RPATH
    set_target_properties(${target} PROPERTIES
        BUILD_RPATH "${ZIG_OUTPUT_DIR}/lib"
        INSTALL_RPATH "${ZIG_OUTPUT_DIR}/lib"
        BUILD_WITH_INSTALL_RPATH FALSE
    )
endforeach()

# ---------------------------------------------------------------------------
# 4) 安装规则（可选）
# ---------------------------------------------------------------------------
include(GNUInstallDirs)
install(TARGETS zigmath_demo zigmath_demo_cpp RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES "${ZIG_LIB_PATH}" DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY "${ZIG_INCLUDE_DIR}/" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
