// main.cpp: defines the entry point for the application.

#include <iostream>
#include <span>
#include "mio/single_include/mio/mio.hpp"
#include "miniz/miniz.h"
#include "miniz/miniz_tinfl.h"
#include <vector>

bool decompress_multiple_zlib_streams(const uint8_t* input, std::size_t inputSize, std::vector<uint8_t>& output) {
    std::size_t offset = 0;

    while (offset < inputSize) {
        mz_stream stream;
        std::memset(&stream, 0, sizeof(stream));
        stream.next_in = input + offset;
        stream.avail_in = inputSize - offset;
        // Set up output buffer
        const std::size_t chunkSize = 4096;
        std::vector<uint8_t> outChunk(chunkSize);
        
        if (inflateInit(&stream) != Z_OK) {
        //    std::cerr << "Failed to initialize inflate" << std::endl;
        //    return false;
        //}

        int status;
        do {
            stream.next_out = outChunk.data();
            stream.avail_out = chunkSize;

            status = mz_inflate(&stream, Z_NO_FLUSH);
            if (status != Z_OK && status != Z_STREAM_END) {
                std::cerr << "Decompression error: " << status << std::endl;
                mz_inflateEnd(&stream);
                return false;
            }

            size_t have = chunkSize - stream.avail_out;
            output.insert(output.end(), outChunk.data(), outChunk.data() + have);

        } while (status != Z_STREAM_END);

        offset = stream.total_in; // Advance to next stream (if any)
        mz_inflateEnd(&stream);

        if (offset >= inputSize)
            break;

        // Move to the next stream's start (handle partial reads)
        input += stream.total_in;
        inputSize -= stream.total_in;
    }

    return true;
}

int main()
{
	
	mio::mmap_source mmap("C:/Users/Astudio/Documents/TVPaintTests/DeBal/bal_3.tvpp");
	std::size_t start_DBOD_block = 61907;
	std::size_t end_DBOD_block = 64891;

	auto DBOD_span = std::span(mmap.begin() + start_DBOD_block,mmap.begin() + end_DBOD_block);

	for (auto& it : DBOD_span) {
		std::cout << it;
	}
	
	std::cout << "\n\nHello CMake.\n" << "mmapped file length: "<< mmap.length() << "\n";
	return 0;
}
