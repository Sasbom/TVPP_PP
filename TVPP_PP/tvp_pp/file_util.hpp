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
	bool camera_field_order;
	double camera_fps;
	double camera_pix_aspect_ratio;
	bool camera_aa;
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
			// make sure first entry isn't empty.
			if ((strings.empty() && !collect.empty()) || (!strings.empty()))
				strings.push_back(utf8::utf16to8(collect));
			collect.clear();
		}
	}
	return strings;
}

FileInfo parse_header_into_fileinfo(std::vector<std::string>& headerinfo) {
	auto fi = FileInfo{};

	// switch casing (sausage emoji)
	static std::unordered_map<std::string, std::size_t> methods = {
		{"Width", 0},
		{"Height", 1},
		{"FrameRate", 2},
		{"PixelAspectRatio", 3},
		{"FieldOrder", 4},
		{"StartFrame", 5},
		{"Shots", 6},
		{"Drawings", 7},
		{"Info", 8},
		{"Notes", 9},
		{"Author", 10},
		{"Build", 11},
		{"Host", 12},
		{"OS", 13},
		{"CreationDate", 14},
		{"LastChangeDate", 15},
		{"Camera.Width", 16},
		{"Camera.Height", 17},
		{"Camera.FieldOrder", 18},
		{"Camera.FrameRate", 19},
		{"Camera.PixelAspectRatio", 20},
		{"Camera.Antialiasing", 21},
		{"Camera.Show43Border", 22},
		{"Camera.SafeArea", 23},
		{"Camera.SafeArea.BorderOut", 24},
		{"Camera.SafeArea.BorderIn", 25},
		{"Locked", 26},
		{"Protected", 27},
		{"Password", 28},
		{"SaveAudioDependencies", 29},
		{"SaveVideoDependencies", 30},
		{"SaveHistory", 31},
		{"UserWorkDuration", 32},
	};
	// this is going to suck.
	for (auto it = headerinfo.begin(); it != headerinfo.end(); it++) {
		if (methods.contains(*it)) {
			switch (methods[*it]) {
			case 0: {
				fi.width = data::parse_assume<int>(*(it + 1));
				it++;
				break;
			}
			case 1: {
				fi.height = data::parse_assume<int>(*(it + 1));
				it++;
				break;
			}
			case 2: {
				fi.fps = data::parse_assume<double>(*(it + 1));
				it++;
				break;
			}
			case 3: {
				fi.pix_aspect_ratio = data::parse_assume<double>(*(it + 1));
				it++;
				break;
			}
			case 4: {
				fi.field_order = static_cast<bool>(data::parse_assume<int>(*(it + 1)));
				it++;
				break;
			}
			case 5: {
				fi.start_frame = data::parse_assume<int>(*(it + 1));
				it++;
				break;
			}
			case 6: {
				fi.shots = data::parse_assume<int>(*(it + 1));
				it++;
				break;
			}
			case 7: {
				fi.drawings = data::parse_assume<int>(*(it + 1));
				it++;
				break;
			}
			case 8: {
				fi.info = *(it + 1);
				it++;
				break;
			}
			case 9: {
				fi.notes = decltype(fi.notes){};
				while (*(it + 1) != "Author") {
					fi.notes.push_back(*(it + 1));
					it++;
				}
				it++;
				break;
			}
			case 10: {
				fi.author = *(it + 1);
				it++;
				break;
			}
			case 11: {
				fi.build = *(it + 1);
				it++;
				break;
			}
			case 12: {
				fi.host = *(it + 1);
				it++;
				break;
			}
			case 13: {
				fi.os = *(it + 1);
				it++;
				break;
			}
			case 14: {
				fi.date_creation = *(it + 1);
				it++;
				break;
			}
			case 15: {
				fi.date_lastchanged = *(it + 1);
				it++;
				break;
			}
			case 16: {
				fi.camera_width = data::parse_assume<int>(*(it + 1));
				it++;
				break;
			}
			case 17: {
				fi.camera_height = data::parse_assume<int>(*(it + 1));
				it++;
				break;
			}
			case 18: {
				fi.camera_field_order = static_cast<bool>(data::parse_assume<int>(*(it + 1)));
				it++;
				break;
			}
			case 19: {
				fi.camera_fps = data::parse_assume<double>(*(it + 1));
				it++;
				break;
			}
			case 20: {
				fi.camera_pix_aspect_ratio = data::parse_assume<double>(*(it + 1));
				it++;
				break;
			}
			case 21: {
				fi.camera_aa = static_cast<bool>(data::parse_assume<int>(*(it + 1)));
				it++;
				break;
			}
			case 22: {
				fi.camera_show43_border = static_cast<bool>(data::parse_assume<int>(*(it + 1)));
				it++;
				break;
			}
			case 23: {
				fi.camera_safearea = static_cast<bool>(data::parse_assume<int>(*(it + 1)));
				it++;
				break;
			}
			case 24: {
				fi.camera_safearea_borderout = data::parse_assume<double>(*(it + 1));
				it++;
				break;
			}
			case 25: {
				fi.camera_safearea_borderin = data::parse_assume<double>(*(it + 1));
				it++;
				break;
			}
			case 26: {
				fi.is_locked = static_cast<bool>(data::parse_assume<int>(*(it + 1)));
				it++;
				break;
			}
			case 27: {
				fi.is_protected = static_cast<bool>(data::parse_assume<int>(*(it + 1)));
				it++;
				break;
			}
			case 28: {
				fi.password = *(it + 1); // yes really. Password is saved plaintext.
				it++;
				break;
			}
			case 29: {
				fi.save_audio_deps = static_cast<bool>(data::parse_assume<int>(*(it + 1)));
				it++;
				break;
			}
			case 30: {
				fi.save_video_deps = static_cast<bool>(data::parse_assume<int>(*(it + 1)));
				it++;
				break;
			}
			case 31: {
				fi.save_history = decltype(fi.save_history){};
				std::size_t history_to_read = data::parse_assume<int>(*(it + 1));
				it++;
				while (history_to_read > 0) {
					fi.save_history.push_back(*(it + 1));
					history_to_read--;
					it++;
				}
				break;
			}
			case 32: {
				fi.user_workduration = data::parse_assume<double>(*(it + 1));
				it++;
				break;
			}
			}
		}
		else if (it->contains("UniqueID")) {
			fi.uid = *it;
		}
	}

	return fi;
}

void print_fileinfo(FileInfo& fi) {
	auto s = std::format(R"(File:
WxH: {} x {}
FPS: {}
Pixel Aspect Ratio: {}
Field Order: {}
Start Frame : {}
uuid: {}
Shots: {}
Drawings: {}
Info: {}
Notes: {}
Author: {}
Build: {} 
Host: {}
OS: {}
Created / Last Changed: {} / {}

Camera:
WxH: {} x {}
Field Order: {}
FPS: {}
Pixel Aspect Ratio: {}
Anti Aliasing: {}
Safe Area: {}
Border in/out: {} / {}

Meta:
Locked: {}
Protected: {}
Password: {}
Save Audio Dependencies: {}
Save Video dependencies: {}
History:
{}
Work Duration: {}
)",
fi.width,fi.height,fi.fps,
fi.pix_aspect_ratio,
fi.field_order,
fi.start_frame,
fi.uid,
fi.shots, fi.drawings,
fi.info, fi.notes,fi.author, fi.build, fi.host,fi.os,
fi.date_creation,fi.date_lastchanged,
fi.camera_width, fi.camera_height, fi.camera_field_order, fi.camera_fps,
fi.camera_pix_aspect_ratio, fi.camera_aa,fi.camera_safearea,
fi.camera_safearea_borderin, fi.camera_safearea_borderout,
fi.is_locked,fi.is_protected,fi.password,fi.save_audio_deps,
fi.save_video_deps,fi.save_history,fi.user_workduration
);
	std::cout << s;
}