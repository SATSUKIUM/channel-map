#ifndef CHANNELMAP_CHANNEL_MAP_SIMPLE_ITEM_HPP_
#define CHANNELMAP_CHANNEL_MAP_SIMPLE_ITEM_HPP_

#include <vector>
#include <cstdint>
namespace chmap {
    struct ChannelMapSimpleItem_FE {
        uint32_t id;
        // id = (ip3rd << 16) | (ip4th << 8) | channel で初期化。それぞれ最大FF、つまりid = 0x00FFFFFFまで
        ChannelMapSimpleItem_FE(uint8_t ip3rd, uint8_t ip4th, uint8_t ch) : id((uint32_t(ip3rd) << 16) | (uint32_t(ip4th) << 8) | uint32_t(ch) ) {}
        public:
        void decode();
    };
    struct ChannelMapSimpleItem_DET {
        uint32_t name;// detector name in 4 char
        uint16_t plane;// plane name in 2 char
        uint8_t segment;// segment number in 8bit int (0-255)
        uint32_t channel;// channel name in 4 char
        public: 
        void decode();
    };
    struct ChannelMapSimpleItem {
        ChannelMapSimpleItem_FE fe;
        ChannelMapSimpleItem_DET det;
    };
}



#endif // CHANNELMAP_CHANNEL_MAP_SIMPLE_ITEM_HPP_