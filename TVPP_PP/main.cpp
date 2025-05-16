// main.cpp: defines the entry point for the application.

#include <iostream>
#include <span>
#include "mio/single_include/mio/mio.hpp"
#include "tvp_pp/zlib_util.hpp"
#include "tvp_pp/RLE.hpp"
#include "tvp_pp/num_util.hpp"
#include "tvp_pp/buffer.hpp"
#include "tvp_pp/file_util.hpp"
#include "tvp_pp/structs/FileInfo.hpp"
#include <fstream>

int main()
{
	mio::basic_mmap_source<std::uint8_t> mmap("C:/Users/Astudio/Documents/TVPaintTests/DeBal/bal_3.tvpp");
	
	std::size_t offset = 0;
	// file info
	auto hdr = seek_header(mmap,offset);
	std::cout << offset << "\n";
	// main thumbnail size
	auto hdr2 = seek_header(mmap,offset);
	std::cout << offset << "\n";
	// main thumbnail
	auto thumb1 = seek_3byteimbuffer(mmap, offset);
	std::cout << offset << "\n";
	std::cout << "after image buffer\n";
	auto hdr3 = seek_header(mmap, offset, 4, 4096);

	std::cout << offset << "\n";
	auto read_hdr = file_read_header(hdr);
	auto fileobj = FileInfo(read_hdr);
	fileobj.print_info();
	
	auto read_hdr3 = file_read_header(hdr3);
	for (auto i : read_hdr3) {
	 	std::cout << "e:" << i << "\n";
	}


	//std::size_t start_DBOD_block = 61907; //63435;
	//std::size_t end_DBOD_block = 63656;

	std::size_t start_DBOD_block = 63696; //63435;
	std::size_t end_DBOD_block = 63720;

	auto DBOD_span = std::span(mmap.begin() + start_DBOD_block,mmap.begin() + end_DBOD_block);

    //decompress_multiple_zlib_streams(DBOD_span.data(),DBOD_span.size(), decompressed_val);
	auto decompressed_val = decompress_span_zlib(DBOD_span);

	auto headerless_span = std::span(decompressed_val.begin(), decompressed_val.end());
	std::cout << "size: " << headerless_span.size() << "\n";
	
	//for (auto& it : decompressed_data.value()) {
	//    std::cout << it;
	//}

	std::ofstream srawfile("test_dump_SRAW.bin", std::ios::binary);
	srawfile.write(reinterpret_cast<char*>(headerless_span.data()), headerless_span.size());

	srawfile.close();

	return 0;
	std::cout << swap_endianness_uintx(std::uint16_t(123)) << '\n';

	auto unroll = unroll_rle(headerless_span,8);

	std::cout << unroll.has_value();

	auto unrollval = unroll.value();
	std::ofstream file("test_dump_img.bin", std::ios::binary);
	file.write(reinterpret_cast<char*>(unrollval.data()), unrollval.size());

	file.close();

	std::cout << "\n\nHello CMake.\n" << "mmapped file length: "<< mmap.length() << "\n";
	return 0;
}
