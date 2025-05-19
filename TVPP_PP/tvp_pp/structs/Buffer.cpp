#include "Buffer.hpp"
#include "../zlib_util.hpp"
#include "../RLE.hpp"
#include <iostream>

bool Buffer::has_buffer() {
	return cache.has_value();
}

Buffer_SRAW::Buffer_SRAW(FileInfo& info, source_t const & source) {
	width = info.width;
	height = info.height;
	buffer_type = buffer_t::SRAW_FRAME;
	this->source = source;
}

Buffer_DBOD::Buffer_DBOD(FileInfo& info, source_t const & source) {
	width = info.width;
	height = info.height;
	buffer_type = buffer_t::DBOD_FRAME;
	this->source = source;
}

void Buffer_SRAW::unroll_source_to_cache()  {

}

framebuf_raw_t Buffer_SRAW::get_framebuffer() {
	if (cache.has_value())
		return cache.value();
	//placeholder
	return framebuf_raw_t{};
}

void Buffer_DBOD::unroll_source_to_cache() {
	// for now, assume ZLIB

	// source has to be a collection of ZLIB headers 
	if (source.index() != 1) {
		std::cout << "Source incorrect\n";
		return;
	}

	auto src = std::get<1>(source);
	auto rolled_buffer = framebuf_raw_t{};
	
	for (auto& src_span : src) {
		auto dec = decompress_span_zlib(src_span);
		std::cout << "decompressed buffer: " << dec.size() << "\n";
		rolled_buffer.insert(rolled_buffer.end(), dec.begin(), dec.end());
	}
	std::cout << "Rolled buffer: " << rolled_buffer.size() << "\n";

	auto view = std::span(rolled_buffer.begin(), rolled_buffer.end());
	cache = unroll_rle(view,8); // offset by 8, DBOD header
}

framebuf_raw_t Buffer_DBOD::get_framebuffer() {
	if (cache.has_value())
		return cache.value();
	
	this->unroll_source_to_cache();

	if (cache.has_value())
		return cache.value();
	else
		return framebuf_raw_t{};
}