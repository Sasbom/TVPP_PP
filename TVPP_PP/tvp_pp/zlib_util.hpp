#pragma once

#include <span>
#include <vector>
#include "../miniz/miniz.h"
#include <cstring>
#include <iostream>
#include <optional>

#ifdef DEPRECATED
bool decompress_zlib(uint8_t const * input, size_t input_size, std::vector<uint8_t>& output) {
    size_t offset = 0;

    while (offset < input_size) {
        mz_stream stream;
        std::memset(&stream, 0, sizeof(stream));
        stream.next_in = input + offset;
        stream.avail_in = input_size - offset;

        // Set up output buffer (4mb / chunk)
        const size_t chunkSize = 4096;
        std::vector<uint8_t> outChunk(chunkSize);

        if (inflateInit(&stream) != Z_OK) {
            std::cerr << "Failed to initialize inflate" << std::endl;
            return false;
        }

        int status;
        do {
            stream.next_out = outChunk.data();
            stream.avail_out = chunkSize;

            status = inflate(&stream, Z_NO_FLUSH);
            if (status != Z_OK && status != Z_STREAM_END) {
                std::cerr << "Decompression error: " << status << std::endl;
                mz_inflateEnd(&stream);
                return false;
            }

            size_t have = chunkSize - stream.avail_out;
            output.insert(output.end(), outChunk.data(), outChunk.data() + have);

        } while (status != Z_STREAM_END);

        offset = stream.total_in; // Advance to next stream (if any)
        inflateEnd(&stream);

        if (offset >= input_size)
            break;

        // Move to the next stream's start (handle partial reads)
        input += stream.total_in;
        input_size -= stream.total_in;
    }

    return true;
}
#endif

std::vector<size_t> inline find_headers(std::span<std::uint8_t const>& input_span) {
    std::vector<size_t> collect{};

    size_t c{};
    for (auto it = input_span.begin(); it != input_span.end() - 1; it++, c++) {
        auto el1 = *it;
        auto el2 = *(it + 1);
        // std::cout << el1 << " " << el2 << "\n";
        if ((el1 == 0x78) && (el2 == 0x01)) {
            collect.push_back(c);
        }
    }
    return collect;
}

// Seek headers and concatenate till everything is done.
std::vector<std::uint8_t> inline decompress_span_zlib(std::span<std::uint8_t const>& input_span) {
    auto hdrs = find_headers(input_span);
    std::vector<std::uint8_t> output{};

    for (auto h : hdrs) {
        std::size_t offset = h;
        std::size_t input_size = input_span.size();
        auto input = input_span.data();

        while (offset < input_size) {
            mz_stream stream;
            std::memset(&stream, 0, sizeof(stream));

            stream.next_in = input + offset;
            stream.avail_in = input_size - offset;

            // Set up output buffer (4mb / chunk)
            std::size_t const chunkSize = 4096;
            std::vector<std::uint8_t> outChunk(chunkSize);

            if (inflateInit(&stream) != Z_OK) {
                std::cerr << "Failed to initialize inflate" << std::endl;
                return std::vector<std::uint8_t>{};
            }

            int status;
            do {
                stream.next_out = outChunk.data();
                stream.avail_out = chunkSize;

                status = inflate(&stream, Z_NO_FLUSH);
                if (status != Z_OK && status != Z_STREAM_END) {
                    std::cerr << "Decompression error: " << status << std::endl;
                    inflateEnd(&stream);
                    return std::vector<std::uint8_t>{};
                }

                std::size_t have = chunkSize - stream.avail_out;
                output.insert(output.end(), outChunk.begin(), outChunk.begin() + have);

            } while (status != Z_STREAM_END);

            offset += stream.total_in; // Advance to next stream (if any)
            inflateEnd(&stream);

            if (offset >= input_size)
                break;

            input += stream.total_in;
            input_size -= stream.total_in;
        }
    }

    return output;
}