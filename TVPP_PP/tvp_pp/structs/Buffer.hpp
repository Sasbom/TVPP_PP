#pragma once
#include <cstdint>
#include <vector>
#include <tuple>
#include <optional>
#include <variant>
#include <span>
#include <memory>
#include "FileInfo.hpp"

struct Layer;

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

struct SRAW_block_s {
	SRAW_block_s(SRAW_block_t const& SRAW_type, std::size_t const & element_nr, std::size_t const& source_frame);
	SRAW_block_t block_t{};
	std::size_t element_nr{};
	std::size_t source_frame{};
};

// rolled up SRAW buffer
using sraw_buffer_el_t = std::variant<framebuf_raw_t, SRAW_block_s>;
using sraw_buffer_t = std::vector<sraw_buffer_el_t>;


struct Buffer {
	std::size_t width{};
	std::size_t height{};
	std::size_t stride{ 4 }; // RGBA 8bit

	buffer_t buffer_type{};

	source_t source;

	cache_t cache;

	void clear_cache();
	bool has_buffer();

	framebuf_raw_t sample_block(std::size_t index, std::size_t block_size);

	// override to define unroll behavior
	virtual void unroll_source_to_cache() = 0;
	virtual framebuf_raw_t get_framebuffer() = 0;
};

struct Buffer_SRAW : public Buffer {
	Buffer_SRAW(FileInfo& info, source_t const& source, std::shared_ptr<Layer> layer, std::size_t const & frame_nr_unique);

	constexpr static std::size_t SRAW_hdr_size = 24;
	std::size_t block_size{};

	sraw_buffer_t interim_buffer{};
	void clear_interim_buffer();

	std::weak_ptr<Layer> layer;
	std::size_t frame_nr_unique{};

	void unroll_source_to_cache() override;
	framebuf_raw_t get_framebuffer() override;
};

struct Buffer_DBOD : public Buffer {
	Buffer_DBOD(FileInfo& info, source_t const & source, std::shared_ptr<Layer> layer);

	constexpr static std::size_t DBOD_hdr_size = 8;

	std::weak_ptr<Layer> layer;

	void unroll_source_to_cache() override;
	framebuf_raw_t get_framebuffer() override;
};

// always repeats 
// NOTE: DO RESEARCH INTO WHAT "REPEAT IMAGES" DOES WHEN PARSING.
// Officially these frames are referred to as "Exposure cells",
// the repeat images function could reference another SRAW/DBOD layer but I have yet to figure out where. 
// https://doc.tvpaint.com/docs/animation-advanced-functions/repeat-images-function
struct Buffer_SRAW_Repeat: public Buffer{
	// implements the buffer interface
	using buffer_source = std::variant<Buffer_DBOD*, Buffer_SRAW*>;
	Buffer_SRAW_Repeat(Buffer_DBOD& source_sraw);
	Buffer_SRAW_Repeat(Buffer_SRAW& source_sraw);
	Buffer_SRAW_Repeat(buffer_source source_sraw);
	Buffer_SRAW_Repeat(buffer_source source_sraw, bool const & is_from_repeatimages);

	buffer_source sraw_source;

	bool is_from_repeatimages{ false };

	void unroll_source_to_cache() override;
	framebuf_raw_t get_framebuffer() override;

	// reference cache grab
	cache_t& get_ref_cache();
};