#include "chmap/channel_map_dopeness.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cctype>

#include <variant>

#define DEBUG_PRINT 0
#define DEBUG_PRINT_DUMMY_MAKER 0
#define DEBUG_PRINT_GETFERANK 0

namespace chmap {
    void ChannelMapDopeness::defineTokenNormalizationRules(){
        // 1st is original name, 2nd is simplified name(uint16_t)
        const std::vector<std::pair<std::string, std::string>> detplanes = {
            {"X", "X "},
            {"U", "U "},
            {"V", "V "},
            {"Xp", "XP"}, // for x prime
            {"Up", "UP"},
            {"Vp", "VP"},
            {"nil", "NI"}            
        };
        // make simplified map
        for(const auto& plane_pair : detplanes){
            uint16_t simplified = two_char_to_uint16(
                plane_pair.second[0],
                plane_pair.second[1]
            );
            token_normalization_2char[plane_pair.first] = simplified;
        }
    }// void ChannelMapDopeness::defineTokenNormalizationRules()

    uint16_t ChannelMapDopeness::two_char_to_uint16(char a, char b) {
        // 2つのcharをuint16_tに変換するルールを規定
        return (uint16_t(uint8_t(a)) << 8) | uint16_t(uint8_t(b));
    }// uint16_t ChannelMapDopeness::two_char_to_uint16

    bool ChannelMapDopeness::isTokenNumeric(const std::string& token) {
        // return true if token is numeric
        return !token.empty() && std::all_of(token.begin(), token.end(), ::isdigit);
    } // bool ChannelMapDopeness::isTokenNumeric

    uint16_t ChannelMapDopeness::parse_to16(const std::string& token) {
        if (isTokenNumeric(token)) {
            return static_cast<uint16_t>(std::stoul(token, nullptr, 0));
        } else {
            auto it = token_normalization_2char.find(token);// check in detname_simplify_map
            if(it == token_normalization_2char.end()) {
                std::cerr << "unknown token for uint16_t conversion: " << token << std::endl;
                std::exit(1);
            } else {
                uint16_t simplified = it->second;
                return simplified;
            }
        }
    }// uint16_t ChannelMapDopeness::parse_to16

    uint8_t ChannelMapDopeness::parse_to8(const std::string& token) {
        if (isTokenNumeric(token)) {
            return static_cast<uint8_t>(std::stoul(token, nullptr, 0));
        } else {
            std::cerr << "unknown token for uint8_t conversion: " << token << std::endl;
            std::exit(1);
        }
    }// uint8_t ChannelMapDopeness::parse_to8
}// namespace chmap