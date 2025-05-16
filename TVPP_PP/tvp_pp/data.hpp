#pragma once

#include <cmath>
#include <variant>
#include <string>
#include <concepts>
#include <type_traits>

template<typename T>
concept is_assumable = requires(T&& t)
{
    requires (std::same_as<T, std::string> || std::same_as<T, int> || std::same_as<T, double>); 
};

using entry_t = std::variant<std::string, int, double>;

namespace data {
    
    namespace int_data {
        inline int _char_to_int(char const & c) {
            int n = static_cast<int>(c) - 48;
            if ((n >= 0) && (n < 10)) {
                return n;
            }
            return -1;
        }

        inline bool _is_char_num(char const & c) {
            return (_char_to_int(c) != -1);
        }

        inline int _parse_int(int gather, char c) {
            int n = _char_to_int(c);

            return (gather * 10) + n;
        }

        inline double _parse_double(bool passed_comma, double gather, char c, int* decimal_point) {
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

        inline entry_t parse_string_to_entry(std::string const& entry, char const& ignore = ',', char const& float_point = '.') {
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
    }

    template<typename T>
        requires is_assumable<T>
    inline T parse_assume(std::string& entry) {
        auto proc_entry = int_data::parse_string_to_entry(entry);
        return std::get<T>(proc_entry);
    }
}
