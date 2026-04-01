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

#ifndef CHANNEL_MAP_DICTIONARY_PRIVATE_HPP_
#define CHANNEL_MAP_DICTIONARY_PRIVATE_HPP_

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace chmap::dictionary {
    uint8_t queryIndex_name(const std::string_view& name);
    uint8_t queryIndex_plane(const std::string_view& plane);
    uint8_t queryIndex_readout_channel(const std::string_view& RO_channel);

    inline constexpr std::array<std::string_view, 20> name_dictionary = {
        "UTOF", "DTOF", "LTOF", "T0  ", "T0RF", "T1  ", "ALCH", "BFTR", "BHT ", "BFT ", "SFT ", "BDC ", "KLDC", "LEFT", "RIGT", "TOP ", "BOTM", "UPST", "DOST", "NIL "
    };
/*
            {"utof", "UTOF"},
            {"dtof", "DTOF"},
            {"ltof", "LTOF"},
            {"t0", "T0  "},
            {"t0ref", "T0RF"},
            {"t1", "T1  "},
            {"all_charged", "ALCH"},
            {"bftref", "BFTR"},
            {"bht", "BHT "},
            {"bft", "BFT "},
            {"sft", "SFT "},
            {"bdc", "BDC "},
            {"kldc", "KLDC"},
            {"nil", "NIL "}
*/
    inline constexpr std::array<std::string_view, 7> plane_dictionary = {
        "X ", "U ", "V ", "XP", "UP", "VP", "NI"
    };
/*
            {"X", "X "},
            {"U", "U "},
            {"V", "V "},
            {"Xp", "XP"}, // for x prime
            {"Up", "UP"},
            {"Vp", "VP"},
            {"nil", "NI"} 
*/
    inline constexpr std::array<std::string_view, 7> readout_channel_dictionary = {"LEFT", "RIGT", "TOP ", "BOTM", "UPST", "DOST", "NI  "};
/*
            {"left", "LEFT"},
            {"right", "RIGT"},
            {"top", "TOP "},
            {"bottom", "BOTM"},
            {"upstream", "UPST"},
            {"downstream", "DOST"},
*/
} // namespace chmap::dictionary


#endif // CHANNEL_MAP_DICTIONARY_PRIVATE_HPP_