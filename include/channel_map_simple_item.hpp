#ifndef CHANNELMAP_CHANNEL_MAP_SIMPLE_ITEM_HPP_
#define CHANNELMAP_CHANNEL_MAP_SIMPLE_ITEM_HPP_

#include <vector>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include <algorithm>

// for std::tie
#include <tuple>
namespace chmap {
    class ChannelMapDopeness; // forward declaration to avoid circular dependency
    struct ChannelMapSimpleItem_FE {
        // zero initialized default constructor
        uint8_t ip3rd{}; // 192.168.ip3rd.ip4th
        uint8_t ip4th{};
        uint8_t ch{}; // input channel of StrReadout FEE
        ChannelMapSimpleItem_FE() = default; // default constructor
        ChannelMapSimpleItem_FE(uint8_t ip3rd_, uint8_t ip4th_, uint8_t ch_) :ip3rd(ip3rd_), ip4th(ip4th_), ch(ch_) {}
        public:
        void decode() const;
        bool operator<(const ChannelMapSimpleItem_FE& right) const {
            return std::tie(this->ip3rd, this->ip4th, this->ch) < std::tie(right.ip3rd, right.ip4th, right.ch);
        }
        bool operator==(const ChannelMapSimpleItem_FE& right) const {
            return std::tie(this->ip3rd, this->ip4th, this->ch) == std::tie(right.ip3rd, right.ip4th, right.ch);
        }
        uint32_t getRawID() const {
            return (static_cast<uint32_t>(ip3rd) << 16) | (static_cast<uint32_t>(ip4th) << 8) | static_cast<uint32_t>(ch);
        }
    };
    struct ChannelMapSimpleItem_DET {
        // zero initialized default constructor
        uint8_t name{};// indexed detector name(assume 0~63)
        uint8_t plane{};// indexed detector plane(assume 0~63)
        uint8_t segment{};// segment number(assume 0~255)
        uint16_t channel_number{};// channel number(assume 0~1023)
        uint8_t readout_channel{};// indexed readout channel name(assume 0~255)
        ChannelMapSimpleItem_DET() = default; // default constructor
        ChannelMapSimpleItem_DET(uint8_t name_idx, uint8_t plane_idx, uint8_t segment_, uint16_t channel_number_, uint8_t readout_channel_idx)
            : name(name_idx), plane(plane_idx), segment(segment_), channel_number(channel_number_), readout_channel(readout_channel_idx) {}
        public: 
        void decode() const;
        // define operator< for sorting and comparison
        bool operator<(const ChannelMapSimpleItem_DET& right) const {
            return std::tie(this->name, this->plane, this->segment, this->channel_number, this->readout_channel) < std::tie(right.name, right.plane, right.segment, right.channel_number, right.readout_channel);
        }
        bool operator==(const ChannelMapSimpleItem_DET& right) const {
            return std::tie(this->name, this->plane, this->segment, this->channel_number, this->readout_channel) == std::tie(right.name, right.plane, right.segment, right.channel_number, right.readout_channel);
        }
        uint32_t getRawID() const { // to be discontenued
            return (name << 24) | (plane << 8) | (segment << 0) | channel_number; // 使っちゃダメ。壊れます。
        }
    };
    struct ChannelMapSimpleItem {
        ChannelMapSimpleItem_FE fe;
        ChannelMapSimpleItem_DET det;
    };
}



#endif // CHANNELMAP_CHANNEL_MAP_SIMPLE_ITEM_HPP_