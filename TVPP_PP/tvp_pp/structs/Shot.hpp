#pragma once
#include <string>
#include <cstdint>
#include <cstddef>

struct Shot {
	std::string name{};
	std::size_t drawings{};

	void print_info();
};