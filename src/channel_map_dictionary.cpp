#include "channel_map_dopeness.hpp"
#include "channel_map_simple_item.hpp"

#include <iomanip>
#include <algorithm>
#include <cctype>

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
    uint8_t queryIndex_name(const std::string& name){
        for(size_t i=0; i<name_dictionary.size(); ++i){
            if(name_dictionary[i] == name){
                return static_cast<uint8_t>(i);
            }
        }
        return 255; // not found
    }
    uint8_t queryIndex_plane(const std::string& plane){
        for(size_t i=0; i<plane_dictionary.size(); ++i){
            if(plane_dictionary[i] == plane){
                return static_cast<uint8_t>(i);
            }
        }
        return 255; // not found
    }
    uint8_t queryIndex_readout_channel(const std::string& readout_channel){
        for(size_t i=0; i<readout_channel_dictionary.size(); ++i){
            if(readout_channel_dictionary[i] == readout_channel){
                return static_cast<uint8_t>(i);
            }
        }
        return 255; // not found
    }
}
