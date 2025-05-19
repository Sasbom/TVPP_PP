#include "Buffer.hpp"
#include "../zlib_util.hpp"

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
}

framebuf_raw_t Buffer_DBOD::get_framebuffer() {
	if (cache.has_value())
		return cache.value();
	//placeholder
	return framebuf_raw_t{};
}