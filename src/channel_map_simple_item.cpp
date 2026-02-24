#include "channel_map_simple_item.hpp"
#include <iostream>
#include <iomanip>

namespace chmap {
    void ChannelMapSimpleItem_DET::decode(){
        // print member variables
        std::cout << "\tDET name: 0x" << std::setw(8) << std::setfill('0') << name;
        std::cout
            << " (in char: " << name
            << static_cast<char>((name >> 24) & 0xFF)
            << static_cast<char>((name >> 16) & 0xFF)
            << static_cast<char>((name >> 8) & 0xFF)
            << static_cast<char>(name & 0xFF)
            << ")," << std::endl;
        std::cout << "\tplane: 0x" << std::setw(4) << std::setfill('0') << plane;
        std::cout
            << " (in char: " << static_cast<uint16_t>(plane)
            << static_cast<char>((plane >> 8) & 0xFF)
            << static_cast<char>(plane & 0xFF)
            << ")," << std::endl;
        std::cout << "\tsegment: " << static_cast<uint8_t>(segment) << std::endl;
        std::cout << "\tchannel: 0x" << std::setw(8) << std::setfill('0');
        std::cout << channel
            << " (in char: "
            << static_cast<char>((channel >> 24) & 0xFF)
            << static_cast<char>((channel >> 16) & 0xFF)
            << static_cast<char>((channel >> 8) & 0xFF)
            << static_cast<char>(channel & 0xFF)
            << ")" << std::endl;
    } // void ChannelMapSimpleItem_DET::decode()

    void ChannelMapSimpleItem_FE::decode(){
        std::cout << "\tFE id: 0x" << std::hex << std::setw(8) << std::setfill('0') << id << std::dec << std::endl;
        std::cout
            << "\t\tFront-End IP address: 192.168."
            << std::setw(3) << std::setfill('0') << ((id >> 24) & 0xFF)
            << std::setw(3) << std::setfill('0') << ((id >> 16) & 0xFF)
            << ", channel: " << (id & 0xFFFF) << std::endl;
    } // void ChannelMapSimpleItem_FE::decode()
} // namespace chmap