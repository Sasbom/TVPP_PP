#include <format>
#include <iostream>
#include <unordered_map>
#include "FileInfo.hpp"
#include "../data.hpp"

FileInfo::FileInfo(std::vector<std::string>& headerinfo) {
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
		{"UserWorkChange", 33},
	};
	// this is going to suck.
	for (auto it = headerinfo.begin(); it != headerinfo.end(); it++) {
		if (methods.contains(*it)) {
			switch (methods[*it]) {
			case 0: {
				this->width = data::parse_assume<int>(*(it + 1));
				it++;
				break;
			}
			case 1: {
				this->height = data::parse_assume<int>(*(it + 1));
				it++;
				break;
			}
			case 2: {
				this->fps = data::parse_assume<double>(*(it + 1));
				it++;
				break;
			}
			case 3: {
				this->pix_aspect_ratio = data::parse_assume<double>(*(it + 1));
				it++;
				break;
			}
			case 4: {
				this->field_order = static_cast<bool>(data::parse_assume<int>(*(it + 1)));
				it++;
				break;
			}
			case 5: {
				this->start_frame = data::parse_assume<int>(*(it + 1));
				it++;
				break;
			}
			case 6: {
				this->shots = data::parse_assume<int>(*(it + 1));
				it++;
				break;
			}
			case 7: {
				this->drawings = data::parse_assume<int>(*(it + 1));
				it++;
				break;
			}
			case 8: {
				this->info = *(it + 1);
				it++;
				break;
			}
			case 9: {
				this->notes = decltype(this->notes){};
				while (*(it + 1) != "Author") {
					this->notes.push_back(*(it + 1));
					it++;
				}
				it++;
				break;
			}
			case 10: {
				this->author = *(it + 1);
				it++;
				break;
			}
			case 11: {
				this->build = *(it + 1);
				it++;
				break;
			}
			case 12: {
				this->host = *(it + 1);
				it++;
				break;
			}
			case 13: {
				this->os = *(it + 1);
				it++;
				break;
			}
			case 14: {
				this->date_creation = *(it + 1);
				it++;
				break;
			}
			case 15: {
				this->date_lastchanged = *(it + 1);
				it++;
				break;
			}
			case 16: {
				this->camera_width = data::parse_assume<int>(*(it + 1));
				it++;
				break;
			}
			case 17: {
				this->camera_height = data::parse_assume<int>(*(it + 1));
				it++;
				break;
			}
			case 18: {
				this->camera_field_order = static_cast<bool>(data::parse_assume<int>(*(it + 1)));
				it++;
				break;
			}
			case 19: {
				this->camera_fps = data::parse_assume<double>(*(it + 1));
				it++;
				break;
			}
			case 20: {
				this->camera_pix_aspect_ratio = data::parse_assume<double>(*(it + 1));
				it++;
				break;
			}
			case 21: {
				this->camera_aa = static_cast<bool>(data::parse_assume<int>(*(it + 1)));
				it++;
				break;
			}
			case 22: {
				this->camera_show43_border = static_cast<bool>(data::parse_assume<int>(*(it + 1)));
				it++;
				break;
			}
			case 23: {
				this->camera_safearea = static_cast<bool>(data::parse_assume<int>(*(it + 1)));
				it++;
				break;
			}
			case 24: {
				this->camera_safearea_borderout = data::parse_assume<double>(*(it + 1));
				it++;
				break;
			}
			case 25: {
				this->camera_safearea_borderin = data::parse_assume<double>(*(it + 1));
				it++;
				break;
			}
			case 26: {
				this->is_locked = static_cast<bool>(data::parse_assume<int>(*(it + 1)));
				it++;
				break;
			}
			case 27: {
				this->is_protected = static_cast<bool>(data::parse_assume<int>(*(it + 1)));
				it++;
				break;
			}
			case 28: {
				this->password = *(it + 1); // yes really. Password is saved plaintext.
				it++;
				break;
			}
			case 29: {
				this->save_audio_deps = static_cast<bool>(data::parse_assume<int>(*(it + 1)));
				it++;
				break;
			}
			case 30: {
				this->save_video_deps = static_cast<bool>(data::parse_assume<int>(*(it + 1)));
				it++;
				break;
			}
			case 31: {
				this->save_history = decltype(this->save_history){};
				std::size_t history_to_read = data::parse_assume<int>(*(it + 1));
				it++;
				while (history_to_read > 0) {
					this->save_history.push_back(*(it + 1));
					history_to_read--;
					it++;
				}
				break;
			}
			case 32: {
				this->user_workduration = data::parse_assume<double>(*(it + 1));
				it++;
				break;
			}
			case 33: {
				this->user_workchange = data::parse_assume<int>(*(it + 1));
				it++;
				break;
			}
			}
		}
		else if (it->contains("UniqueID")) {
			this->uid = *it;
		}
	}
}

void FileInfo::print_info() {
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
Work Change: {}
)",
width, height, fps,
pix_aspect_ratio,
field_order,
start_frame,
uid,
shots, drawings,
info, notes, author, build, host, os,
date_creation, date_lastchanged,
camera_width, camera_height, camera_field_order, camera_fps,
camera_pix_aspect_ratio, camera_aa, camera_safearea,
camera_safearea_borderin, camera_safearea_borderout,
is_locked, is_protected, password, save_audio_deps,
save_video_deps, save_history, user_workduration, user_workchange
);
	std::cout << s;
}