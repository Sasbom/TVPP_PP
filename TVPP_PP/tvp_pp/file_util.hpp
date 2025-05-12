#pragma once
#include <unordered_map>
#include <iostream>
#include "../utfcpp/utf8.h"
#include <span>
#include <cstdint>
#include "../mio/single_include/mio/mio.hpp"
#include "num_util.hpp"

// unknown sentinel info BC 40 11 D7 | �@�

std::span<std::uint8_t const> seek_header(mio::ummap_source& mmap_file, std::size_t max_read = 100) {
	// 5A AF AA AB | Z¯ª«
	std::uint32_t sentinel = 0x5AAFAAAB;
	std::size_t skips = 1;
	std::size_t length = 0;
	auto read_4 = [](std::uint8_t const * it) {
		return bigend_cast_from_ints<std::uint32_t>(*it, *(it + 1),*(it + 2), *(it + 3));
	};

	std::size_t c{ 0 };
	auto it = mmap_file.begin();
	for (; it != mmap_file.end(); it++, c++) {
		if (read_4(it) == sentinel && skips > 0) {
			skips -= 1;
			it += 4; // skip header
			continue;
		}
		else if (read_4(it) == sentinel){
			it += 8; // offset past sentinel into length
			length = read_4(it);
			it += 8;
			break;
		}
		if (c >= 100) {
			break;
		}
	}
	return std::span<std::uint8_t const>(it, it + length);
}

void file_read_header(std::span<std::uint8_t const>& header_keyvalue_section) {
	std::vector<std::string> strings{};
	
	std::u16string collect{};
	for (auto it = header_keyvalue_section.begin(); it != header_keyvalue_section.end(); it += 2) {
		auto cur_num = bigend_cast_from_ints<std::uint16_t>(*it, *(it + 1));
		if (cur_num > 27) {
			collect += static_cast<utf8::utfchar16_t>(cur_num);
		}
		else {
			strings.push_back(utf8::utf16to8(collect));
			collect.clear();
		}
	}
	for (auto s : strings) {
		std::cout << s << "\n";
	}
}