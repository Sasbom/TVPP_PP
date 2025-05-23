#pragma once
#include <string>
#include <cstdint>
#include <cstddef>
#include <vector>

struct Shot {
	Shot(std::vector<std::string>& headerinfo);
	Shot() = default;
	std::string name{};
	std::size_t drawings{};

	void print_info();
};