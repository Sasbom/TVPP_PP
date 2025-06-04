#include "Shot.hpp"
#include <format>
#include <iostream>
#include <unordered_map>
#include "../../tvp_pp/data.hpp"

Shot::Shot(std::vector<std::string>& headerinfo) {
	static std::unordered_map<std::string, std::size_t> methods = {
		{"Name", 0},
		{"Drawings", 1},
	};
	// this is going to suck.
	for (auto it = headerinfo.begin(); it != headerinfo.end(); it++) {
		if (methods.contains(*it)) {
			switch (methods[*it]) {
			case 0: {
				this->name = *(it + 1);
				it++;
				break;
			}
			case 1: {
				this->drawings = data::parse_assume<int>(*(it + 1));
				it++;
				break;
			}
			}
		}
	}
}

void Shot::print_info() {
	auto s = std::format(R"(Shot:
Shot Name: {}
Drawings: {}
)", name, drawings);
	std::cout << s;
}