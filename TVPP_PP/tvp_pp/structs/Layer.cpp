#include "Layer.hpp"
#include "../num_util.hpp"
#include "../file_util.hpp"
#include <cmath>
#include <format>
#include <iostream>
#include <filesystem>
#include "../../stb/stb_image_write.h"
#include <fstream>

#ifdef VERBOSE
#define LOG(message) std::cout << message << "\n"; 
#else
#define LOG(message)
#endif


Layer::Layer(std::span<std::uint8_t const>& layer_info, std::size_t const& clip_idx = 0, std::size_t const& layer_idx = 0 ) {
	auto read_4 = [](auto it) {
		return bigend_cast_from_ints<std::uint32_t>(*it, *(it + 1), *(it + 2), *(it + 3));
	};
	
    this->clip_idx = clip_idx;
    this->layer_idx = layer_idx;

	auto it = layer_info.begin();

	auto LNAM_len = read_4(it + 4);
	it += 8;
    auto LNAM_span = std::span(it, it + LNAM_len);
    it += LNAM_len;
    auto LNAW_len = read_4(it + 4);
    it += 8;
    auto LNAW_span = std::span(it, it + LNAW_len);
    it += LNAW_len;

    this->name_from_LNAM_LNAW(LNAM_span, LNAW_span);

    //skip past LRHD
    it += 4;
    auto LRHD_len = read_4(it);
    auto end = it + 4 + LRHD_len;
    it += 4;
    auto LRHD_end = it;

    this->first_frame_num = read_4(it);
    this->frame_offset = static_cast<std::int32_t>(read_4(it+4)); // is signed.
    this->frames_amount = read_4(it + 8);

    this->opacity_byte = *(LRHD_end + 20-1);
    this->opacity = std::round((static_cast<double>(this->opacity_byte)/255.0)*100);

    this->blend_mode = static_cast<blendmode_t>(read_4(it + 60));

    std::uint8_t layer_flags = *(LRHD_end + 32-1);
    this->unpack_layerflags(layer_flags);

    this->repeat_out_type = static_cast<repeat_t>(*(LRHD_end + 46-1));
    this->repeat_in_type = static_cast<repeat_t>(*(LRHD_end + 54-1));
    this->group_id = *(LRHD_end + 58-1);
}

// I'm wondering if I can't just grab the LNAW and LNAM both.
// EDIT: I can.
void Layer::name_from_LNAM_LNAW(std::span<std::uint8_t const> const& lnam, std::span<std::uint8_t const> const& lnaw) {
    /*std::string collect{};
    std::size_t posm = 0;
    for (auto i = lnaw.begin(); i != lnaw.end(); i++) {
        if (*i == lnam[posm]) {
            collect += static_cast<char>(*i);
            posm++;
        }
        else {
            if (posm + 2 < lnam.size()) {
                while (*(i + 1) != lnam[posm + 2]) {
                    collect += static_cast<char>(*i);
                    i++;
                }
                i--;
                posm += 1;
                if (posm + 2 < lnam.size())
                    break;
            }
        }
    }
    name =  collect;*/

    name_ascii = std::string(lnam.begin(), lnam.end() - 1);
    name = std::string(lnaw.begin(), lnaw.end() - 1);
}

void Layer::unpack_layerflags(std::uint8_t const& layer_flags) {
    constexpr static std::uint8_t const INVIS = 1 << 0;
    constexpr static std::uint8_t const LIGHT = 1 << 1;
    constexpr static std::uint8_t const STNCL = 1 << 2;
    constexpr static std::uint8_t const LOCKD = 1 << 4;
    constexpr static std::uint8_t const POSLK = 1 << 5;
    constexpr static std::uint8_t const TRANS = 1 << 6;

    this->invisible = ((layer_flags & INVIS) != 0);
    this->lighttable = ((layer_flags & LIGHT) != 0);
    this->stencil = ((layer_flags & STNCL) != 0);
    this->locked = ((layer_flags & LOCKD) != 0);
    this->position_locked = ((layer_flags & POSLK) != 0);
    this->preserve_trans = ((layer_flags & TRANS) != 0);
};

std::string Layer::repeat_t_to_str(repeat_t const& rep) {
    switch (rep) {
    case repeat_t::NONE:
        return "NONE";
    case repeat_t::REPEAT:
        return "REPEAT";
    case repeat_t::PINGPONG:
        return "PINGPONG";
    case repeat_t::HOLD:
        return "HOLD";
    }
    return "NONE";
}

#ifdef DIFFERENCE // on msvc, difference is a macro in winuser.
#define RESTORE_DIFFERENCE
#undef DIFFERENCE
#endif

std::string Layer::blendmode_t_to_str(blendmode_t const& blend) {
    switch (blend) {
    case blendmode_t::COLOR:
        return "Color";
    case blendmode_t::BEHIND:
        return "Behind";
    case blendmode_t::ERASE:
        return "Erase";
    case blendmode_t::SHADE:
        return "Shade";
    case blendmode_t::LIGHT:
        return "Light";
    case blendmode_t::COLORIZE:
        return "Colorize";
    case blendmode_t::TINT:
        return "Tint";
    case blendmode_t::SATURATE2:
        return "Saturate2";
    case blendmode_t::VALUE:
        return "Value";
    case blendmode_t::ADD:
        return "Add";
    case blendmode_t::SUB:
        return "Sub";
    case blendmode_t::MULTIPLY:
        return "Multiply";
    case blendmode_t::SCREEN:
        return "Screen";
    case blendmode_t::REPLACE:
        return "Replace";
    case blendmode_t::SUBSTITUTE:
        return "Substitute";
    case blendmode_t::DIFFERENCE:
        return "Difference";
    case blendmode_t::DIVIDE:
        return "Divide";
    case blendmode_t::OVERLAY:
        return "Overlay";
    case blendmode_t::OVERLAY2:
        return "Overlay2";
    case blendmode_t::DODGE:
        return "Dodge";
    case blendmode_t::BURN:
        return "Burn";
    case blendmode_t::HARDLIGHT:
        return "Hard Light";
    case blendmode_t::SOFTLIGHT:
        return "Soft Light";
    case blendmode_t::GRAINEXTRACT:
        return "Grain Extract";
    case blendmode_t::GRAINMERGE:
        return "Grain Merge";
    case blendmode_t::SUBTRACT:
        return "Subtract";
    case blendmode_t::DARKENONLY:
        return "Darken Only";
    case blendmode_t::LIGHTENONLY:
        return "Lighten Only";
    case blendmode_t::ALPHADIFF:
        return "Alpha Diff";
    }
    return "COLOR";
}

#ifdef RESTORE_DIFFERENCE
#define DIFFERENCE 11
#undef RESTORE_DIFFERENCE
#endif

void Layer::print_info() {
    auto s = std::format(R"(Layer:
Name: {}
Frame Offset: {}
Frames in Layer: {}
Repeat In / Repeat Out: {} / {}
Group: {}

Opacity: {}%
Blend Mode: {}

Layer Flags:
Invisible: {}
Lighttable: {}
Stencil Mode: {}
Locked: {}
Position Locked: {}
Preserve Transparency: {}
)",
name, frame_offset, frames_amount,
repeat_t_to_str(repeat_in_type), repeat_t_to_str(repeat_out_type),
group_id, opacity, blendmode_t_to_str(blend_mode),
invisible, lighttable, stencil, locked, position_locked, preserve_trans
);
    std::cout << s;
}

void Layer::read_into_layer(mio::ummap_source& mmap, std::size_t& offset, FileInfo& fileinfo) {
    
    auto read_4 = [](std::uint8_t const* it) {
        return bigend_cast_from_ints<std::uint32_t>(*it, *(it + 1), *(it + 2), *(it + 3));
    };
    constexpr static std::uint32_t const ZCHK = 0x5A43484B;
    constexpr static std::uint32_t const DBOD = 0x44424F44;
    constexpr static std::uint32_t const SRAW = 0x53524157;
    constexpr static std::uint32_t const LEXT = 0x4C455854;

    auto it = mmap.begin() + offset;

    Buffer_SRAW_Repeat::buffer_source last{};

    while (true) {
        auto hdr = read_4(it);

        if (hdr == LEXT)
            break;

        if (hdr == ZCHK) {
            //std::cout << "ZCHK!\n";
            auto sig = read_4(it + 8);
            if (sig == DBOD) {
                LOG("DBOD!");
                // deal with DBOD
                auto dbod_source = seek_ZCHK_DBOD(mmap, offset);
                frames_unique_idx.push_back(frames.size());
                frames.push_back(std::make_unique<buffer_var>(Buffer_DBOD(fileinfo, dbod_source,shared_from_this())));
                last = &std::get<0>(*frames[frames.size()-1].get());
                it = mmap.begin() + offset;
                continue;
            }
            if (sig == SRAW) {
                // first, check for repeat magic bytes
                auto _c0 = *(it + 15);
                auto _2f = *(it + 23);
                auto _01 = *(it + 27);
                auto _64 = *(it + 31);
                if ((_c0 == 12) && (_2f == 47) && (_01 == 1) && (_64 == 100)) {
                    LOG("SRAW REPEAT!\n");
                    frames.push_back(std::make_unique<buffer_var>(Buffer_SRAW_Repeat(last)));
                    offset += 64;
                    it = mmap.begin() + offset;
                    continue;
                }
                else {
                    LOG("SRAW OG!");
                    auto sraw_source = seek_ZCHK_SRAW_VEC(mmap, offset);
                    frames_unique_idx.push_back(frames.size());
                    frames.push_back(std::make_unique<buffer_var>(Buffer_SRAW(fileinfo, sraw_source, shared_from_this(),frames_unique_idx.size()-1)));
                    last = &std::get<1>(*frames[frames.size() - 1].get());
                    it = mmap.begin() + offset;
                    continue;
                }
            }
        }
        offset++;
        it++;
    }
}

void Layer::cache_layer_contents() {
    for (auto& frame : frames) {
        auto ptr = frame.get();

        if (ptr->index() == 0) {
            auto& lyr = std::get<0>(*ptr);
            lyr.get_framebuffer();
        }
        else if (ptr->index() == 1) {
            auto& lyr = std::get<1>(*ptr);
            lyr.get_framebuffer();
        }
        else if (ptr->index() == 2) {
            auto& lyr = std::get<2>(*ptr);
            lyr.get_framebuffer();
        }
    }
}

void Layer::clear_layer_contents() {
    for (auto& frame : frames) {
        auto ptr = frame.get();

        if (ptr->index() == 0) {
            auto& lyr = std::get<0>(*ptr);
            lyr.clear_cache();
        }
        else if (ptr->index() == 1) {
            auto& lyr = std::get<1>(*ptr);
            lyr.clear_cache();
        }
        else if (ptr->index() == 2) {
            auto& lyr = std::get<2>(*ptr);
            lyr.clear_cache();
        }
    }
}

void Layer::dump_frames(std::string const& prefix, std::string const& folder_name, FileInfo& file_info) {
    namespace fs = std::filesystem;
    
    std::string path = folder_name+"\\";
    auto fpath = fs::path(folder_name);
    fs::create_directory(fpath);

    std::size_t framenr = frame_offset;

    auto pad = [](std::size_t const& num, std::size_t len = 4) {
        auto str = std::to_string(num);
        while (str.length() != len) {
            str.insert(str.begin(), '0');
        }
        return str;
    };

    for (auto& frame : frames) {
        auto fullpath = path + std::format("{}_{}_{}.png", name_ascii.c_str(), prefix, pad(framenr));
        auto fullpathbin = path + std::format("{}_{}_{}.bin", name_ascii.c_str(), prefix, pad(framenr));
        auto ptr = frame.get();

        std::cout << "Writing out " << fullpath << "\n";

        framebuf_raw_t fr{};
        if (ptr->index() == 0) {
            auto& lyr = std::get<0>(*ptr);
            fr = lyr.get_framebuffer();
        } else if (ptr->index() == 1) {
            auto& lyr = std::get<1>(*ptr);
            fr = lyr.get_framebuffer();
        }
        else if (ptr->index() == 2) {
            auto& lyr = std::get<2>(*ptr);
            fr = lyr.get_framebuffer();
        }
        //std::cout << "writing it. " << fr.size() << "\n";
        //auto f = std::ofstream(fullpathbin.c_str(), std::ios::binary);
        //f.write(reinterpret_cast<char*>(fr.data()), fr.size());
        //f.close();

        stbi_write_png(fullpath.c_str(), file_info.width, file_info.height, 4, fr.data(), 4 * file_info.width);
        framenr += 1;
    }

}

cache_t& Layer::in_range_cache(std::size_t const& frame) {
    auto ptr = frames.at(frame).get();

    if (ptr->index() == 0) {
        return std::get<0>(*ptr).cache;
    }
    else if (ptr->index() == 1) {
        return std::get<1>(*ptr).cache;
    }
    else if (ptr->index() == 2) {
        return std::get<2>(*ptr).get_ref_cache();
    }
    return this->EMPTY_CACHE;
}


// fetch cache at frame taking pre and post behavior into account
cache_t& Layer::get_cache_at_frame(std::size_t const& frame) {
    auto frames_amt = this->frames.size();
    
    if (frame < frame_offset) {
    // PRE-BEHAVIOR.
        switch (this->repeat_out_type) {
        case repeat_t::NONE: {
            return this->EMPTY_CACHE;
        }
        case repeat_t::REPEAT: {
            return this->EMPTY_CACHE;
        }
        case repeat_t::PINGPONG: {
            return this->EMPTY_CACHE;
        }
        case repeat_t::HOLD: {
            return this->EMPTY_CACHE;
        }
        }

    }
    else if (frame >= frame_offset + frames_amt) {
        // POST-BEHAVIOR.
        switch (this->repeat_out_type) {
        case repeat_t::NONE: {
            return this->EMPTY_CACHE;
        }
        case repeat_t::REPEAT: {
            return this->EMPTY_CACHE;
        }
        case repeat_t::PINGPONG: {
            return this->EMPTY_CACHE;
        }
        case repeat_t::HOLD: {
            return this->EMPTY_CACHE;
        }
        }
    }
    else {
        // ACTUAL IN-RANGE BEHAVIOR
        std::size_t cur_frame_idx = frame - frame_offset;
        return in_range_cache(cur_frame_idx);
    }
    
    return this->EMPTY_CACHE;
}
