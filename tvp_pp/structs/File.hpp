#pragma once
#include "Shot.hpp"
#include "FileInfo.hpp"
#include "ThumbInfo.hpp"
#include "Clip.hpp"
#include "../file_util.hpp"
#include <memory>

struct File {
	File(mio::ummap_source& mmap_file);
	File(std::string mmap_file_path);

	size_t offset{0};

	mio::ummap_source mmap{};

	FileInfo file_info{};
	ThumbInfo thumb_info{};
	Shot shot_info{};
	std::vector<std::unique_ptr<Clip>> clips{};

	void print_layers();

	void dump_file();
	void dump_file_mark();

private:
	void clip_cycle(mio::ummap_source& mmap_file);
};