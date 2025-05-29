#define NOMINMAX
#include <cmath>
#include "Buffer.hpp"
#include "../zlib_util.hpp"
#include "../RLE.hpp"
#include "../num_util.hpp"
#include <iostream>
#include <cstring>
#include <fstream>
#include <format>
#include "Layer.hpp"

SRAW_block_s::SRAW_block_s(SRAW_block_t const& SRAW_type, std::size_t const& element_nr = 9999, std::size_t const & source_frame = 0) {
	block_t = SRAW_type;
	this->element_nr = element_nr;
	this->source_frame = source_frame;
}

void Buffer::clear_cache() {
	// setting the optional to nullopt will free up resources
	this->cache = std::nullopt;
};

bool Buffer::has_buffer() {
	return cache.has_value();
}

framebuf_raw_t Buffer::sample_block(std::size_t index, std::size_t block_size) {
	if (!has_buffer()) {
		return framebuf_raw_t{};
	}
	
	std::size_t stride = 4;

	std::size_t wblocks = std::ceil(this->width / block_size);
	std::size_t hblocks = std::ceil(this->height / block_size);

	long int xb = index % wblocks;
	long int yb = std::floor(index / wblocks);

	std::size_t start_offset = (yb * block_size * width * stride) + (xb * block_size * stride);
	using lint = long int;

	long int xb_w = std::min(block_size, width - xb * block_size);
	long int yb_w = std::min(block_size, height - yb * block_size);

	auto& from_buffer = this->cache.value();

	framebuf_raw_t to_buffer{};
	to_buffer.resize(xb_w * yb_w * stride);

	for (std::size_t y{ 0 }; y < yb_w; y++) {
		auto start_row = from_buffer.data() + start_offset + (y * width * stride);
		memcpy(to_buffer.data()+(y*stride*xb_w), start_row, (xb_w * stride));
	}
	return to_buffer;
}

Buffer_SRAW::Buffer_SRAW(FileInfo& info, source_t const & source, std::shared_ptr<Layer> layer, std::size_t const& frame_nr_unique = 0) : layer(layer) {
	width = info.width;
	height = info.height;
	buffer_type = buffer_t::SRAW_FRAME;
	this->source = source;
	this->frame_nr_unique = frame_nr_unique;
}

void Buffer_SRAW::clear_interim_buffer() {
	// Clear interim cache.
	this->interim_buffer.clear();
	this->interim_buffer.shrink_to_fit();
}

Buffer_SRAW_Repeat::Buffer_SRAW_Repeat(buffer_source source) {
	sraw_source = source;
};

Buffer_SRAW_Repeat::Buffer_SRAW_Repeat(buffer_source source, bool const& is_from_repeatimages) {
	sraw_source = source;
	this->is_from_repeatimages = true;
}

Buffer_SRAW_Repeat::Buffer_SRAW_Repeat(Buffer_SRAW& source) {
	sraw_source = &source;
};

Buffer_SRAW_Repeat::Buffer_SRAW_Repeat(Buffer_DBOD& source) {
	sraw_source = &source;
};

cache_t& Buffer_SRAW_Repeat::get_ref_cache() {
	switch (sraw_source.index()) {
	case 0: {
		return std::get<0>(sraw_source)->cache;
	}
	case 1: {
		return std::get<1>(sraw_source)->cache;
	}
	}
	
	return cache; //won't hold anything.
}

Buffer_DBOD::Buffer_DBOD(FileInfo& info, source_t const& source, std::shared_ptr<Layer> layer): layer(layer) {
	width = info.width;
	height = info.height;
	buffer_type = buffer_t::DBOD_FRAME;
	this->source = source;
}

void Buffer_SRAW::unroll_source_to_cache()  {
	// for now, assume ZLIB
	if (source.index() != 1) {
		std::cout << "Source incorrect\n";
		return;
	}
	// source has to be a contiguously packed ZLIB block with multiple headers, no shenanigans in between like in DBOD
	// EDIT: Nevermind. This can happen all the same.
	//auto src = std::get<0>(source);
	//auto rolled_buffer = decompress_span_zlib(src);

	auto src = std::get<1>(source);
	auto rolled_buffer = framebuf_raw_t{};

	for (auto& src_span : src) {
		auto dec = decompress_span_zlib(src_span);
		rolled_buffer.insert(rolled_buffer.end(), dec.begin(), dec.end());
	}

	// SRAW [4 byt len] [block size] [len thumb + 4] [thumb w] [thumb h]

	auto read_4 = [](auto it) {
		return bigend_cast_from_ints<std::uint32_t>(*it, *(it + 1), *(it + 2), *(it + 3));
	};

	auto begin = rolled_buffer.begin();
	this->block_size = read_4(begin + 8);
	std::size_t thumbnail_offset = read_4(begin + 12);

	//std::ofstream file("sraw_dump_buffer.bin", std::ios::binary);
	//file.write(reinterpret_cast<const char*>(rolled_buffer.data()), rolled_buffer.size());
	//file.close();

	auto check_valid = [&](auto it) {
		for (std::size_t i{ 0 }; i < 4; i++) {
			if (it + i == rolled_buffer.end()) {
				return false;
			}
		}
		return true;
	};
	std::size_t offset = 20 + thumbnail_offset;
	// start reading from thumnbail onwards into interim buffer.
	for (auto it = rolled_buffer.begin() + 20 + thumbnail_offset; it != rolled_buffer.end();/* it++ */) {
		
		if (!check_valid(it))
			break;

		std::size_t length = read_4(it);
		if (length == 0) {
			std::size_t source_frame = read_4(it + 4);
			std::size_t repeat_type = read_4(it + 8);
			// ALWAYS repeat type.
			this->interim_buffer.push_back(SRAW_block_s(SRAW_block_t::REPEAT,repeat_type,source_frame));
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
				this->interim_buffer.push_back(SRAW_block_s(SRAW_block_t::FAILEDPARSE));
				std::cout << "FAILED\n";
			}
			it += length;

		}
	}

	auto unpack_all = framebuf_raw_t{};
	unpack_all.resize(this->width * this->height * 4,255); //prealloc

	std::size_t stride = 4;

	std::size_t c{ 0 };
	framebuf_raw_t * last_buf = nullptr;

	std::size_t wblocks = std::ceil(this->width / this->block_size);
	std::size_t hblocks = std::ceil(this->height / this->block_size);
	


	auto unpack_block_into_frame = [&](std::size_t const& index, framebuf_raw_t& buffer) {
		long int xb = index % wblocks;
		long int yb = std::floor(index / wblocks);

		std::size_t start_offset = (yb * block_size * width * stride) + (xb * block_size * stride);
		using lint = long int;

		long int xb_w = std::min(block_size, width - xb * block_size);
		long int yb_w = std::min(block_size, height - yb * block_size);

		std::size_t local_y{ 0 };
		for (; local_y < yb_w; local_y++) {
			auto start_row = unpack_all.data() + start_offset + (local_y * width * stride);
			memcpy(start_row, buffer.data() + (local_y * xb_w * stride), (xb_w* stride));
			//std::copy(buffer.data() + (local_y * xb_w * stride), buffer.data() + (xb_w * (local_y + 1) * stride), start_row);
		}
	};

	std::size_t idx{ 0 };
	for (auto& el : this->interim_buffer) {
		if (el.index() == 0) {
			unpack_block_into_frame(idx, std::get<0>(el));
			idx++;
		}
		else if (el.index() == 1) {
			auto action = std::get<1>(el);
			if (action.block_t == SRAW_block_t::ZERO) {
				// UNREACHABLE LEGACY: NO SUCH THING AS ZERO.
				std::cout << "ZERO! " << idx << "\n";
				// do nothing because preallocated with 0 memory.
			}
			else if (action.block_t == SRAW_block_t::REPEAT) {
				if (action.element_nr < this->interim_buffer.size()) {
					if (action.source_frame == 0) {
						auto ref_el = this->interim_buffer[action.element_nr];
						if (ref_el.index() == 0) {
							unpack_block_into_frame(idx, std::get<0>(ref_el));
						}
						else {
							std::cout << "TIME TRAVEL FAILED!\n";
						}
					}
					else if(auto ulayer = this->layer.lock()){
						// IT SEEMS THAT THE SOURCE FRAME IS AN OFFSET LOOKING BACK.
						auto real_frame = ulayer->frames_unique_idx.at(this->frame_nr_unique - action.source_frame);
						auto& source = ulayer->frames.at(real_frame);
						if (source.get()->index() == 0) {
							auto& dbod = std::get<0>(*source.get());
							auto buf = dbod.sample_block(action.element_nr, block_size);
							unpack_block_into_frame(idx, buf);
							this->interim_buffer[idx] = buf;
						}
						else if (source.get()->index() == 1) {
							auto& sraw = std::get<1>(*source.get());
							auto buf = sraw.sample_block(action.element_nr, block_size);
							unpack_block_into_frame(idx, buf);
							this->interim_buffer[idx] = buf;
						}
					}
				}
			}
			else if (action.block_t == SRAW_block_t::FAILEDPARSE) {
				std::cout << "FAILED PARSE!" << "\n";
				cache = std::nullopt;
				return;
			}
			idx++;
		}
		//break;
		c++;
	}

	// Clear interim cache.
	this->clear_interim_buffer();

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