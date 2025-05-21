#include "Buffer.hpp"
#include "../zlib_util.hpp"
#include "../RLE.hpp"
#include "../num_util.hpp"
#include <iostream>
#include <cstring>

bool Buffer::has_buffer() {
	return cache.has_value();
}

Buffer_SRAW::Buffer_SRAW(FileInfo& info, source_t const & source) {
	width = info.width;
	height = info.height;
	buffer_type = buffer_t::SRAW_FRAME;
	this->source = source;
}

Buffer_SRAW_Repeat::Buffer_SRAW_Repeat(Buffer_SRAW& source){
	sraw_source = &source;
};

Buffer_SRAW_Repeat::Buffer_SRAW_Repeat(Buffer_DBOD& source) {
	sraw_source = &source;
};

Buffer_DBOD::Buffer_DBOD(FileInfo& info, source_t const & source) {
	width = info.width;
	height = info.height;
	buffer_type = buffer_t::DBOD_FRAME;
	this->source = source;
}

void Buffer_SRAW::unroll_source_to_cache()  {
	// for now, assume ZLIB
	if (source.index() != 0) {
		std::cout << "Source incorrect\n";
		return;
	}

	// source has to be a contiguously packed ZLIB block with multiple headers, no shenanigans in between like in DBOD
	auto src = std::get<0>(source);
	auto rolled_buffer = decompress_span_zlib(src);
	// SRAW [4 byt len] [block size] [len thumb + 4] [thumb w] [thumb h]

	auto read_4 = [](auto it) {
		return bigend_cast_from_ints<std::uint32_t>(*it, *(it + 1), *(it + 2), *(it + 3));
	};

	auto begin = rolled_buffer.begin();
	this->block_size = read_4(begin + 8);
	std::size_t thumbnail_offset = read_4(begin + 12);

	// start reading from thumnbail onwards into interim buffer.
	for (auto it = rolled_buffer.begin() + 20 + thumbnail_offset; it != rolled_buffer.end();/* it++ */) {
		std::size_t length = read_4(it);
		if (length == 0) {
			std::size_t repeat_type = read_4(it + 8);
			if (repeat_type > 0)
				this->interim_buffer.push_back(SRAW_block_t::REPEAT);
			else
				this->interim_buffer.push_back(SRAW_block_t::ZERO);
			it += 12;
			continue;
		}
		else {
			it += 4;
			auto rle_buf = std::span(it, it + length);
			auto buf = unroll_rle(rle_buf);
			if (buf.has_value()) {
				this->interim_buffer.push_back(buf.value());
			}
			else {
				this->interim_buffer.push_back(SRAW_block_t::FAILEDPARSE);
			}
			it += length;
		}
	}

	auto unpack_all = framebuf_raw_t{};
	unpack_all.resize(this->width * this->height * 4); //prealloc

	std::size_t stride = 4;

	std::size_t c{ 0 };
	framebuf_raw_t * last_buf = nullptr;

	std::size_t wblocks = std::ceil(this->width / this->block_size);
	std::size_t hblocks = std::ceil(this->height / this->block_size);
	
	auto unpack_block_into_frame = [&](std::size_t const& c, framebuf_raw_t& buf) {
		long int xb = c % wblocks;
		long int yb = std::floor(c / wblocks);

		std::size_t start_offset = ((yb * block_size * width) + (xb * block_size)) * stride;

		using lint = long int;

		long int xb_w = std::min(std::abs(lint(block_size) - (((xb + 1) * lint(block_size)) % lint(width))), lint(block_size));
		long int yb_w = std::min(std::abs(lint(block_size) - (((yb + 1) * lint(block_size)) % lint(height))), lint(block_size));

		std::size_t local_y{ 0 };
		//std::cout << "unrolling..\n";
		for (; local_y < yb_w; local_y++) {
			auto start_row = unpack_all.data() + start_offset + (local_y * width * stride);
			std::copy(buf.data() + (local_y * xb_w * stride), buf.data() + (xb_w * (local_y + 1) * stride), start_row);
		}
	};

	for (auto& el : this->interim_buffer) {
		if (el.index() == 0) {
			auto& buf = std::get<0>(el);
			last_buf = &buf;
			unpack_block_into_frame(c, buf);
		}
		else if (el.index() == 1) {
			auto action = std::get<1>(el);
			if (action == SRAW_block_t::ZERO) {
				// do nothing because preallocated with 0 memory.
			}
			else if (action == SRAW_block_t::REPEAT) {
				if (last_buf != nullptr) {
					auto& buf = *last_buf;
					unpack_block_into_frame(c, buf);
				}
			}
			else if (action == SRAW_block_t::FAILEDPARSE) {
				//std::cout << "FAILED PARSE!" << "\n";
				cache = std::nullopt;
				return;
			}
		}
		c++;
	}
	cache = unpack_all;
}

framebuf_raw_t Buffer_SRAW::get_framebuffer() {
	if (cache.has_value())
		return cache.value();

	this->unroll_source_to_cache();

	if (cache.has_value())
		return cache.value();
	else
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
		rolled_buffer.insert(rolled_buffer.end(), dec.begin(), dec.end());
	}

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

void Buffer_SRAW_Repeat::unroll_source_to_cache() {
	if (sraw_source.index() == 0)
		std::get<0>(sraw_source)->unroll_source_to_cache();
	if (sraw_source.index() == 1)
		std::get<1>(sraw_source)->unroll_source_to_cache();
}

framebuf_raw_t Buffer_SRAW_Repeat::get_framebuffer() {	
	if (sraw_source.index() == 0)
		return std::get<0>(sraw_source)->get_framebuffer();
	if (sraw_source.index() == 1)
		return std::get<1>(sraw_source)->get_framebuffer();
}