#include "channel_map_dopeness.hpp"
#include "channel_map_simple_item.hpp"

// handle string, and so on.
#include <string>
#include <iostream>
#include <iomanip>

// file, and so on.
#include <cstdlib>
#include <variant>

// stopwatch
#include <chrono>

// file out
#include <filesystem>
#include <fstream>

// static analysis
// #include <TFile.h>
// #include <TTree.h>

#include <unistd.h>

#define general_chmap 0
#define OF_BENCHMARK 0
#define CHECK_DUPLICATE_FE_ID 0
#define PRINT_ALL_ITEMS_FE 0
#define DUMMY 0
#define ntrials 1000000

/*
mapdata.csvのファイルパスを与えるとchannel-map-simpleの動作テストをする
*/
int main(int argc, char* argv[]) {
    auto t0 = std::chrono::high_resolution_clock::now();
    auto t1 = std::chrono::high_resolution_clock::now();
    auto t2 = std::chrono::high_resolution_clock::now();

    std::string input_file_path = argv[1];
    chmap::ChannelMapDopeness& channel_map_dopeness = chmap::ChannelMapDopeness::get_instance();
    t0 = std::chrono::high_resolution_clock::now();
    bool isCreateInvMap = true;;
    channel_map_dopeness.initialize(input_file_path, isCreateInvMap); // initialize()の第2引数は、逆引きマップを作るかどうか
    t1 = std::chrono::high_resolution_clock::now();
    std::cout << "\n[in dope_skeleton.cpp] ChannelMapDopeness initialized in " << std::chrono::duration<double, std::micro>(t1 - t0).count() << " microseconds." << std::endl;


    // test t1 right channel
    uint8_t test_ip3rd_T1right = 0x02;
    uint8_t test_ip4th_T1right = 0xAA;
    uint8_t test_ch_T1right = 12;
    // test utof left channel
    uint8_t test_ip3rd_utof_left = 0x02;
    uint8_t test_ip4th_utof_left = 0xA9;
    uint8_t test_ch_utof_left = 8;
    // test bdc 1 V plane channel 4
    uint8_t test_ip3rd_bdc1 = 0x02;
    uint8_t test_ip4th_bdc1 = 0xA1;
    uint8_t test_ch_bdc1 = 32;
    // test kldc 2 U' plane channel 16
    uint8_t test_ip3rd_kldc2 = 0x02;
    uint8_t test_ip4th_kldc2 = 0xB2;
    uint8_t test_ch_kldc2 = 96;

    uint32_t det_name;
    uint16_t det_plane;
    uint8_t det_segment;
    uint32_t det_channel;

    std::vector<std::tuple<uint8_t, uint8_t, uint16_t, std::string>> test_items = {
        {test_ip3rd_T1right, test_ip4th_T1right, test_ch_T1right, "T1_right_channel"},
        {test_ip3rd_utof_left, test_ip4th_utof_left, test_ch_utof_left, "utof_left_channel"},
        {test_ip3rd_bdc1, test_ip4th_bdc1, test_ch_bdc1, "bdc_1_V_plane_channel_4"},
        {test_ip3rd_kldc2, test_ip4th_kldc2, test_ch_kldc2, "kldc_2_U'_plane_channel_16"},
        {0xFF, 0xFF, 0xFFFF, "non-existing_channel"}
    };

    #if 1
    {
        std::cout << std::string(80, '=') << std::endl;
        uint32_t doped_index;
        if(!channel_map_dopeness.getDopeKey_FEtoDET(test_ip3rd_T1right, test_ip4th_T1right, test_ch_T1right, doped_index)) {
            std::cout << "FE id for T1 right channel is out of range in getDopeKey_FEtoDET()." << std::endl;
            return 1;
        }
        chmap::ChannelMapSimpleItem_DET detitem = channel_map_dopeness.getDETItem(doped_index);
        detitem.decode();
        std::cout << std::string(80, '=') << std::endl;

    }
    #endif

    for(const auto& item : test_items) {
        uint8_t ip3rd = std::get<0>(item);
        uint8_t ip4th = std::get<1>(item);
        uint16_t ch = std::get<2>(item);
        const std::string& description = std::get<3>(item);
        std::cout << "\n[in dope_skeleton.cpp] Testing getDETItem for FE id of " << description << ":" << std::endl;
        channel_map_dopeness.printFEid(chmap::ChannelMapSimpleItem_FE(ip3rd, ip4th, ch));
        std::cout << "\t\t(↓Corresponding DET info)" << std::endl;
        uint32_t doped_index;
        if(!channel_map_dopeness.getDopeKey_FEtoDET(ip3rd, ip4th, ch, doped_index)) {
            std::cout << "\tFE id is out of range in getDopeKey_FEtoDET()." << std::endl;
            continue;
        }
        chmap::ChannelMapSimpleItem_DET det_item = channel_map_dopeness.getDETItem(doped_index);
        std::cout << "query: " << static_cast<uint32_t>(ip3rd) << ", " << static_cast<uint32_t>(ip4th) << ", " << static_cast<uint32_t>(ch) << std::endl;

        if(isCreateInvMap){
            uint32_t rank_inv;
            if(!channel_map_dopeness.getRank_DETtoFE(det_item.name, det_item.plane, det_item.segment, det_item.channel, rank_inv)) {
                std::cout << "\tDET info is out of range in getRank_DETtoFE(). This should not happen since it was obtained from a valid doped_index." << std::endl;
                continue;
            }
            chmap::ChannelMapSimpleItem_FE fe_item_inv = channel_map_dopeness.getFEIItem(rank_inv);
            fe_item_inv.decode();
        }


        t0 = std::chrono::high_resolution_clock::now();
        for(int i=0; i<ntrials; i++) {
            #if 1
            if(!channel_map_dopeness.getDopeKey_FEtoDET(ip3rd, ip4th, ch, doped_index)){
                std::cout << "\tFE id is out of range in getDopeKey_FEtoDET() in the loop. This should not happen since it was checked before the loop." << std::endl;
                break;
            }
            chmap::ChannelMapSimpleItem_DET det_item_inner = channel_map_dopeness.getDETItem(doped_index);
            #endif
            #if 0
            doped_index = channel_map_dopeness.unchecked_getDopeKey_FE(ip3rd, ip4th, ch);
            chmap::ChannelMapSimpleItem_DET det_item_inner = channel_map_dopeness.getDETItem(doped_index);
            #endif
            det_name = det_item_inner.name;
            det_plane = det_item_inner.plane;
            det_segment = det_item_inner.segment;
            det_channel = det_item_inner.channel;
        }
        t1 =  std::chrono::high_resolution_clock::now();
        for(int i=0; i<ntrials; i++) {
            chmap::ChannelMapSimpleItem_DET det_item_inner;
        }
        t2 =  std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::micro> elapsed_subtract_overhead_loop = (t1 - t0) - (t2 - t1);
        std::cout << "\tDET name: " << std::hex << std::setw(8) << std::setfill('0') << det_name << std::dec
                    << ", plane: " << std::hex << std::setw(8) << std::setfill('0') << det_plane << std::dec
                    << ", segment: " << static_cast<uint8_t>(det_segment)
                    << ", channel: " << std::hex << std::setw(8) << std::setfill('0') << det_channel << std::dec
                    << std::endl;
        std::cout << "\n[in dope_skeleton.cpp] Performed " << ntrials << " trials of getDETItem from " << channel_map_dopeness.getNumberOfChannels() << " channels in " << elapsed_subtract_overhead_loop.count() << " microseconds." << " Overhead: " << std::chrono::duration<double , std::micro>(t2 - t1).count() << " microseconds." << std::endl;
        std::cout << "\tAverage time per getDETItem call: " << (elapsed_subtract_overhead_loop.count() / ntrials) << " microseconds." << std::endl;

        #if OF_BENCHMARK // file out
        of_benchmark << channel_map_dopeness.getNumberOfChannels() << " " << ntrials << " " << std::chrono::duration<double , std::micro>(t1 - t0).count() << " " << std::chrono::duration<double , std::micro>(t2 - t1).count() << " " <<  description <<  std::endl;
        #endif

    }

    return 0;
}