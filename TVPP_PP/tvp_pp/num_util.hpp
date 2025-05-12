#pragma once
#include <cstdint>
#include <concepts>
#include <array>
#include <type_traits>
#include <cmath>
#include <variant>
#include <string>

using entry_t = std::variant<std::string, int, double>;

template<typename I, typename ... Ts>
concept uintx_compatible = requires(I i, Ts&& ... t) {
    requires (((sizeof(t)) + ...) == sizeof(i));
    requires (std::unsigned_integral<std::remove_cvref_t<Ts>> && ...) && std::unsigned_integral<std::remove_cvref_t<I>>;
};

template<typename T = std::uint32_t, typename ... Ts>
requires uintx_compatible<T, Ts...>
std::remove_cvref_t<T> bigend_cast_from_ints(Ts&& ... nums) {
    auto shift = [g = (sizeof(T) * 8)](auto&& n) mutable {
        g -= sizeof(n) * 8;
        return g;
        };

    return ((nums << shift(nums)) | ...);
}

// cast seperate integers to a bigger integers (default 32bit uint)
template<typename T = std::uint32_t, typename ... Ts>
    requires uintx_compatible<T, Ts...>
std::remove_cvref_t<T> lilend_cast_from_ints(Ts&& ... nums) {
    auto shift = [g = 0](auto&& n) mutable {
        auto old_g = g;
        g += sizeof(n) * 8;
        return old_g;
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

int _char_to_int(char& c) {
    int n = static_cast<int>(c) - 48;
    if ((n >= 0) && (n < 10)) {
        return n;
    }
    return -1;
}

bool _is_char_num(char& c) {
    return (_char_to_int(c) != -1);
}

int _parse_int(int gather, char c) {
    int n = _char_to_int(c);

    return (gather * 10) + n;
}

double _parse_double(bool passed_comma, double gather, char c, int* decimal_point) {
    int n = _char_to_int(c);
    if (!passed_comma) {
        gather = (gather * 10.0) + static_cast<double>(n);
    }
    else {
        gather += static_cast<double>(n) / static_cast<double>(std::pow(10, (*decimal_point)));
        (*decimal_point)++;
    }
    return gather;
}

entry_t parse_string_as_number(std::string const & entry, char const & ignore, char const & float_point) {
    if (entry.empty()) {
        return entry;
    }

    bool is_double = true;
    bool is_int = true;

    double d = 0;
    int decimal = 1;
    int i = 0;

    int sign = 1;
    int skip = 0;
    if (entry.starts_with('-')) {
        sign = -1;
        skip = 1;
    }
    if (entry.starts_with('+')) {
        skip = 1;
    }

    bool dot = false;

    for (auto& c : entry) {
        // skip - / +
        if (skip > 0) {
            skip -= 1;
            continue;
        }

        if (c == ignore) {
            continue;
        }

        if (c == float_point && !dot) {
            dot = true;
            is_int = false;
            continue;
        }
        else if (c == float_point && dot) {
            is_int = false;
            is_double = false;
            break;
        }

        if (!_is_char_num(c)) {
            is_int = false;
            is_double = false;
            break;
        }

        if (is_int) {
            i = _parse_int(i, c);

        }

        if (is_double) {
            d = _parse_double(dot, d, c, &decimal);
        }

    }
    if (is_int && !dot) {
        return  (i * sign);
    }
    if (is_double) {
        return (d * static_cast<double>(sign));
    }
    return entry;
}