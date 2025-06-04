#include "ThumbInfo.hpp"
#include <format>
#include <iostream>
#include <unordered_map>
#include "../../tvp_pp/data.hpp"

ThumbInfo::ThumbInfo(std::vector<std::string>& headerinfo) {
	static std::unordered_map<std::string, std::size_t> methods = {
		{"Width", 0},
		{"Height", 1},
		{"Type", 2},
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
				this->type = data::parse_assume<int>(*(it + 1));
				it++;
				break;
			}
			}
		}
	}
}

void ThumbInfo::print_info() {
	auto s = std::format(R"(Thumbnail Info:
WxH: {} x {}
Type: {}
)", width,height,type);
	std::cout << s;
}

std::vector<std::uint8_t> ThumbInfo::reserve_buffer(std::size_t pixel_stride) {
	return std::vector<std::uint8_t>(width * height * pixel_stride);
}