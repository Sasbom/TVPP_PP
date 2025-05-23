#pragma once
#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>
#include <memory>
#include "Layer.hpp"

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

	std::vector<std::unique_ptr<Layer>> layers{};

	void print_info();
};