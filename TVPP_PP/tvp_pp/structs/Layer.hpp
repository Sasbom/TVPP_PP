#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <cstddef>
#include <span>

struct Layer {
	Layer(std::span<std::uint8_t const>& layer_info);

	std::string name{};

	std::size_t first_frame_num{};
	std::size_t frame_offset{};

	// Replace these repeat in/out w/ enum class type
	std::size_t repeat_in_type{};
	std::size_t repeat_out_type{};
	
	std::size_t group_id{};

	std::uint8_t opacity_byte{};
	double opacity{};

	bool invisible{};
	bool lighttable{};
	bool stencil{};
	bool locked{};
	bool position_locked{};
	bool preserve_trans{};

	void print_info();

private:
	void name_from_LNAM_LNAW(std::span<std::uint8_t const> const& LNAM, std::span<std::uint8_t const> const& LNAW);
	void unpack_layerflags(std::uint8_t const& layer_flags);
};