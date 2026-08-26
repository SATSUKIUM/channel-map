#ifndef CHMAP_ITEM_HPP_
#define CHMAP_ITEM_HPP_

#include <vector>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include <algorithm>

// for std::tie
#include <tuple>

#include "chmap/detector_configurations.hpp"

namespace chmap {
    class ChannelMapDopeness; // forward declaration to avoid circular dependency

    struct FEAddrItem {
        uint8_t ip3rd{}; // 192.168.ip3rd.ip4th
        uint8_t ip4th{}; // 192.168.ip3rd.ip4th
        uint8_t ch{}; // input channel of StrReadout FEE
        FEAddrItem() = default; // default constructor
        FEAddrItem(uint8_t ip3rd_, uint8_t ip4th_, uint8_t ch_) :ip3rd(ip3rd_), ip4th(ip4th_), ch(ch_) {}
        public:
        void decode() const;
        bool operator<(const FEAddrItem& right) const {
            return std::tie(this->ip3rd, this->ip4th, this->ch) < std::tie(right.ip3rd, right.ip4th, right.ch);
        }
        bool operator==(const FEAddrItem& right) const {
            return std::tie(this->ip3rd, this->ip4th, this->ch) == std::tie(right.ip3rd, right.ip4th, right.ch);
        }
    }; // struct FEAddrItem

    struct DETIdItem {
        uint8_t name{};// indexed detector name(assume 0~63)
        uint8_t plane{};// indexed detector plane(assume 0~63)
        uint8_t segment{};// segment number(assume 0~255)
        uint16_t channel_number{};// channel number(assume 0~1023)
        uint8_t readout_channel{};// indexed readout channel name(assume 0~255)
        DETIdItem() = default; // default constructor
        DETIdItem(uint8_t name_idx, uint8_t plane_idx, uint8_t segment_, uint16_t channel_number_, uint8_t readout_channel_idx)
            : name(name_idx), plane(plane_idx), segment(segment_), channel_number(channel_number_), readout_channel(readout_channel_idx) {}
        public: 
        void decode() const;
        // define operator< for sorting and comparison
        bool operator<(const DETIdItem& right) const {
            return std::tie(this->name, this->plane, this->segment, this->channel_number, this->readout_channel) < std::tie(right.name, right.plane, right.segment, right.channel_number, right.readout_channel);
        }
        bool operator==(const DETIdItem& right) const {
            return std::tie(this->name, this->plane, this->segment, this->channel_number, this->readout_channel) == std::tie(right.name, right.plane, right.segment, right.channel_number, right.readout_channel);
        }

        DETConfItem* detconf = nullptr;
    }; // struct DETIdItem

    struct ItemPair {
        FEAddrItem fe;
        DETIdItem det;
    };
}


#endif // CHMAP_ITEM_HPP_