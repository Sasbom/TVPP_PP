#pragma once
#include <cstdint>
#include <vector>
#include <tuple>
#include <optional>
#include <variant>
#include <span>

using loc_t = std::tuple<std::size_t, std::size_t>;
using framebuf_raw_t = std::vector<uint8_t>;
using cache_t = std::optional<framebuf_raw_t>;

enum class buffer_t {
	DBOD_FILE,
    SRAW_FILE,
    DBOD_FRAME,
	SRAW_FRAME
};

enum class SRAW_block_t {
	ZERO,
	REPEAT,
};

// rolled up SRAW buffer
using sraw_buffer_el_t = std::variant<framebuf_raw_t, SRAW_block_t>;
using sraw_buffer_t = std::vector<sraw_buffer_el_t>;

struct Buffer {
	std::size_t width{};
	std::size_t height{};
	std::size_t stride{ 4 }; // RGBA 8bit
	buffer_t buffer_type{};

	std::span<std::uint8_t const> source;

	cache_t cache;

	virtual void unroll_source_to_cache() = 0;

};

struct Buffer_SRAW : public Buffer {
	constexpr static std::size_t SRAW_hdr_size = 24;
	std::size_t block_size{};

	sraw_buffer_t interim_buffer{};
};

struct Frame {
	loc_t location{};

	std::size_t duration{};
	std::size_t start{};

	cache_t cached_buffer{};
};