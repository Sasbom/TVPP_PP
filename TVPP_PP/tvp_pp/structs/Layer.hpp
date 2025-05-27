#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <cstddef>
#include <span>
#include <variant>
#include <memory>
#include "../../mio/single_include/mio/mio.hpp"
#include "Buffer.hpp"

enum struct repeat_t{
	NONE, // cut off
	REPEAT, // repeat from start
	PINGPONG, // Non frame repeating Ping pong 1 2 3 4 5 4 3 2 1
	HOLD, // Hold first / last frame.
};

struct Layer: std::enable_shared_from_this<Layer> {
	using buffer_var = std::variant<Buffer_DBOD, Buffer_SRAW, Buffer_SRAW_Repeat>;
	using buffer_vec = std::vector<std::unique_ptr<buffer_var>>;

	Layer(std::span<std::uint8_t const>& layer_info, std::size_t const & clip_idx, std::size_t const& layer_idx);

	std::size_t clip_idx{};
	std::size_t layer_idx{};

	buffer_vec frames{};
	// maps where DBOD and SRAW (unique frames) are stored in this->frames
	std::vector<std::size_t> frames_unique_idx{};

	std::string name{};
	std::string name_ascii{};

	std::size_t first_frame_num{};
	std::size_t frame_offset{};

	// Replace these repeat in/out w/ enum class type
	repeat_t repeat_in_type{};
	repeat_t repeat_out_type{};
	
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

	void read_into_layer(mio::ummap_source& mmap ,std::size_t& offset, FileInfo& file_info);
	void cache_layer_contents();
	void clear_layer_contents();
	void dump_frames(std::string const& prefix, std::string const& folder_name, FileInfo& file_info);
private:
	void name_from_LNAM_LNAW(std::span<std::uint8_t const> const& LNAM, std::span<std::uint8_t const> const& LNAW);
	void unpack_layerflags(std::uint8_t const& layer_flags);

	std::string repeat_t_to_str(repeat_t const& rep);
};