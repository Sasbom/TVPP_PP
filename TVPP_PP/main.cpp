// main.cpp: defines the entry point for the application.

#include <iostream>
#include <span>
#include "mio/single_include/mio/mio.hpp"
#include "tvp_pp/zlib_util.h"
#include "tvp_pp/RLE.h"
#include <fstream>

int main()
{
	
	mio::basic_mmap_source<std::uint8_t> mmap("C:/Users/Astudio/Documents/TVPaintTests/DeBal/bal_3.tvpp");
	std::size_t start_DBOD_block = 61907; //63435;
	std::size_t end_DBOD_block = 63656;

	auto DBOD_span = std::span(mmap.begin() + start_DBOD_block,mmap.begin() + end_DBOD_block);

    //decompress_multiple_zlib_streams(DBOD_span.data(),DBOD_span.size(), decompressed_val);
	auto decompressed_val = decompress_span_zlib(DBOD_span);

	auto headerless_span = std::span(decompressed_val.begin(), decompressed_val.end());
	std::cout << "size: " << headerless_span.size() << "\n";
	
	//for (auto& it : decompressed_data.value()) {
	//    std::cout << it;
	//}

	auto unroll = unroll_rle(headerless_span,8);

	std::cout << unroll.has_value();

	auto unrollval = unroll.value();
	std::ofstream file("test_dump_img.bin", std::ios::binary);
	file.write(reinterpret_cast<char*>(unrollval.data()), unrollval.size());

	file.close();

	std::cout << "\n\nHello CMake.\n" << "mmapped file length: "<< mmap.length() << "\n";
	return 0;
}
