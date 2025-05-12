#pragma once
#include <cstdint>
#include <vector>
#include <tuple>
#include <optional>

using loc_t = std::tuple<std::size_t, std::size_t>;
using framebuf_raw_t = std::vector<uint8_t>;
using cache_t = std::optional<framebuf_raw_t>;

enum class buffer_t {
	DBOD_FILE,
    SRAW_FILE,
    DBOD_FRAME,
	SRAW_FRAME
};

struct Buffer {
	loc_t location{};
	buffer_t buffer_type{};

	std::size_t duration{};
	std::size_t start{};

	cache_t cached_buffer{};
};