#pragma once
#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>

struct Clip {
	Clip(std::vector<std::string>& headerinfo);
	
	std::string name{};
	std::string dialog{};
	std::string action{};
	std::string note{};
	double dialog_size{};
	double action_size{};
	double note_size{};
	std::size_t mark_in{};
	std::size_t mark_in_pos{};
	std::size_t mark_out{};
	std::size_t mark_out_pos{};
	bool hidden{};
	std::size_t color_idx{};

	void print_info();
};