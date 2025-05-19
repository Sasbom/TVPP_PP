#include "Buffer.hpp"

bool Buffer::has_buffer() {
	return cache.has_value();
}

Buffer_SRAW::Buffer_SRAW(FileInfo& info, std::span<std::uint8_t const> const & source) {
	width = info.width;
	height = info.height;
	buffer_type = buffer_t::SRAW_FRAME;
	this->source = source;
}

Buffer_DBOD::Buffer_DBOD(FileInfo& info, std::span<std::uint8_t const> const & source) {
	width = info.width;
	height = info.height;
	buffer_type = buffer_t::DBOD_FRAME;
	this->source = source;
}

void Buffer_SRAW::unroll_source_to_cache()  {

}

void Buffer_DBOD::unroll_source_to_cache() {
	
}