#pragma once
#include <unordered_map>
#include <iostream>
#include "../utfcpp/utf8.h"
#include <span>
#include <cstdint>
#include "../mio/single_include/mio/mio.hpp"
#include "num_util.hpp"
#include <string>
#include <format>
#include "data.hpp"

std::span<std::uint8_t const> seek_header(mio::ummap_source& mmap_file,std::size_t& offset, std::size_t skips = 1, std::size_t max_read = 100 ) {
	// 5A AF AA AB | Z¯ª«
	static std::uint32_t sentinel = 0x5AAFAAAB;
	//std::size_t skips = 1;
	std::size_t length = 0;
	auto read_4 = [](std::uint8_t const * it) {
		return bigend_cast_from_ints<std::uint32_t>(*it, *(it + 1),*(it + 2), *(it + 3));
	};

	std::size_t c{ 0 };
	auto it = mmap_file.begin() + offset;
	for (; it != mmap_file.end(); it++, c++, offset++) {
		if (read_4(it) == sentinel && skips > 0) {
			skips -= 1;
			it += 4; // skip header
			offset += 4;
			continue;
		}
		if (read_4(it) == sentinel && skips == 0){
			it += 8; // offset past sentinel into length
			length = read_4(it);
			it += 8;
			offset += 16 + length;
			break;
		}
		if (c >= max_read) {
			break;
		}
	}
	return std::span<std::uint8_t const>(it, it + length);
}

std::span<std::uint8_t const> seek_3byteimbuffer(mio::ummap_source& mmap_file, std::size_t& offset, std::size_t skips = 0) {
	// 5A AF AA AB | Z¯ª«
	static std::uint32_t const sentinel = 0x5AAFAAAB;
	//std::size_t skips = 1;
	std::size_t length = 0;
	auto read_4 = [](std::uint8_t const* it) {
		return bigend_cast_from_ints<std::uint32_t>(*it, *(it + 1), *(it + 2), *(it + 3));
		};

	std::size_t c{ 0 };
	auto it = mmap_file.begin() + offset;
	for (; it != mmap_file.end(); it++, c++, offset++) {
		if (read_4(it) == sentinel && skips > 0) {
			skips -= 1;
			it += 4; // skip header
			offset += 4;
			continue;
		}
		else if (read_4(it) == sentinel) {
			it += 8; // offset past sentinel into length
			length = read_4(it);
			it += 4;
			offset += 12 + length;
			break;
		}
		if (c >= 100) {
			break;
		}
	}
	return std::span<std::uint8_t const>(it, it + length);
}

std::span<std::uint8_t const> seek_ZCHK_SRAW(mio::ummap_source& mmap_file, std::size_t& offset) {
	// ZCHK [4 byt] SRAW [4 byt] czmp [16byt] [Length ZLIB 4 byt] [ ZLIB BLOCK -> ]
	static std::uint32_t const ZCHK = 0x5A43484B;
	static std::uint32_t const SRAW = 0x53524157;
	static std::uint32_t const czmp = 0x637A6D70;

	std::size_t stage = 0;
	std::size_t length = 0;
	auto read_4 = [](std::uint8_t const* it) {
		return bigend_cast_from_ints<std::uint32_t>(*it, *(it + 1), *(it + 2), *(it + 3));
	};

	auto it = mmap_file.begin() + offset;
	for (; it != mmap_file.end(); it++, offset++) {
		if (read_4(it) == ZCHK && stage == 0) {
			it += 8; // skip header + 4 chksum bytes
			offset += 8;
			stage += 1;
			continue;
		}
		if (read_4(it) == SRAW && stage == 1) {
			it += 8; // skip header + 4 chksum bytes
			offset += 8;
			stage += 1;
			continue;
		}
		if (read_4(it) == czmp && stage == 1) {
			it += 20; // skip header + 16 bytes
			length = read_4(it);
			it += 4;
			offset += 24;
			stage += 1;
			break;
		}
	}
	return std::span<std::uint8_t const>(it, it + length);
}

std::vector<std::string> file_read_header(std::span<std::uint8_t const>& header_keyvalue_section) {
	std::vector<std::string> strings{};
	std::u16string collect{};
	// last 8 bytes are some footer to a header section like this ending in [33 8X XX XX | BC 40 11 D7]
	// [BC 40 11 D7] is the sentinel where the section ends so we just need to cut 4 bytes.
	for (auto it = header_keyvalue_section.begin(); it != header_keyvalue_section.end()-4; it += 2) {
		auto cur_num = bigend_cast_from_ints<std::uint16_t>(*it, *(it + 1));
		if (cur_num > 27) {
			collect += static_cast<utf8::utfchar16_t>(cur_num);
		}
		else {
			// make sure first entry isn't empty.
			if ((strings.empty() && !collect.empty()) || (!strings.empty()))
				strings.push_back(utf8::utf16to8(collect));
			collect.clear();
		}
	}
	// take care of last bits
	if (!collect.empty()) {
		strings.push_back(utf8::utf16to8(collect));
	}
	return strings;
}