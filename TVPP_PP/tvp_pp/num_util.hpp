#pragma once
#include <cstdint>
#include <concepts>
#include <array>

template<typename I, typename ... Ts>
concept uintx_compatible = requires(I i, Ts&& ... t) {
    requires (((sizeof(t)) + ...) == sizeof(i));
    requires (std::unsigned_integral<Ts> && ...) && std::unsigned_integral<I>;
};

template<typename T = std::uint32_t, typename ... Ts>
requires uintx_compatible<T, Ts...>
T bigend_cast_from_ints(Ts&& ... nums) {
    auto shift = [g = (sizeof(T) * 8)](auto&& n) mutable {
        g -= sizeof(n) * 8;
        return g;
        };

    return ((nums << shift(nums)) | ...);
}

template<typename T = std::uint32_t>
requires std::unsigned_integral<T> && (sizeof(T) > 1)
void swap_endianness_uintx_inplace(T& num) {
    std::uint8_t* n_ptr = reinterpret_cast<std::uint8_t*>(&num);
    std::size_t c{ 0 };
    for (auto i = n_ptr; i != n_ptr + (sizeof(num) / 2); i++, c++) {
        auto a = *i;
        auto b_ptr = n_ptr + (sizeof(num) - c - 1);
        *i = *b_ptr;
        *b_ptr = a;
    }
}

template<typename T = std::uint32_t>
    requires std::unsigned_integral<T> && (sizeof(T) > 1)
T swap_endianness_uintx(T num) {
    swap_endianness_uintx_inplace(num);
    return num;
}