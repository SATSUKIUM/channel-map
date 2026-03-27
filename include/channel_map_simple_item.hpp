#ifndef CHANNELMAP_CHANNEL_MAP_SIMPLE_ITEM_HPP_
#define CHANNELMAP_CHANNEL_MAP_SIMPLE_ITEM_HPP_

#include <vector>
#include <cstdint>
namespace chmap {
    struct ChannelMapSimpleItem_FE {
        // zero initialized default constructor
        uint8_t ip3rd{}; // 192.168.ip3rd.ip4th
        uint8_t ip4th{};
        uint8_t ch{}; // input channel of StrReadout FEE
        ChannelMapSimpleItem_FE(uint8_t ip3rd_, uint8_t ip4th_, uint8_t ch_) :ip3rd(ip3rd_), ip4th(ip4th_), ch(ch_) {}
        public:
        void decode();
        bool operator<(const ChannelMapSimpleItem_FE& right) const {
            return std::tie(this->ip3rd, this->ip4th, this->ch) < std::tie(right.ip3rd, right.ip4th, right.ch);
        }
    };
    struct ChannelMapSimpleItem_DET {
        // zero initialized default constructor
        uint32_t name{};// detector name in 4 char
        uint16_t plane{};// plane name in 2 char
        uint8_t segment{};// segment number in 8bit int (0-255)
        uint32_t channel{};// channel name in 4 char
        ChannelMapSimpleItem_DET(uint32_t name_, uint16_t plane_, uint8_t segment_, uint32_t channel_) : name(name_), plane(plane_), segment(segment_), channel(channel_) {}
        public: 
        void decode();
        // define operator< for sorting and comparison
        bool operator<(const ChannelMapSimpleItem_DET& right) const {
            return std::tie(this->name, this->plane, this->segment, this->channel) < std::tie(right.name, right.plane, right.segment, right.channel);
        }
    };
    struct ChannelMapSimpleItem {
        ChannelMapSimpleItem_FE fe;
        ChannelMapSimpleItem_DET det;
    };
}



#endif // CHANNELMAP_CHANNEL_MAP_SIMPLE_ITEM_HPP_