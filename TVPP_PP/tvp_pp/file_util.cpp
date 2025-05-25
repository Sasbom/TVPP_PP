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
#include <utility>
#include "file_util.hpp"

std::span<std::uint8_t const> seek_header(mio::ummap_source& mmap_file,std::size_t& offset, std::size_t skips, std::size_t max_read) {
	// 5A AF AA AB | Z¯ª«
	constexpr static std::uint32_t sentinel = 0x5AAFAAAB;
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

std::span<std::uint8_t const> seek_3byteimbuffer(mio::ummap_source& mmap_file, std::size_t& offset, std::size_t skips) {
	// 5A AF AA AB | Z¯ª«
	constexpr static std::uint32_t const sentinel = 0x5AAFAAAB;
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

// DEPECRATE?
std::span<std::uint8_t const> seek_ZCHK_SRAW(mio::ummap_source& mmap_file, std::size_t& offset) {
	// ZCHK [4 byt] SRAW [4 byt] czmp [16byt] [Length ZLIB 4 byt] [ ZLIB BLOCKS -> ]
	// The ZLIB blocks can contain multiple headers contiguously.
	constexpr static std::uint32_t const ZCHK = 0x5A43484B;
	constexpr static std::uint32_t const SRAW = 0x53524157;
	constexpr static std::uint32_t const czmp = 0x637A6D70;

	std::size_t stage = 0;
	std::size_t length = 0;
	auto read_4 = [](std::uint8_t const* it) {
		return bigend_cast_from_ints<std::uint32_t>(*it, *(it + 1), *(it + 2), *(it + 3));
		};

	for (auto it = mmap_file.begin() + offset; it != mmap_file.end(); /*it++, offset++*/) {
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
		if (read_4(it) == czmp && stage == 2) {
			it += 20; // skip header + 16 bytes
			length = read_4(it); // the length CAN NOT be read like this with DBOD headers.
			it += 4;
			offset += 24 + length;
			return std::span<std::uint8_t const>(it, it + length);
		}
	}
	// NOTE: throw after recieved object is size 0.s 
	std::span<std::uint8_t const> backup{};
}

std::vector<std::span<std::uint8_t const>> seek_ZCHK_SRAW_VEC(mio::ummap_source& mmap_file, std::size_t& offset) {
	// DBOD frames compressed are stored in zlib chunks that have 12 byte "sub headers" between them.
	// This means that the initially read "length" before the compressed block doesn't tell us
	// the whole story.
	// DBOD [4 byt] czmp [16 byt] [len ZLIB block 4 byt] [ZLIB -> len ]
	// followed by a 12 byte "sub header"??
	// [8 bytes with some data] [4 byt next ZLIB block len] [ZLIB -> len]
	// Unpacked it should be an RLE buffer that unrolls into a regular framebuffer.

	constexpr static std::uint32_t const ZCHK = 0x5A43484B;
	constexpr static std::uint32_t const SRAW = 0x53524157;
	constexpr static std::uint32_t const czmp = 0x637A6D70;
	constexpr static std::uint32_t const LEXT = 0x4C455854;

	std::vector<std::span<std::uint8_t const>> spans{};

	std::size_t stage = 0;
	std::size_t length = 0;

	auto check_valid = [&](std::uint8_t const* it) {
		for (std::size_t i{ 0 }; i < 4; i++) {
			if (it + i == mmap_file.end()) {
				return false;
			}
		}
		return true;
		};

	auto read_4 = [](std::uint8_t const* it) {
		return bigend_cast_from_ints<std::uint32_t>(*it, *(it + 1), *(it + 2), *(it + 3));
		};

	for (auto it = mmap_file.begin() + offset; it != mmap_file.end(); /*it++, offset++*/) {
		if (read_4(it) == ZCHK && stage == 0) {
			std::cout << "found ZCHK!\n";
			it += 8; // skip header + 4 chksum bytes
			offset += 8;
			stage += 1;
			continue;
		}
		if (read_4(it) == SRAW && stage == 1) {
			std::cout << "found SRAW!\n";
			it += 8; // skip header + 4 chksum bytes
			offset += 8;
			stage += 1;
			continue;
		}
		if (read_4(it) == czmp && stage == 2) {
			// get initial length and append valid blocks of ZLIB data
			it += 20; // skip header + 16 bytes
			offset += 20;
			std::cout << "reading length @" << offset << "\n";
			length = read_4(it); // initial block length
			std::cout << "read length " << length << "\n";
			//std::cout << length << "\n";
			it += 4;
			offset += 4;
			spans.push_back(std::span(it, it + length));
			it += length;
			offset += length;
			stage += 1;
			continue;
		}
		if ((stage == 3 && ((read_4(it + 1) == ZCHK) || (read_4(it) == ZCHK))) || (stage == 3 && ((read_4(it + 1) == LEXT) || (read_4(it) == LEXT)))) {
			std::cout << "ZCHK OR LEXT ENCOUNTERED\n";
			break;
		}
		// seek ZCHK
		// at the end of ZLIB every block there's an (optional) 00 byte followed by ZCHK.
		// so it + 1 should skip 00 and read out ZCHK (or not)
		if (stage == 3 && (read_4(it + 1) != ZCHK) && (read_4(it) != ZCHK) && (read_4(it + 1) != LEXT) && (read_4(it) != LEXT)) {
			
			// if conditions are met parse 12 byte sub header.
			it += 8;
			offset += 8;
			length = read_4(it);
			std::cout << length << " @ offset: " << offset << "\n";
			it += 4;
			spans.push_back(std::span(it, it + length));
			offset += 4 + length;
			it += length;
			continue;
		}
	}
	return spans;
}

std::vector<std::span<std::uint8_t const>> seek_ZCHK_DBOD(mio::ummap_source& mmap_file, std::size_t& offset) {
	// DBOD frames compressed are stored in zlib chunks that have 12 byte "sub headers" between them.
	// This means that the initially read "length" before the compressed block doesn't tell us
	// the whole story.
	// DBOD [4 byt] czmp [16 byt] [len ZLIB block 4 byt] [ZLIB -> len ]
	// followed by a 12 byte "sub header"??
	// [8 bytes with some data] [4 byt next ZLIB block len] [ZLIB -> len]
	// Unpacked it should be an RLE buffer that unrolls into a regular framebuffer.

	constexpr static std::uint32_t const ZCHK = 0x5A43484B;
	constexpr static std::uint32_t const DBOD = 0x44424F44;
	constexpr static std::uint32_t const czmp = 0x637A6D70;

	std::vector<std::span<std::uint8_t const>> spans{};

	std::size_t stage = 0;
	std::size_t length = 0;

	auto read_4 = [](std::uint8_t const* it) {
		return bigend_cast_from_ints<std::uint32_t>(*it, *(it + 1), *(it + 2), *(it + 3));
	};

	for (auto it = mmap_file.begin() + offset; it != mmap_file.end(); /*it++, offset++*/) {
		if (read_4(it) == ZCHK && stage == 0) {
			it += 8; // skip header + 4 chksum bytes
			offset += 8;
			stage += 1;
			continue;
		}
		if (read_4(it) == DBOD && stage == 1) {
			it += 8; // skip header + 4 chksum bytes
			offset += 8;
			stage += 1;
			continue;
		}
		if (read_4(it) == czmp && stage == 2) {
			// get initial length and append valid blocks of ZLIB data
			it += 20; // skip header + 16 bytes
			length = read_4(it); // initial block length
			//std::cout << length << "\n";
			it += 4;
			offset += 24;
			spans.push_back(std::span(it, it + length));
			it += length;
			offset += length;
			stage += 1;
			continue;
		}
		// seek ZCHK
		// at the end of ZLIB every block there's an (optional) 00 byte followed by ZCHK.
		// so it + 1 should skip 00 and read out ZCHK (or not)
		if (stage == 3 && ((read_4(it+1) != ZCHK) && (read_4(it) != ZCHK)) ) {
			// if conditions are met parse 12 byte sub header.
			it += 8;
			length = read_4(it);
			//std::cout << length << "\n";
			it += 4;
			spans.push_back(std::span(it, it + length));
			offset += 12+length;
			it += length;
			continue;
		}
		else if (stage == 3 && ((read_4(it + 1) == ZCHK) || (read_4(it) == ZCHK))) {
			break;
		}
		
	}
	return spans;
}

LEXT_AFTER seek_LEXT_UDAT_STCK_FCFG(mio::ummap_source& mmap_file, std::size_t& offset) {
	// skip past data at the end of a layer, to potentially end up at a new clip,
	// another new layer, or the end of a file.
	
	constexpr static std::uint32_t const LEXT = 0x4C455854;
	constexpr static std::uint32_t const UDAT = 0x55444154;
	constexpr static std::uint32_t const STCK = 0x5354434B;
	constexpr static std::uint32_t const XSRC = 0x58535243;
	constexpr static std::uint32_t const FCFG = 0x46434647;

	constexpr static std::uint32_t const LNAM = 0x4C4E414D;

	auto read_4 = [](std::uint8_t const* it) {
		return bigend_cast_from_ints<std::uint32_t>(*it, *(it + 1), *(it + 2), *(it + 3));
	};

	auto check_valid = [&](std::uint8_t const* it) {
		for (std::size_t i{ 0 }; i < 4; i++) {
			if (it + i == mmap_file.end()) {
				return false;
			}
		}
		return true;
	};

	std::size_t stage{ 0 };
	auto it = mmap_file.begin()+offset;

	while (true) {
		if ((stage == 0) && (read_4(it) == LEXT)) {
			auto len = read_4(it + 4);
			offset += 8 + len;
			it += 8 + len;
			stage++;
			continue;
		}

		if ((stage == 1) && (read_4(it) == UDAT)) {
			auto len = read_4(it + 4);
			offset += 8 + len;
			it += 8 + len;
			stage++;
			continue;
		}

		// stage 2 can branch off into LNAM.
		if ((stage == 2) && (read_4(it) == LNAM)) {
			return LEXT_AFTER::LAYER;
		}

		if ((stage == 2) && (read_4(it) == STCK)) {
			auto len = read_4(it + 4);
			offset += 8 + len;
			it += 8 + len;
			stage++;
			continue;
		}

		if ((stage == 3) && (read_4(it) == XSRC)) {
			auto len = read_4(it + 4);
			offset += 8 + len;
			it += 8 + len;
			stage++;
			continue;
		}

		if ((stage == 4) && (read_4(it) == FCFG)) {
			auto len = read_4(it + 4);
			offset += 8 + len;
			it += 8 + len;
			if (!check_valid(it)) {
				return LEXT_AFTER::TVP_EOF;
			}
			break;
		}

		it++;
		offset++;
	}

	return LEXT_AFTER::CLIP;
}

std::span<std::uint8_t const> seek_3bytimbuffer_XS24(mio::ummap_source& mmap_file, std::size_t& offset) {
	auto read_4 = [](std::uint8_t const* it) {
		return bigend_cast_from_ints<std::uint32_t>(*it, *(it + 1), *(it + 2), *(it + 3));
	};
	
	constexpr static std::uint32_t const XS24 = 0x58533234;
	auto it = mmap_file.begin() + offset;

	std::size_t limit_read = 100;
	while (true) {
		if (limit_read == 0) {
			break;
		}

		if (read_4(it) == XS24) {
			it += 4;
			auto length = read_4(it);
			// XS24 [length] [ -> ...
			offset += 8 + length;
			return std::span(it + 4, it + length);
		}
		it++;
		offset++;
		limit_read--;
	}
	return std::span<std::uint8_t const>{};
}

std::span<std::uint8_t const> seek_layer_header(mio::ummap_source& mmap_file, std::size_t& offset) {
	// gather a span of layer data.
	constexpr static std::uint32_t const LNAM = 0x4C4E414D;
	constexpr static std::uint32_t const LNAW = 0x4C4E4157;
	constexpr static std::uint32_t const LRHD = 0x4C524844;

	auto read_4 = [](std::uint8_t const* it) {
		return bigend_cast_from_ints<std::uint32_t>(*it, *(it + 1), *(it + 2), *(it + 3));
	};

	std::size_t stage{ 0 };
	auto it = mmap_file.begin() + offset;
	auto begin = mmap_file.begin() + offset;
	while(it != mmap_file.end()) {
		if (stage == 0) {
			if (read_4(it) == LNAM) {
				//std::cout << "found LNAM";
				begin = it;
				stage += 1;
				it += 4;
				offset += 4;
				std::size_t lnam_len = read_4(it);
				//std::cout << "read length:" << lnam_len << " ";
				it += lnam_len + 4;
				offset += lnam_len+ 4;
				continue;
			}
			else {
				//std::cout << static_cast<char>(*it);
				offset++;
				it++;
				continue;
			}
		}
		if (stage == 1) {
			if (read_4(it) == LNAW) {
				//std::cout << "Found LNAW";
				stage += 1;
				it += 4;
				offset += 4;
				std::size_t lnaw_len = read_4(it);
				it += lnaw_len+ 4;
				offset += lnaw_len + 4;
				continue;
			}
			else {
				// Failed...
				break;
			}
		}
		if (stage == 2) {
			if (read_4(it) == LRHD) {
				it += 4;
				std::size_t lrhd_len = read_4(it);
				it += lrhd_len + 4;
				offset += lrhd_len + 8;
				return std::span(begin, it);
			}
			else {
				// Failed...
				break;
			}
		}
	}
	return std::span<std::uint8_t const>{}; // unreachable.
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