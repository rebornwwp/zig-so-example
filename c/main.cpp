#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <memory>

#include "zigmath.h"

int main() {
    std::cout << "zigmath version: " << zm_version() << '\n';

    std::cout << "10 + 20 = " << zm_add(10, 20) << '\n';

    const int32_t data[] = {1, 2, 3, 4, 5};
    std::cout << "sum([1..5]) = "
              << zm_sum(data, sizeof(data) / sizeof(data[0])) << '\n';

    // zm_greet 返回的字符串由 Zig 侧的 c_allocator 分配，
    // 必须用 zm_string_free 释放；用 unique_ptr 保证异常安全也不会漏掉。
    std::unique_ptr<char, decltype(&zm_string_free)> msg(
        zm_greet("C++ caller"), &zm_string_free);
    if (!msg) {
        std::cerr << "zm_greet failed\n";
        return EXIT_FAILURE;
    }
    std::cout << msg.get() << '\n';

    return EXIT_SUCCESS;
}
