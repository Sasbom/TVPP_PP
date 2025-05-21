#pragma once
#include <cstdint>
#include <vector>
#include <tuple>
#include <optional>
#include <variant>
#include <span>
#include "FileInfo.hpp"

using loc_t = std::tuple<std::size_t, std::size_t>;
using framebuf_raw_t = std::vector<uint8_t>;
using cache_t = std::optional<framebuf_raw_t>;
using flat_source_t = std::span<std::uint8_t const>;
using vec_source_t = std::vector<std::span<std::uint8_t const>>;
using source_t = std::variant<flat_source_t, vec_source_t>;


enum class buffer_t {
	DBOD_FILE,
    SRAW_FILE,
    DBOD_FRAME,
	SRAW_FRAME
};

enum class SRAW_block_t {
	ZERO,
	REPEAT,
	FAILEDPARSE
};

// rolled up SRAW buffer
using sraw_buffer_el_t = std::variant<framebuf_raw_t, SRAW_block_t>;
using sraw_buffer_t = std::vector<sraw_buffer_el_t>;

struct Buffer {

	std::size_t width{};
	std::size_t height{};
	std::size_t stride{ 4 }; // RGBA 8bit

	buffer_t buffer_type{};

	source_t source;

	cache_t cache;

	bool has_buffer();

	// override to define unroll behavior
	virtual void unroll_source_to_cache() = 0;
	virtual framebuf_raw_t get_framebuffer() = 0;
};

struct Buffer_SRAW : public Buffer {
	Buffer_SRAW(FileInfo& info, source_t const & source);

	constexpr static std::size_t SRAW_hdr_size = 24;
	std::size_t block_size{};

	sraw_buffer_t interim_buffer{};

	void unroll_source_to_cache() override;
	framebuf_raw_t get_framebuffer() override;
};

struct Buffer_DBOD : public Buffer {
	Buffer_DBOD(FileInfo& info, source_t const & source);

	constexpr static std::size_t DBOD_hdr_size = 8;

	void unroll_source_to_cache() override;
	framebuf_raw_t get_framebuffer() override;
};

// always repeats 
struct Buffer_SRAW_Repeat: public Buffer{
	// implements the buffer interface
	using buffer_source = std::variant<Buffer_DBOD*, Buffer_SRAW*>;
	Buffer_SRAW_Repeat(Buffer_DBOD& source_sraw);
	Buffer_SRAW_Repeat(Buffer_SRAW& source_sraw);

	buffer_source sraw_source;

	void unroll_source_to_cache() override;
	framebuf_raw_t get_framebuffer() override;
};

// Parses a layer, indexing where frames are stored in mapped memory offsets.
// The first "Frame" is always of DBOD type.
// TODO: The presence or absence of ZCHK should eventually be taken into account.
//       We are assuming "TVPaint" (aka, zlib 78 01) compression is on right now.
//struct Layer {
//	loc_t location{};
//
//	std::size_t duration{};
//	std::size_t start{};
//
//	cache_t cached_buffer{};
//};