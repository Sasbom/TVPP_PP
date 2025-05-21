#include "Layer.hpp"
#include "../num_util.hpp"

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


}

void Layer::name_from_LNAM_LNAW(std::span<std::uint8_t const> const& lnam, std::span<std::uint8_t const> const& lnaw) {
    std::string collect{};
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
    name =  collect;
}