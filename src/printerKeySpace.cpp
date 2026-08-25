#include "chmap/channel_map_dopeness.hpp"
#include "chmap/item.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
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
namespace chmap {  
    void ChannelMapDopeness::printFEtoDETscan() {
        double fill_ratio = static_cast<double>(fItems.size()) / sizeSpace_FEKey;
        std::cout << "[ChannelMapDopeness::initialize] summary of FE key space scan" << std::endl;
        std::cout << "\tFE key space size: " << sizeSpace_FEKey << " = " << sizeSpace_ip3rd << " * " << sizeSpace_ip4th << " * " << sizeSpace_ch << std::endl;
        std::cout << "\t\t" << (double)sizeSpace_FEKey * 48 / (1024.0*1024.0) << " [MB]" << std::endl; // 48 comes from sizeof(ItemPair)
        std::cout << "\t\tsizeSpace_ip3rd: 0x" << std::setw(2) << std::hex << sizeSpace_ip3rd << " (" << std::setw(2) << std::setfill('0') << static_cast<uint32_t>(min_ip3rd) << " ~ " << std::setw(2) << std::setfill('0') << static_cast<uint32_t>(max_ip3rd) << ")" << std::endl;
        std::cout << "\t\tsizeSpace_ip4th: 0x" << std::setw(2) << std::hex << sizeSpace_ip4th << " (" << std::setw(2) << std::setfill('0') << static_cast<uint32_t>(min_ip4th) << " ~ " << std::setw(2) << std::setfill('0') << static_cast<uint32_t>(max_ip4th) << ")" << std::endl;
        std::cout << "\t\tsizeSpace_ch: 0x" << std::setw(2) << std::hex << sizeSpace_ch << " (" << std::setw(2) << std::setfill('0') << static_cast<uint32_t>(min_ch) << " ~ " << std::setw(2) << std::setfill('0') << static_cast<uint32_t>(max_ch) << ")" << std::endl;

        fill_ratio = static_cast<double>(fItems.size()) / sizeSpace_FEKey;
        std::cout << "\tFE ID range coverage: " << fill_ratio * 100.0 << " %" << std::endl;
    } // void ChannelMapDopeness::printFEtoDETscan()

    void ChannelMapDopeness::printDETtoFEscan(){
        std::cout << "[ChannelMapDopeness::initialize_InvMap] summary of DET key space scan:" << std::endl;
        std::cout << "\tDET key space size: " << sizeSpace_DETKey << " = " << sizeSpace_name_idx << " * " << sizeSpace_plane_idx << " * " << sizeSpace_segment << " * " << sizeSpace_channel_number << " * " << sizeSpace_readout_channel_idx << std::endl;
        std::cout << "\t\t" << (double)sizeSpace_DETKey * 24 / (1024.0*1024.0) << " [MB]" << std::endl; // 24 comes from sizeof(ItemPair)
        std::cout << "\t\tname_idx: 0x" << std::setw(2) << std::hex << sizeSpace_name_idx << " (" << std::setw(2) << std::setfill('0') << static_cast<uint32_t>(min_name_idx) << " ~ " << std::setw(2) << std::setfill('0') << static_cast<uint32_t>(max_name_idx) << ")" << std::endl;
        std::cout << "\t\tplane_idx: 0x" << std::setw(2) << std::hex << sizeSpace_plane_idx << " (" << std::setw(2) << std::setfill('0') << static_cast<uint32_t>(min_plane_idx) << " ~ " << std::setw(2) << std::setfill('0') << static_cast<uint32_t>(max_plane_idx) << ")" << std::endl;
        std::cout << "\t\tsegment: 0x" << std::setw(2) << std::hex << sizeSpace_segment << " (" << std::setw(2) << std::setfill('0') << static_cast<uint32_t>(min_segment) << " ~ " << std::setw(2) << std::setfill('0') << static_cast<uint32_t>(max_segment) << ")" << std::endl;
        std::cout << "\t\tchannel_number: 0x" << std::setw(4) << std::hex << sizeSpace_channel_number << " (" << std::setw(4) << std::setfill('0') << static_cast<uint32_t>(min_channel_number) << " ~ " << std::setw(4) << std::setfill('0') << static_cast<uint32_t>(max_channel_number) << ")" << std::endl;
        std::cout << "\t\treadout_channel_idx: 0x" << std::setw(2) << std::hex << sizeSpace_readout_channel_idx << " (" << std::setw(2) << std::setfill('0') << static_cast<uint32_t>(min_readout_channel_idx) << " ~ " << std::setw(2) << std::setfill('0') << static_cast<uint32_t>(max_readout_channel_idx) << ")" << std::endl;

        std::cout << "\tDET ID range coverage: " << (static_cast<double>(fItems.size()) / sizeSpace_DETKey) * 100.0 << " %" << std::endl;
    } // void ChannelMapDopeness::printDETtoFEscan()
}