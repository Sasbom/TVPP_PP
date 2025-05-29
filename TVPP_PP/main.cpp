#include "mio/single_include/mio/mio.hpp"
#include "tvp_pp/structs/File.hpp"


int main()
{
	//mio::basic_mmap_source<std::uint8_t> mmap("C:/Users/Astudio/Documents/TVPaintTests/DeBal/bal_3.tvpp");
	//mio::basic_mmap_source<std::uint8_t> mmap("C:/Users/Astudio/Documents/TVPaintTests/10_2clips.tvpp");
	mio::basic_mmap_source<std::uint8_t> mmap("C:/Users/Astudio/Documents/TVPaintTests/TheFace_repeatframes3.tvpp");
	auto tvp_file = File(mmap);
	//tvp_file.print_layers();
	tvp_file.dump_file_mark();
	return 0;
}
