#pragma once

#include <vector>
#include <span>
#include <optional>
#include <array>
#include <iostream>

#define BUF_OVER_NUL(offset,data) \
if (offset > data.size()) {\
	std::cerr << "Error reading stream at offset: " << offset << "\n";\
	return std::nullopt;\
}\

// Unroll VALID pixel RLE data. If not valid, returned value will contain std::nullopt.
std::optional<std::vector<std::uint8_t>> unroll_rle(std::span<std::uint8_t>& data, std::size_t const & init_offset = 0) {
	std::vector<std::uint8_t> buffer{};
	std::size_t offset = init_offset;

	auto read_1 = [&]() {
		auto result = data[offset];
		offset += 1;
		return result;
	};

	auto read_4 = [&]() {
		auto result = std::array<std::uint8_t, 4>{};
		memcpy(result.data(), data.data()+offset, sizeof(std::uint8_t)*4);
		offset += 4;
		return result;
	};

	auto repeat_insert_buffer = [&](std::size_t const& amount,auto&& ... insert) {
		for (std::size_t i{ 0 }; i < amount; i++) {
			(buffer.push_back(insert), ...);
		}
	};

	std::size_t amt = 0;
	bool repeat = false;

	while (offset < data.size()) {
		auto RLE_val = read_1();
		if (offset == data.size()) {
			break;
		}
		BUF_OVER_NUL(offset, data)

		if (RLE_val < 128) {
			amt = RLE_val + 1;
			repeat = false;
		}
		else {
			amt = 257 - RLE_val;
			repeat = true;
		}
		if (repeat) {
			auto&& [B, G, R, A] = read_4();
			BUF_OVER_NUL(offset, data)
			repeat_insert_buffer(amt, R, G, B, A);
		}
		else {
			for (std::size_t i{ 0 }; i < amt; i++) {
				auto&& [B, G, R, A] = read_4();
				BUF_OVER_NUL(offset, data)
				repeat_insert_buffer(1, R, G, B, A);
			}
		}
	}

	return buffer;
}