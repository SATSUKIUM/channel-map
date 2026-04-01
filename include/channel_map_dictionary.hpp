/*
This header file is included in the way below.
[in channel_map_dopeness.hpp]
namespace chmap {
    class ChannelMapDopeness {
        #include "channel_map_dictionary.hpp"
    ...
    } // class ChannelMapDopeness
} // namespace chmap
*/

#ifndef CHANNEL_MAP_DICTIONARY_HPP_
#define CHANNEL_MAP_DICTIONARY_HPP_

#include <cstdint>
#include <string>
#include <string_view>

uint8_t names();
uint8_t planes();
uint8_t readout_channels();

#endif // CHANNEL_MAP_DICTIONARY_HPP_