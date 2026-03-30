#include "channel_map_simple_item.hpp"
#include <iostream>
#include <iomanip>

namespace chmap {
    void ChannelMapSimpleItem_DET::decode() const {
        // print member variables
        std::cout << "\tDET name: 0x" << std::hex << std::setw(8) << std::setfill('0') << name;
        std::cout
            << " (in char: "
            << static_cast<char>((name >> 24) & 0xFF)
            << static_cast<char>((name >> 16) & 0xFF)
            << static_cast<char>((name >> 8) & 0xFF)
            << static_cast<char>(name & 0xFF)
            << ")," << std::endl;
        std::cout << "\tplane: 0x" << std::hex << std::setw(4) << std::setfill('0') << plane;
        std::cout
            << " (in char: "
            << static_cast<char>((plane >> 8) & 0xFF)
            << static_cast<char>(plane & 0xFF)
            << ")," << std::endl;
        std::cout << "\tsegment: " << static_cast<int>(segment) << std::endl;
        std::cout << "\tchannel: 0x" << std::hex << std::setw(8) << std::setfill('0');
        std::cout << channel
            << " (in char: "
            << static_cast<char>((channel >> 24) & 0xFF)
            << static_cast<char>((channel >> 16) & 0xFF)
            << static_cast<char>((channel >> 8) & 0xFF)
            << static_cast<char>(channel & 0xFF)
            << ")" << std::endl;
    } // void ChannelMapSimpleItem_DET::decode()

    void ChannelMapSimpleItem_FE::decode() const {
        std::cout << "\tFE id: 0x00" << std::hex << std::setw(2) << std::setfill('0') << ip3rd << std::setw(2) << std::setfill('0') << ip4th << std::setw(2) << std::setfill('0') << ch << std::dec << std::endl;
        std::cout
            << "\t\tFront-End IP address: 192.168."
            << std::setw(3) << std::setfill('0') << ((ip3rd) & 0xFF)
            << "."
            << std::setw(3) << std::setfill('0') << ((ip4th) & 0xFF)
            << ", channel: " << (ch & 0xFF) << std::endl;
    } // void ChannelMapSimpleItem_FE::decode()
} // namespace chmap