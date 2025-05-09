#pragma once
#include <cstdint>
#include <concepts>

// Ridiculous way to compile some numbers together as a big endian uint32.
template<typename ... Ts> requires (std::is_integral_v<Ts> && ...)
std::uint32_t be_int32(Ts ... nums) {
    std::uint32_t num = 0;
    std::size_t start = 32;

    ([&](auto&& n) {
        start -= sizeof(n) * 8;
        num += (n << start);
        }(nums), ...);

    return num;
}
