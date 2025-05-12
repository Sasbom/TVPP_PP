#pragma once
#include <unordered_map>
#include <iostream>
#include "../utfcpp/utf8.h"
#include <span>
#include <cstdint>
#include "../mio/single_include/mio/mio.hpp"
#include "num_util.hpp"
#include <string>

struct FileInfo {
	std::size_t width;
	std::size_t height;
	double fps;
	double pix_aspect_ratio;
	bool field_order;
	std::size_t start_frame;
	std::string uid;
	std::size_t shots;
	std::size_t drawings;
	std::string info;
	std::vector<std::string> notes;
	std::string author;
	std::string build; // date of build
	std::string host; // software name
	std::string os;
	std::string date_creation;
	std::string date_lastchanged;

	std::size_t camera_width;
	std::size_t camera_height;
	unsigned short camera_field_order;
	double camera_pix_aspect_ratio;
	bool camera_aa;
	bool camera;
	bool camera_show43_border;
	bool camera_safearea;
	double camera_safearea_borderout;
	double camera_safearea_borderin;
	
	bool is_locked;
	bool is_protected;
	std::string password;
	
	bool save_audio_deps;
	bool save_video_deps;
	std::vector<std::string> save_history;
	double user_workduration;
	std::string user_workchange; // don't know about this one yet.
};



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

std::vector<std::string> file_read_header(std::span<std::uint8_t const>& header_keyvalue_section) {
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
	return strings;
}

FileInfo parse_header_into_fileinfo(std::vector<std::string>& headerinfo) {
	auto fi = FileInfo{};
}