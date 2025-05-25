#include "File.hpp"
#include <filesystem>
#include <iostream>
File::File(mio::ummap_source& mmap) {
	offset = 0;
	
	auto file_hdr = seek_header(mmap, offset);
	auto file_hdr_read = file_read_header(file_hdr);
	file_info = FileInfo(file_hdr_read);
	
	auto thumb_hdr = seek_header(mmap, offset);
	auto thumb_hdr_read = file_read_header(thumb_hdr);
	thumb_info = ThumbInfo(thumb_hdr_read);
	
	seek_3byteimbuffer(mmap, offset); // skip past thumbnail.
	
	auto shot_hdr = seek_header(mmap, offset, 4, 4096);
	auto shot_hdr_read = file_read_header(shot_hdr);
	shot_info = Shot(shot_hdr_read);

	clip_cycle(mmap);
}

void File::clip_cycle(mio::ummap_source& mmap) {
	auto next = LEXT_AFTER::CLIP;
	
	while (next == LEXT_AFTER::CLIP) {
		auto clip_hdr = seek_header(mmap, offset, 1, 2048);
		auto clip_hdr_read = file_read_header(clip_hdr);
		clips.push_back(std::make_unique<Clip>(clip_hdr_read));
		clips.back()->print_info();
		// Skip FORM TVPP XS24
		std::cout << offset << "\n";
		seek_3bytimbuffer_XS24(mmap, offset);
		
		next = LEXT_AFTER::LAYER;
		
		while (next == LEXT_AFTER::LAYER) {
			auto layer_hdr = seek_layer_header(mmap, offset);
			clips.back()->layers.push_back(std::make_unique<Layer>(layer_hdr));
			clips.back()->layers.back()->read_into_layer(mmap, offset, file_info);

			next = seek_LEXT_UDAT_STCK_FCFG(mmap, offset);
			std::cout << int(next) << "\n";
		}
	}
}

void File::dump_file() {
	namespace fs = std::filesystem;
	std::string base = "dump";

	file_info.print_info();

	std::cout << "Dumping all file contents...\n";
	
	for (auto& c : clips) {
		std::string basefolder = std::format("{}_{}", base, c->name.c_str());
		auto fpath = fs::path(basefolder);
		fs::create_directory(fpath);
		
		for (auto& l : c->layers) {
			std::string folder = std::format("{}_{}\\{}", base, c->name.c_str(),l->name_ascii.c_str());
			std::cout << l->name << "\n";
			l->dump_frames("filedump", folder.c_str(), file_info);
		}
	}
}