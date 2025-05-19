// main.cpp: defines the entry point for the application.

#include <iostream>
#include <span>
#include "mio/single_include/mio/mio.hpp"
#include "tvp_pp/zlib_util.hpp"
#include "tvp_pp/RLE.hpp"
#include "tvp_pp/num_util.hpp"
#include "tvp_pp/file_util.hpp"
#include "tvp_pp/structs/FileInfo.hpp"
#include "tvp_pp/structs/ThumbInfo.hpp"
#include "tvp_pp/structs/Buffer.hpp"
#include "tvp_pp/structs/Shot.hpp"
#include "stb/stb_image_write.h"
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
	
	auto read_hdr2 = file_read_header(hdr2);
	auto thumbobj = ThumbInfo(read_hdr2);
	thumbobj.print_info();

	auto read_hdr3 = file_read_header(hdr3);
	auto shot = Shot(read_hdr3);
	shot.print_info();

	offset = 104649;
	auto dbod_span = seek_ZCHK_DBOD(mmap, offset);
	std::cout << dbod_span.size() << "\n";
	auto dbod = Buffer_DBOD(fileobj, dbod_span);
	auto buf = dbod.get_framebuffer();

	std::cout << "Buffer size: " << buf.size() << "\n";
	std::cout << dbod.width << " " << dbod.height << "\n";
	stbi_write_png("test_img.png", dbod.width, dbod.height,4, buf.data(), dbod.width*4);
	std::cout << "\n\nHello CMake.\n" << "mmapped file length: "<< mmap.length() << "\n";
	return 0;
}
