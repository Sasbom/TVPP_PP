#include "Layer.hpp"
#include "../num_util.hpp"
#include "../file_util.hpp"
#include "../zlib_util.hpp"
#include <cmath>
#include <format>
#include <iostream>
#include <filesystem>
#include "../../stb/stb_image_write.h"
#include <fstream>
#include <random>
#include <functional>

#ifdef LAYER_VERBOSE
#define LOG(message) std::cout << message << "\n"; 
#else
#define LOG(message)
#endif

#define NOMINMAX


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

    auto read_4_b = [](auto it) {
        return bigend_cast_from_ints<std::uint32_t>(*it, *(it + 1), *(it + 2), *(it + 3));
    };
    
    auto limit_to_zero = [](auto&& n) {
        return (n > 0) ? n : 0;
    };
    
    constexpr static std::uint32_t const ZCHK = 0x5A43484B;
    constexpr static std::uint32_t const DBOD = 0x44424F44;
    constexpr static std::uint32_t const SRAW = 0x53524157;
    constexpr static std::uint32_t const LEXT = 0x4C455854;

    auto it = mmap.begin() + offset;

    Buffer_SRAW_Repeat::buffer_source last{};

    bool repeat_images{ false };
    SRAW_repeatimages_t mode = SRAW_repeatimages_t::LOOP; // default
    std::size_t repeat_images_length{ 0 };
    long int repeat_images_start_index{ 0 };

    std::size_t random_hash = std::hash<std::string>()(std::format("{}{}{}", this->clip_idx, this->layer_idx, this->name));
    auto twister = std::mt19937();
    twister.seed(static_cast<std::mt19937::result_type>(random_hash));
    
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
                // turn off repeat images mode.
                repeat_images = false;
                mode = SRAW_repeatimages_t::LOOP; // default
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
                    auto rep_sraw_source = seek_ZCHK_SRAW(mmap, offset);
                    it = mmap.begin() + offset;
                    auto rep_sraw_span = std::span(rep_sraw_source.begin(), rep_sraw_source.end());
                    auto sraw_info = decompress_span_zlib(rep_sraw_span);
                    // read out "Repeat Images" section

                    // Conditions for engaging a different readout mode are that the repeat length must be greater than 1.
                    // either 0 or 1 doesn't get acknowledged as a proper "repeat images" start.
                    auto repeat_mode = static_cast<SRAW_repeatimages_t>(read_4_b(sraw_info.begin() + 12));
                    std::size_t repeat_length = read_4_b(sraw_info.begin() + 16);
                    LOG("Repeat byte: " << static_cast<int>(repeat_mode) << " Repeat length: " << repeat_length)
                    
                    if (!repeat_images && repeat_length > 1) {
                        // switch to processing new repeat images section
                        repeat_images = true;
                        mode = repeat_mode;
                        repeat_images_length = repeat_length;
                        repeat_images_start_index = frames.size(); // doesn't have to be subtracted because we're pushing back later
                    }
                    else if (repeat_images && repeat_length > 1) {
                        // new image repeat section in same span of exposures/sraw_repeat frames
                        mode = repeat_mode;
                        repeat_images_length = repeat_length;
                        repeat_images_start_index = frames.size();
                    }
                    if (repeat_images) {
                        switch (mode) {
                        case SRAW_repeatimages_t::LOOP: {
                            // [exp] denotes an "exposure" frame, aka a SRAW_REPEAT without any repeat images.
                            // ---
                            // 0 1 2 3 [loop 4] [exp] [exp] [exp] [exp] [exp] [exp] ...
                            // 0 1 2 3 0        1     2     3     0     1     2     3
                            //
                            // You can "overshoot" the loop range as well, in which case the earliest frame "sticks"
                            // This behavior is buggy and can sometimes not show frames in software on playblack.
                            // When scrubbing through the timeline the software "clamps" to the "earliest" sampled frame.
                            // 0 1 2 3 [loop 6] [exp] [exp] [exp] [exp] [exp] [exp] [exp] [exp] [exp] ...
                            // 0 1 2 3 0        0     0     0     1     2     3     0     0     0     1
                            
                            // before insertion, frames.size() reflects the position the new frame is going to be at.
                            long int current_index = frames.size(); 
                            long int rel_index = current_index - repeat_images_start_index - repeat_images_start_index;
                            // repeat, accounting for overshooting and mimicking TVPain(t)s behavior to clamp to lowest index.
                            long int sample_index = ((rel_index % repeat_images_length)) - (repeat_images_length - repeat_images_start_index);
                            sample_index = limit_to_zero(sample_index);

                            auto sample_buffer = frames[sample_index].get();
                            Buffer_SRAW_Repeat::buffer_source source;
                            if (sample_buffer->index() == 0) {
                                source = &std::get<0>(*sample_buffer);
                            }
                            else if (sample_buffer->index() == 1) {
                                source = &std::get<1>(*sample_buffer);
                            }
                            else if (sample_buffer->index() == 2) {
                                auto repeat = std::get<2>(*sample_buffer);
                                if (!repeat.is_from_repeatimages){
                                    source = repeat.sraw_source;
                                }
                                else {
                                    // stick to last frame if origin of this frame was from a repeat images section.
                                    // This mirrors the behavior where in TVpaint, exposures are always looking towards the last frame. 
                                    source = last; 
                                }
                            }
                            // push back with info that this frame came from a Repeat Images section
                            frames.push_back(std::make_unique<buffer_var>(Buffer_SRAW_Repeat(source, true)));
                            break;
                        }
                        case SRAW_repeatimages_t::PINGPONG: {
                            // PING PONG LAST SET OF FRAMES.
                            // FOR EXAMPLE: START IDX = 8 ; LENGTH = 5
                            // 0 1 2 [3 4 5 6 7] start pingpong 6 5 4 3 4 5 6 7 6 5 4 3 4 5 6 7
                            long int current_index = frames.size();
                            long int pingpong_cycle_size = repeat_images_length * 2 - 2;
                            long int rel_index = current_index - repeat_images_start_index - pingpong_cycle_size + 1;

                            long int  pingpong_real_index = 0;

                            long int pingpong_cycle_index = rel_index % (pingpong_cycle_size);
                            
                            if (pingpong_cycle_index >= repeat_images_length) {
                                pingpong_real_index = pingpong_cycle_size - pingpong_cycle_index;
                            }
                            else {
                                pingpong_real_index = pingpong_cycle_index;
                            }

                            long int pingpong_idx = repeat_images_start_index - pingpong_real_index - 1;
                            pingpong_idx = limit_to_zero(pingpong_idx);

                            auto sample_buffer = frames[pingpong_idx].get();
                            Buffer_SRAW_Repeat::buffer_source source;
                            if (sample_buffer->index() == 0) {
                                source = &std::get<0>(*sample_buffer);
                            }
                            else if (sample_buffer->index() == 1) {
                                source = &std::get<1>(*sample_buffer);
                            }
                            else if (sample_buffer->index() == 2) {
                                auto repeat = std::get<2>(*sample_buffer);
                                if (!repeat.is_from_repeatimages) {
                                    source = repeat.sraw_source;
                                }
                                else {
                                    // stick to last frame if origin of this frame was from a repeat images section.
                                    // This mirrors the behavior where in TVpaint, exposures are always looking towards the last frame. 
                                    source = last;
                                }
                            }
                            // push back with info that this frame came from a Repeat Images section
                            frames.push_back(std::make_unique<buffer_var>(Buffer_SRAW_Repeat(source, true)));
                            break;
                        }
                        case SRAW_repeatimages_t::RANDOM: {
                            // I HAVE NO IDEA WHAT THE HELL IM SUPPOSED TO DO HERE, OKAY??
                            // I wouldn't want this behavior to be truly random because that could cause inconsistencies
                            // in distributed rendering.
                            // Maybe I'll generate some deterministic "seed" value from the position of the layer in the file,
                            // and feed it to the mersenne twister engine.
                            // this means deterministic "random" behavior at least...
                            // I have no guarrantee that the behavior will mirror TVPaint, though.
                            // 
                            // Thanks for listening,
                            // Sas van Gulik - 29 05 2025.
                            std::size_t upper_bound = repeat_images_start_index-1;
                            std::size_t lower_bound = limit_to_zero(repeat_images_start_index - repeat_images_length);

                            auto distributor = std::uniform_int_distribution(upper_bound, lower_bound);

                            std::size_t sample = distributor(twister);

                            auto sample_buffer = frames[sample].get();
                            Buffer_SRAW_Repeat::buffer_source source;
                            if (sample_buffer->index() == 0) {
                                source = &std::get<0>(*sample_buffer);
                            }
                            else if (sample_buffer->index() == 1) {
                                source = &std::get<1>(*sample_buffer);
                            }
                            else if (sample_buffer->index() == 2) {
                                auto repeat = std::get<2>(*sample_buffer);
                                if (!repeat.is_from_repeatimages) {
                                    source = repeat.sraw_source;
                                }
                                else {
                                    // stick to last frame if origin of this frame was from a repeat images section.
                                    // This mirrors the behavior where in TVpaint, exposures are always looking towards the last frame. 
                                    source = last;
                                }
                            }
                            // push back with info that this frame came from a Repeat Images section
                            frames.push_back(std::make_unique<buffer_var>(Buffer_SRAW_Repeat(source, true)));
                            break;
                        }
                        }
                        continue;
                    }
                    else {
                        // normal repeat.
                        frames.push_back(std::make_unique<buffer_var>(Buffer_SRAW_Repeat(last)));
                        //offset += 64; (is already moved by seek_ZCHK_SRAW)
                        continue;
                    }
                }
                else {
                    LOG("SRAW OG!");
                    auto sraw_source = seek_ZCHK_SRAW_VEC(mmap, offset);
                    frames_unique_idx.push_back(frames.size());
                    frames.push_back(std::make_unique<buffer_var>(Buffer_SRAW(fileinfo, sraw_source, shared_from_this(),frames_unique_idx.size()-1)));
                    last = &std::get<1>(*frames[frames.size() - 1].get());
                    it = mmap.begin() + offset;
                    // turn off repeat images mode.
                    repeat_images = false;
                    mode = SRAW_repeatimages_t::LOOP; // default
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

    auto framenr = frame_offset;

    auto pad = [](int long const& num, std::size_t len = 4) {
        auto str = std::to_string(std::abs(num));
        if (num < 0) {
            str.insert(str.begin(), '-');
        }
        while (str.length() != len) {
            str.insert(str.begin(), '0');
        }
        return str;
    };

    this->cache_layer_contents();
    std::size_t real_frame{};
    for (auto& frame : frames) {
        auto fullpath = path + std::format("{}_{}_{}.png", name_ascii.c_str(), prefix, pad(framenr));
        auto fullpathbin = path + std::format("{}_{}_{}.bin", name_ascii.c_str(), prefix, pad(framenr));
        auto ptr = frame.get();

        std::cout << "Writing out " << fullpath << "\n";

        //framebuf_raw_t fr{};
        //if (ptr->index() == 0) {
        //    auto& lyr = std::get<0>(*ptr);
        //    fr = lyr.get_framebuffer();
        //} else if (ptr->index() == 1) {
        //    auto& lyr = std::get<1>(*ptr);
        //    fr = lyr.get_framebuffer();
        //}
        //else if (ptr->index() == 2) {
        //    auto& lyr = std::get<2>(*ptr);
        //    fr = lyr.get_framebuffer();
        //}
        //std::cout << "writing it. " << fr.size() << "\n";
        //auto f = std::ofstream(fullpathbin.c_str(), std::ios::binary);
        //f.write(reinterpret_cast<char*>(fr.data()), fr.size());
        //f.close();
        try
        {
        framebuf_raw_t& fr = get_cache_at_frame(real_frame).value();
        stbi_write_png(fullpath.c_str(), file_info.width, file_info.height, 4, fr.data(), 4 * file_info.width);
        }
        catch (const std::bad_optional_access& e)
        {
            std::cout << e.what() << '\n';
        }
        framenr += 1;
        real_frame++;
    }
    this->clear_layer_contents();
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
cache_t& Layer::get_cache_at_frame(int long const& frame) {
    auto frames_amt = this->frames.size();
    
    if (frame < frame_offset) {
    // PRE-BEHAVIOR.
        switch (this->repeat_in_type) {
        case repeat_t::NONE: {
            return this->EMPTY_CACHE;
        }
        case repeat_t::REPEAT: {
            int long cur_frame_idx = frame - frame_offset;
            return in_range_cache(cur_frame_idx % frames.size());
        }
        case repeat_t::PINGPONG: {
            int long cur_frame_idx = frame - frame_offset;
            
            // early return optimization
            if (frames.size() == 1) {
                return in_range_cache(0);
            }
            else if (frames.size() == 2) {
                return in_range_cache(cur_frame_idx % frames.size());
            }
            // size 3 cycle 4    || size 4 cycle 6       || size 5 cycle 8
            // 1 2 3 [ 2 ] 1 2.. || 1 2 3 4 [3 2] 1 2 .. || 1 2 3 4 5 [4 3 2] 1 2..
            long int cycle_size = frames.size() * 2 - 2;
            auto cycle_index = cur_frame_idx % cycle_size;
            std::size_t real_idx;
            if (cycle_index >= frames.size()) {
                real_idx = cycle_size - cycle_index;
            }
            else {
                real_idx = cycle_index;
            }
            return in_range_cache(real_idx);
        }
        case repeat_t::HOLD: {
            return in_range_cache(0); // first frame
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
            int long cur_frame_idx = frame - frame_offset;
            return in_range_cache(cur_frame_idx % frames.size());
        }
        case repeat_t::PINGPONG: {
            int long cur_frame_idx = frame - frame_offset;

            if (frames.size() == 1) {
                return in_range_cache(frames.size()-1);
            } else if (frames.size() == 2) {
                return in_range_cache(cur_frame_idx % frames.size());
            }
            long int cycle_size = frames.size() * 2 - 2;  
            auto cycle_index = cur_frame_idx % cycle_size;
            std::size_t real_idx;
            if (cycle_index >= frames.size()) {
                real_idx = cycle_size - cycle_index;
            }
            else {
                real_idx = cycle_index;
            }
            return in_range_cache(real_idx);
        }
        case repeat_t::HOLD: {
            return in_range_cache(frames.size()-1); // last frame
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
