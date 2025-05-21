#include "Layer.hpp"
#include "../num_util.hpp"
#include <cmath>
#include <format>
#include <iostream>

Layer::Layer(std::span<std::uint8_t const>& layer_info) {
	auto read_4 = [](auto it) {
		return bigend_cast_from_ints<std::uint32_t>(*it, *(it + 1), *(it + 2), *(it + 3));
	};
	
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
    this->frame_offset = read_4(it+4);

    this->opacity_byte = *(LRHD_end + 20-1);
    this->opacity = std::round((static_cast<double>(this->opacity_byte)/255.0)*100);

    std::uint8_t layer_flags = *(LRHD_end + 32-1);
    this->unpack_layerflags(layer_flags);

    this->repeat_out_type = *(LRHD_end + 46-1);
    this->repeat_in_type = *(LRHD_end + 54-1);
    this->group_id = *(LRHD_end + 58-1);
}

// watch out with this, end of line ? into utf-8 bytes not handled properly. 
// I'm wondering if I can't just grab the LNAW and LNAM both.
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

void Layer::print_info() {
    auto s = std::format(R"(Layer:
Name: {}
Frame Offset: {}
Repeat In / Repeat Out: {} / {}
Group: {}

Opacity: {}%

Layer Flags:
Invisible: {}
Lighttable: {}
Stencil Mode: {}
Locked: {}
Position Locked: {}
Preserve Transparency: {}
)",
name, frame_offset, 
repeat_in_type, repeat_out_type, 
group_id, opacity,
invisible, lighttable, stencil, locked, position_locked, preserve_trans
);
    std::cout << s;
}