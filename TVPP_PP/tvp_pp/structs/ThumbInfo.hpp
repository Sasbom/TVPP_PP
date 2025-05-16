#pragma once
#include <cstddef>
#include <vector>
#include <string>

struct ThumbInfo {

	ThumbInfo(std::vector<std::string>& thumbinfo);

	std::size_t width{};
	std::size_t height{};

	std::size_t type{};

	void print_info();

	std::vector<std::uint8_t> reserve_buffer(std::size_t pixel_stride = 3);
};