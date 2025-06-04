#pragma once
#include <unordered_map>
#include <iostream>
#include "../utfcpp/utf8.h"
#include <span>
#include <cstdint>
#include "../mio/single_include/mio/mio.hpp"
#include "num_util.hpp"
#include <string>
#include <format>
#include "data.hpp"
#include <utility>

enum class LEXT_AFTER {
	LAYER,
	CLIP,
	TVP_EOF,
};

std::span<std::uint8_t const> seek_header(mio::ummap_source& mmap_file, std::size_t& offset, std::size_t skips = 1, std::size_t max_read = 100);

std::span<std::uint8_t const> seek_3byteimbuffer(mio::ummap_source& mmap_file, std::size_t& offset, std::size_t skips = 0);

std::span<std::uint8_t const> seek_ZCHK_SRAW(mio::ummap_source& mmap_file, std::size_t& offset);

std::vector<std::span<std::uint8_t const>> seek_ZCHK_SRAW_VEC(mio::ummap_source& mmap_file, std::size_t& offset);

std::vector<std::span<std::uint8_t const>> seek_ZCHK_DBOD(mio::ummap_source& mmap_file, std::size_t& offset);

LEXT_AFTER seek_LEXT_UDAT_STCK_FCFG(mio::ummap_source& mmap_file, std::size_t& offset);

std::span<std::uint8_t const> seek_3bytimbuffer_XS24(mio::ummap_source& mmap_file, std::size_t& offset);

std::span<std::uint8_t const> seek_layer_header(mio::ummap_source& mmap_file, std::size_t& offset);

std::vector<std::string> file_read_header(std::span<std::uint8_t const>& header_keyvalue_section);

