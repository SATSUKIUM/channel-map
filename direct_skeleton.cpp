#include "channel_map_direct.hpp"
#include "channel_map_simple_item.hpp"

// below for the original channel-map
// #include "channel_map.hpp"
// #include "debug_print.hpp"
// #include "channel_tuple.hpp"
// #include "element.hpp"

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
    chmap::ChannelMapDirect& channel_map_direct = chmap::ChannelMapDirect::get_instance();
    t0 = std::chrono::high_resolution_clock::now();
    channel_map_direct.initialize(input_file_path);
    t1 = std::chrono::high_resolution_clock::now();
    std::cout << "\n[in simple_skeleton.cpp] ChannelMapDirect initialized in " << std::chrono::duration<double, std::micro>(t1 - t0).count() << " microseconds." << std::endl;
    
    int i;
    std::cin >> i;

    // t0 = std::chrono::high_resolution_clock::now();
    // channel_map_direct.printAllItemsDET();
    // t1 = std::chrono::high_resolution_clock::now();
    // std::cout << "\n[in simple_skeleton.cpp] printAllItemsDET completed in " << std::chrono::duration<double, std::micro>(t1 - t0).count() << " microseconds." << std::endl;
    #if OF_BENCHMARK // file out, number of channels, time for search
    std::ofstream of_benchmark("benchmark_results.txt", std::ios::app);
    std::cout << "\n[in simple_skeleton.cpp] Benchmark of " << channel_map_simple.getNumberOfChannels() << " channels started." << std::endl;
    #endif

    
    // channel_map_direct.printAllItemsFE();
    // channel_map_direct.printAllItemsDET();

    #if PRINT_ALL_ITEMS_FE
    std::ofstream of_all_items_fe("all_items_fe.txt");
    for(const auto& item : channel_map_simple.fItemsFE) {
        of_all_items_fe << "FE id: 0x" << std::hex << std::setw(8) << std::setfill('0') << item.id << std::dec << std::endl;
    }
    of_all_items_fe.close();
    #endif

    // test t1 right channel
    uint8_t test_ip3rd_T1right = 0x02;
    uint8_t test_ip4th_T1right = 0xAA;
    uint16_t test_ch_T1right = 12;
    // test utof left channel
    uint8_t test_ip3rd_utof_left = 0x02;
    uint8_t test_ip4th_utof_left = 0xA9;
    uint16_t test_ch_utof_left = 8;
    // test bdc 1 V plane channel 4
    uint8_t test_ip3rd_bdc1 = 0x02;
    uint8_t test_ip4th_bdc1 = 0xA1;
    uint16_t test_ch_bdc1 = 32;
    // test kldc 2 U' plane channel 16
    uint8_t test_ip3rd_kldc2 = 0x02;
    uint8_t test_ip4th_kldc2 = 0xB2;
    uint16_t test_ch_kldc2 = 96;


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
        chmap::ChannelMapSimpleItem_DET* detitem = channel_map_direct.getDETItem(test_ip3rd_T1right, test_ip4th_T1right, test_ch_T1right);
        channel_map_direct.printDETinfo(*detitem);
        std::cout << std::string(80, '=') << std::endl;

    }
    #endif

    for(const auto& item : test_items) {
        uint8_t ip3rd = std::get<0>(item);
        uint8_t ip4th = std::get<1>(item);
        uint16_t ch = std::get<2>(item);
        const std::string& description = std::get<3>(item);
        std::cout << "\n[in simple_skeleton.cpp] Testing getDETItem for FE id of " << description << ":" << std::endl;
        channel_map_direct.printFEid(chmap::ChannelMapSimpleItem_FE(ip3rd, ip4th, ch));
        std::cout << "\t\tCorresponding DET info:" << std::endl;
        chmap::ChannelMapSimpleItem_DET* det_item = channel_map_direct.getDETItem(ip3rd, ip4th, ch);
        if(det_item != nullptr) {
            channel_map_direct.printDETinfo( *det_item );
            t0 = std::chrono::high_resolution_clock::now();
            for(int i=0; i<ntrials; i++) {
                chmap::ChannelMapSimpleItem_DET* det_item_inner = channel_map_direct.getDETItem(ip3rd, ip4th, ch);
                det_name = det_item_inner->name;
                det_plane = det_item_inner->plane;
                det_segment = det_item_inner->segment;
                det_channel = det_item_inner->channel;
            }
            t1 =  std::chrono::high_resolution_clock::now();
            for(int i=0; i<ntrials; i++) {
                chmap::ChannelMapSimpleItem_DET* det_item_inner;
            }
            t2 =  std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::micro> elapsed_subtract_overhead_loop = (t1 - t0) - (t2 - t1);
            std::cout << "\tDET name: " << std::hex << std::setw(8) << std::setfill('0') << det_name << std::dec
                      << ", plane: " << std::hex << std::setw(8) << std::setfill('0') << det_plane << std::dec
                      << ", segment: " << static_cast<uint8_t>(det_segment)
                      << ", channel: " << std::hex << std::setw(8) << std::setfill('0') << det_channel << std::dec
                      << std::endl;
            std::cout << "\n[in simple_skeleton.cpp] Performed " << ntrials << " trials of getDETItem from " << channel_map_direct.getNumberOfChannels() << " channels in " << elapsed_subtract_overhead_loop.count() << " microseconds." << " Overhead: " << std::chrono::duration<double , std::micro>(t2 - t1).count() << " microseconds." << std::endl;
            std::cout << "\tAverage time per getDETItem call: " << (elapsed_subtract_overhead_loop.count() / ntrials) << " microseconds." << std::endl;

            #if OF_BENCHMARK // file out
            of_benchmark << channel_map_direct.getNumberOfChannels() << " " << ntrials << " " << std::chrono::duration<double , std::micro>(t1 - t0).count() << " " << std::chrono::duration<double , std::micro>(t2 - t1).count() << " " <<  description <<  std::endl;
            #endif
        } else {
            std::cout << "\tDET item not found for the given FE id." << std::endl;
        }
    }

    #if general_chmap
    constexpr int n_trials = ntrials;
    t0 = std::chrono::high_resolution_clock::now();
    t1 =  std::chrono::high_resolution_clock::now();
    t2 =  std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::micro> elapsed_subtract_overhead_loop = (t1 - t0) - (t2 - t1);

    std::string detector_id, detector_channel;
    chmap::number_t detector_plane, detector_segment;
    t0 = std::chrono::high_resolution_clock::now();
    for(int i=0; i<n_trials; i++) {
        chmap::ChannelTuple det = channel_map.get("detector", fe1);
        detector_id = std::get<std::string>(det[0]);
        detector_plane = std::get<chmap::number_t>(det[1]);
        detector_segment = std::get<chmap::number_t>(det[2]);
        detector_channel = std::get<std::string>(det[3]);
    }
    t1 =  std::chrono::high_resolution_clock::now();
    for(int i=0; i<n_trials; i++) {
        chmap::ChannelTuple det;
    }
    t2 =  std::chrono::high_resolution_clock::now();

    elapsed_subtract_overhead_loop = (t1 - t0) - (t2 - t1);
    std::cout << "\n[in simple_skeleton.cpp] Performed " << n_trials << " trials of get detector (using general ChannelMap) in " << elapsed_subtract_overhead_loop.count() << " microseconds." << std::endl;
    std::cout << "\tAverage time per get detector call: " << (elapsed_subtract_overhead_loop.count() / n_trials) << " microseconds." << std::endl;
    #endif

    #if 0
    std::cout << "\n[in simple_skeleton.cpp] generating root file including all channel fe id" << std::endl;
    TFile* output_root_file = new TFile("all_items_after_dummy.root", "RECREATE");
    TTree* tree = new TTree("channel_map_simple_tree", "Tree containing all channel map simple items after dummy entry addition");
    uint32_t feid;
    tree->Branch("feid", &feid, "feid/i");
    uint32_t nentry = channel_map_simple.getNumberOfChannels();
    uint32_t entry_count = 0;
    for(const auto& item : channel_map_simple.fItemsFE) {
        feid = item.id;
        tree->Fill();
        entry_count++;
        if(entry_count % 10000 == 0) {
            std::cout << "\tProgress: " << (entry_count * 100) / nentry << "% (" << entry_count << " entries filled)" << std::endl;
        }
    }
    tree->Write();
    output_root_file->Close();
    #endif
    return 0;
}