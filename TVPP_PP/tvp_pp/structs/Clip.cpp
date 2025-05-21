#include <format>
#include <iostream>
#include <unordered_map>
#include "Clip.hpp"
#include "../../tvp_pp/data.hpp"

Clip::Clip(std::vector<std::string>& headerinfo) {
	static std::unordered_map<std::string, std::size_t> methods = {
		{"Name", 0},
		{"Dialog", 1},
		{"Action", 2},
		{"Note", 3},
		{"DialogSize", 4},
		{"ActionSize", 5},
		{"NoteSize", 6},
		{"MarkIn", 7},
		{"MarkInPosition", 8},
		{"MarkOut", 9},
		{"MarkOutPosition", 10},
		{"Hidden", 11},
		{"ColorIdx", 12},
	};
	// this is going to suck.
	for (auto it = headerinfo.begin(); it != headerinfo.end(); it++) {
		if (methods.contains(*it)) {
			switch (methods[*it]) {
			case 0: {
				name = *(it + 1);
				it++;
				break;
			}
			case 1: {
				dialog = *(it + 1);
				it++;
				break;
			}
			case 2: {
				action = *(it + 1);
				it++;
				break;
			}
			case 3: {
				note = *(it + 1);
				it++;
				break;
			}
			case 4: {
				dialog_size = data::parse_assume<double>(*(it + 1));
				it++;
				break;
			}
			case 5: {
				action_size = data::parse_assume<double>(*(it + 1));
				it++;
				break;
			}
			case 6: {
				note_size = data::parse_assume<double>(*(it + 1));
				it++;
				break;
			}
			case 7: {
				mark_in = data::parse_assume<int>(*(it + 1));
				it++;
				break;
			}
			case 8: {
				mark_in_pos = data::parse_assume<int>(*(it + 1));
				it++;
				break;
			}
			case 9: {
				mark_out = data::parse_assume<int>(*(it + 1));
				it++;
				break;
			}
			case 10: {
				mark_out_pos = data::parse_assume<int>(*(it + 1));
				it++;
				break;
			}
			case 11: {
				hidden = static_cast<bool>(data::parse_assume<int>(*(it + 1)));
				it++;
				break;
			}
			case 12: {
				color_idx = data::parse_assume<int>(*(it + 1));
				it++;
				break;
			}
			}
		}
	}
}

void Clip::print_info() {
	auto s = std::format(R"(Clip:
Name: {}
Dialog: {}
Action: {}
Note: {}
Dialog Size : {}
Action Size: {}
Note Size: {}
Mark In: {}
Mark In Position: {}
Mark Out: {}
Mark Out Position: {}
Hidden: {} 
Color Index: {}
)",
name,dialog,action,note,
dialog_size,action_size,note_size,
mark_in,mark_in_pos,mark_out,mark_out_pos,
hidden,color_idx
);
	std::cout << s;
}