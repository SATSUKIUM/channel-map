#include "chmap/item.hpp"
#include "chmap/channel_map_dopeness.hpp"
#include <iostream>
#include <iomanip>
#include <string>

namespace chmap {
    void DETIdItem::decode() const {
        std::string detname_str, plane_str, readout_channel_str;
        // print member variables
        std::cout << std::endl << "\tdetname index: 0x" << std::hex << std::setw(8) << std::setfill('0') << name;
        if(ChannelMapDopeness::get_instance().detname_dictionary.invIndex(name, detname_str)){
            std::cout << " (detname: " << detname_str << ")";
        }
        std::cout << std::endl;

        std::cout << "\tplane index: 0x" << std::hex << std::setw(8) << std::setfill('0') << plane;
        if(ChannelMapDopeness::get_instance().plane_dictionary.invIndex(plane, plane_str)){
            std::cout << " (plane: " << plane_str << ")";
        }
        std::cout << std::endl;

        std::cout << "\tsegment: " << static_cast<int>(segment) << std::endl;

        std::cout << "\tchannel name index: 0x" << std::hex << std::setw(8) << std::setfill('0') << readout_channel << std::dec;
        if(ChannelMapDopeness::get_instance().readout_channel_dictionary.invIndex(readout_channel, readout_channel_str)){
            std::cout << " (readout channel: " << readout_channel_str << ")";
        }
        std::cout << std::endl;

        std::cout << "\tchannel number: 0x" << std::hex << std::setw(16) << std::setfill('0') << channel_number << std::dec << std::endl;


    } // void DETIdItem::decode()

    void FEAddrItem::decode() const {
        std::cout << "\tFE id: 0x00" << std::hex << std::setw(2) << std::setfill('0') << static_cast<uint32_t>(ip3rd) << std::setw(2) << std::setfill('0') << static_cast<uint32_t>(ip4th) << std::setw(2) << std::setfill('0') << static_cast<uint32_t>(ch) << std::dec << std::endl;
        std::cout
            << "\t\tFront-End IP address: 192.168."
            << std::setw(3) << std::setfill('0') << ((ip3rd) & 0xFF)
            << "."
            << std::setw(3) << std::setfill('0') << ((ip4th) & 0xFF)
            << ", channel: " << (ch & 0xFF) << std::endl;
    } // void FEAddrItem::decode()
} // namespace chmap