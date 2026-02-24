#ifndef CHANNELMAP_CHANNEL_MAP_SIMPLE_ITEM_HPP_
#define CHANNELMAP_CHANNEL_MAP_SIMPLE_ITEM_HPP_

#include <vector>
#include <cstdint>
namespace chmap {
    struct ChannelMapSimpleItem_FE {
        uint32_t id;
        // id = (ip3rd << 24) | (ip4th << 16) | channel で初期化。channelは最大FFFF
        ChannelMapSimpleItem_FE(uint8_t ip3rd, uint8_t ip4th, uint16_t ch) : id((uint32_t(ip3rd) << 24) | (uint32_t(ip4th) << 16) | uint32_t(ch) ) {}
    };
    struct ChannelMapSimpleItem_DET {
        uint32_t name;// detector name in 4 char
        uint16_t plane;// plane name in 2 char
        uint8_t segment;// segment number in 8bit int (0-255)
        uint32_t channel;// channel name in 4 char
        public: 
        void printContent();
    };
    struct ChannelMapSimpleItem {
        ChannelMapSimpleItem_FE fe;
        ChannelMapSimpleItem_DET det;
    };
}



#endif // CHANNELMAP_CHANNEL_MAP_SIMPLE_ITEM_HPP_