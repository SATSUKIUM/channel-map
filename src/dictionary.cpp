#include "chmap/channel_map_dopeness.hpp"
#include "chmap/dictionary.hpp"
#include "chmap/item.hpp"

#include <iomanip>
#include <algorithm>
#include <cctype>
#include <stdexcept>

/*
dope-vectorの概念を用いたchannel-mapの実装
ユーザーの運用は下記のように限定させてもらう。bool getDopeKey_FEで各々ハンドリングしてもらう。
uint32_t doped_index;
if(!channel_map_dopeness.getDopeKey_FE(ip3rd, ip4th, ch, doped_index)){
    // handle out of range key
    continue;
}else{
    // usual process}
*/
namespace chmap::dictionary {
    void NameIndexDictionary::newWord(const std::string& str){
        if(std::find(names_string.begin(), names_string.end(), str) == names_string.end()){
            names_string.push_back(str);
        }
    }

    void NameIndexDictionary::sortWords(){
        std::sort(names_string.begin(), names_string.end());
    }

    void NameIndexDictionary::buildDictionary(bool reserveZero){
        zero_reserved = reserveZero;
        const size_t first = reserveZero ? 1 : 0;
        if (names_string.size() > 256 - first)
            throw std::runtime_error("Too many names for uint8_t dictionary");
        forward_dictionary.clear();
        inverse_dictionary.clear();
        if (reserveZero) inverse_dictionary.emplace_back();
        for(size_t i = 0; i < names_string.size(); ++i){
            forward_dictionary.emplace_back(names_string[i], static_cast<uint8_t>(i + first));
            inverse_dictionary.push_back(names_string[i]);
        }
    }

    bool NameIndexDictionary::IndexToString(uint8_t idx, std::string& str) const {
        if((!zero_reserved || idx != 0) && idx < inverse_dictionary.size()){
            str = inverse_dictionary[idx];
            return true;
        }
        return false;
    }

    bool NameIndexDictionary::StringToIndex(const std::string& str, uint8_t& idx) const {
        auto it = std::find_if(forward_dictionary.begin(), forward_dictionary.end(), [&str](const std::pair<std::string, uint8_t>& pair){
            return pair.first == str;
        });
        if(it != forward_dictionary.end()){
            idx = it->second;
            return true;
        }
        return false;
    }

    uint8_t queryIndex_name(const std::string_view& name){
        for(size_t i=0; i<name_dictionary.size(); ++i){
            if(name_dictionary[i] == name){
                return static_cast<uint8_t>(i);
            }
        }
        return 255; // not found
    }
    uint8_t queryIndex_plane(const std::string_view& plane){
        for(size_t i=0; i<plane_dictionary.size(); ++i){
            if(plane_dictionary[i] == plane){
                return static_cast<uint8_t>(i);
            }
        }
        return 255; // not found
    }
    uint8_t queryIndex_readout_channel(const std::string_view& readout_channel){
        for(size_t i=0; i<readout_channel_dictionary.size(); ++i){
            if(readout_channel_dictionary[i] == readout_channel){
                return static_cast<uint8_t>(i);
            }
        }
        return 255; // not found
    }
} // namespace chmap::dictionary