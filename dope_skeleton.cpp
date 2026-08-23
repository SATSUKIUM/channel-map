#include "chmap/channel_map_dopeness.hpp"
#include "chmap/item.hpp"

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
#define isInvMap 1
#define ntrials 1000000

/*
mapdata.csvのファイルパスを与えるとchannel-map-simpleの動作テストをする
*/
int main(int argc, char* argv[]) {
    auto t0 = std::chrono::high_resolution_clock::now();
    auto t1 = std::chrono::high_resolution_clock::now();
    auto t2 = std::chrono::high_resolution_clock::now();
    auto t0_inv = std::chrono::high_resolution_clock::now();
    auto t1_inv = std::chrono::high_resolution_clock::now();
    auto t2_inv = std::chrono::high_resolution_clock::now();
    bool isCreateInvMap = true;

    std::string input_file_path = argv[1];
    chmap::ChannelMapDopeness& channel_map_dopeness = chmap::ChannelMapDopeness::get_instance();
    t0 = std::chrono::high_resolution_clock::now();
    #if isInvMap
    isCreateInvMap = true;
    #endif
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
    uint8_t det_name_idx_inner, det_plane_idx_inner, det_segment_inner, det_readout_channel_idx_inner;
    uint16_t det_channel_number_inner;
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
        // t1について動作確認
        uint32_t dopeKey_FEtoDET;
        if(!channel_map_dopeness.getDopeKey_FEtoDET(test_ip3rd_T1right, test_ip4th_T1right, test_ch_T1right, dopeKey_FEtoDET)) {
            std::cout << "FE id for T1 right channel is out of range in getDopeKey_FEtoDET()." << std::endl;
            return 1;
        }
        const chmap::DETIdItem& detitem = channel_map_dopeness.getDETIdItem(dopeKey_FEtoDET);
        detitem.decode();

        // test for index to string conversion
        std::string det_name_str;
        std::string det_plane_str;
        std::string det_readout_channel_str;
        if(!channel_map_dopeness.detname_dictionary.invIndex(detitem.name, det_name_str)) {
            std::cout << "det name index could not be resolved to string." << std::endl;
            return 1;
        }
        std::cout << "(test) det name string: " << det_name_str << " got from index: " << static_cast<int>(detitem.name) << std::endl;
        if(!channel_map_dopeness.plane_dictionary.invIndex(detitem.plane, det_plane_str)) {
            std::cout << "det plane index could not be resolved to string." << std::endl;
            return 1;
        }
        std::cout << "(test) det plane string: " << det_plane_str << " got from index: " << static_cast<int>(detitem.plane) << std::endl;
        if(!channel_map_dopeness.readout_channel_dictionary.invIndex(detitem.readout_channel, det_readout_channel_str)) {
            std::cout << "readout channel index could not be resolved to string." << std::endl;
            return 1;
        }
        std::cout << "(test) det readout channel string: " << det_readout_channel_str << " got from index: " << static_cast<int>(detitem.readout_channel) << std::endl;

        // test for registering DETConfItem
        chmap::DETConfItem demo_detconf;
        if(!channel_map_dopeness.registerDETConfItem(
                det_name_str,
                det_plane_str,
                detitem.segment,
                det_readout_channel_str,
                detitem.channel_number,
                &demo_detconf)) {
            std::cout << "failed to register DETConfItem." << std::endl;
            return 1;
        }
        if(channel_map_dopeness.getDETIdItem(dopeKey_FEtoDET).detconf != &demo_detconf) {
            std::cout << "DETConfItem pointer was not stored in the dope vector." << std::endl;
            return 1;
        }
        std::cout << "registered DETConfItem pointer for " << det_name_str << ", " << det_plane_str << ", segment " << static_cast<int>(detitem.segment) << ", channel " << detitem.channel_number << std::endl;

        std::unique_ptr<chmap::GeomItemDC> geom_dc = std::make_unique<chmap::GeomItemDC>();
        geom_dc->SetGlobalPosition(0.0, 0.0, 100.0);
        geom_dc->SetResolution(0.1, 0.1, 0.1);
        geom_dc->SetRotationAngles(0.0, 0.0, 0.0);
        if(!channel_map_dopeness.registerDETConfSubItem<chmap::GeomItem, chmap::GeomItemDC>(dopeKey_FEtoDET, std::move(geom_dc), &chmap::DETConfItem::membername_geom)) {
            std::cout << "failed to register GeomItemDC as a subitem of DETConfItem." << std::endl;
            return 1;
        }
        std::cout << "registered GeomItemDC as a subitem of DETConfItem for " << det_name_str << ", " << det_plane_str << ", segment " << static_cast<int>(detitem.segment) << ", channel " << detitem.channel_number << std::endl;
        std::cout << "retrieved GeomItemDC from DETConfItem: " << std::endl;
        const chmap::GeomItemDC* retrieved_geom_dc = dynamic_cast<const chmap::GeomItemDC*>(channel_map_dopeness.getDETIdItem(dopeKey_FEtoDET).detconf->membername_geom.get());
        if(retrieved_geom_dc) {
            std::cout << "retrieved GeomItemDC successfully." << std::endl;
        } else {
            std::cout << "failed to retrieve GeomItemDC." << std::endl;
        }
        std::cout << std::string(80, '=') << std::endl;
    }
    #endif

    {
        // test for KLDC
        std::cout << std::string(80, '=') << std::endl;
        std::cout << "[in dope_skeleton.cpp] Testing getDETItem for FE id of KLDC 2 U' plane channel 16:" << std::endl;
/*
    // test kldc 2 U' plane channel 16
    uint8_t test_ip3rd_kldc2 = 0x02;
    uint8_t test_ip4th_kldc2 = 0xB2;
    uint8_t test_ch_kldc2 = 96;
*/
        uint32_t dopeKey_FEtoDET;
        if(!channel_map_dopeness.getDopeKey_FEtoDET(test_ip3rd_kldc2, test_ip4th_kldc2, test_ch_kldc2, dopeKey_FEtoDET)) {
            std::cout << "FE id for KLDC 2 U' plane channel 16 is out of range in getDopeKey_FEtoDET()." << std::endl;
            return 1;
        }
        const chmap::DETIdItem& detitem = channel_map_dopeness.getDETIdItem(dopeKey_FEtoDET);
        detitem.decode();
        uint8_t det_name_idx = detitem.name;
        uint8_t det_plane_idx = detitem.plane;
        uint8_t det_segment = detitem.segment;
        uint8_t det_readout_channel_idx = detitem.readout_channel;
        uint16_t det_channel_number = detitem.channel_number;
        std::string det_name_str, det_plane_str, det_readout_channel_str;
        if(!channel_map_dopeness.detname_dictionary.invIndex(det_name_idx, det_name_str)) {
            std::cout << "det name index could not be resolved to string." << std::endl;
            return 1;
        }
        else{
            std::cout << "det name string: \"" << det_name_str << "\" got from index: \"" << static_cast<int>(det_name_idx) << "\"" << std::endl;
            if(std::string("kldc") == det_name_str) {
                std::cout << "det name string matches expected value \"kldc\"." << std::endl;
            } else {
                std::cout << "det name string does not match expected value \"kldc\"." << std::endl;
            }
        }
        if(!channel_map_dopeness.plane_dictionary.invIndex(det_plane_idx, det_plane_str)) {
            std::cout << "det plane index could not be resolved to string." << std::endl;
            return 1;
        }
        else{
            std::cout << "det plane string: \"" << det_plane_str << "\" got from index: \"" << static_cast<int>(det_plane_idx) << "\"" << std::endl;
            if(std::string("U") == det_plane_str) {
                std::cout << "det plane string matches expected value \"U\"." << std::endl;
            } else {
                std::cout << "det plane string does not match expected value \"U\"." << std::endl;
            }
        }
        if(!channel_map_dopeness.readout_channel_dictionary.invIndex(det_readout_channel_idx, det_readout_channel_str)) {
            std::cout << "readout channel index could not be resolved to string." << std::endl;
            return 1;
        }
        else{
            std::cout << "det readout channel string: \"" << det_readout_channel_str << "\" got from index: \"" << static_cast<int>(det_readout_channel_idx) << "\"" << std::endl;
            if(std::string("0") == det_readout_channel_str) {
                std::cout << "det readout channel string matches expected value \"0\"." << std::endl;
            } else {
                std::cout << "det readout channel string does not match expected value \"0\"." << std::endl;
            }
        }

        std::cout << std::string(80, '=') << std::endl;
    }

    for(const auto& item : test_items) {
        uint8_t ip3rd = std::get<0>(item);
        uint8_t ip4th = std::get<1>(item);
        uint16_t ch = std::get<2>(item);
        const std::string& description = std::get<3>(item);
        std::cout << "\n[in dope_skeleton.cpp] Testing getDETItem for FE id of " << description << ":" << std::endl;
        chmap::FEAddrItem(ip3rd, ip4th, ch).decode();
        std::cout << "\t\t(↓Corresponding DET info)" << std::endl;
        uint32_t dopeKey_FEtoDET;
        if(!channel_map_dopeness.getDopeKey_FEtoDET(ip3rd, ip4th, ch, dopeKey_FEtoDET)) {
            std::cout << "\tFE id is out of range in getDopeKey_FEtoDET()." << std::endl;
            continue;
        }
        chmap::DETIdItem det_item = channel_map_dopeness.getDETIdItem(dopeKey_FEtoDET);
        #if isInvMap
        std::cout << "query: " << static_cast<uint32_t>(ip3rd) << ", " << static_cast<uint32_t>(ip4th) << ", " << static_cast<uint32_t>(ch) << std::endl;
        #endif

        #if isInvMap
        std::cout << std::string(80, '=') << std::endl;
        std::cout << "\tChecking inverse mapping (DET -> FE)" << std::endl;
        det_item.decode();
        if(isCreateInvMap){
            uint32_t dopeKey_DETtoFE;
            if(!channel_map_dopeness.getDopeKey_DETtoFE(det_item.name, det_item.plane, det_item.segment, det_item.readout_channel, det_item.channel_number, dopeKey_DETtoFE)) {
                std::cout << "\tDET info is out of range in getDopeKey_DETtoFE(). This should not happen since it was obtained from a valid doped_index." << std::endl;
                continue;
            }
            chmap::FEAddrItem fe_item_inv = channel_map_dopeness.getFEAddrItem(dopeKey_DETtoFE);
            fe_item_inv.decode();
        }
        std::cout << std::string(80, '=') << std::endl;
        #endif


        if(!channel_map_dopeness.getDopeKey_FEtoDET(ip3rd, ip4th, ch, dopeKey_FEtoDET)) {
            std::cout << "\tFE id is out of range in getDopeKey_FEtoDET(). This should not happen since it was checked before." << std::endl;
            continue;
        }
        chmap::DETIdItem det_item_outer = channel_map_dopeness.getDETIdItem(dopeKey_FEtoDET);
        if(!channel_map_dopeness.getDopeKey_DETtoFE(det_item_outer, dopeKey_FEtoDET)){
            std::cout << "\tDET info is out of range in getDopeKey_DETtoFE(). This should not happen since it was obtained from a valid dopeKey_FEtoDET." << std::endl;
            continue;
        }
        chmap::FEAddrItem fe_item_outer = channel_map_dopeness.getFEAddrItem(dopeKey_FEtoDET);
        uint8_t det_name, det_plane,det_segment, det_readout_channel;
        uint16_t det_channel_number;
        std::string det_name_str, det_plane_str, det_readout_channel_str;
        t0 = std::chrono::high_resolution_clock::now();
        for(int i=0; i<ntrials; i++) {
            if(!channel_map_dopeness.getDopeKey_FEtoDET(ip3rd, ip4th, ch, dopeKey_FEtoDET)){
                std::cout << "\tFE id is out of range in getDopeKey_FEtoDET() in the loop. This should not happen since it was checked before the loop." << std::endl;
                break;
            }
            chmap::DETIdItem det_item_inner = channel_map_dopeness.getDETIdItem(dopeKey_FEtoDET);
            det_name = det_item_inner.name;
            det_plane = det_item_inner.plane;
            det_segment = det_item_inner.segment;
            det_readout_channel = det_item_inner.readout_channel;
            det_channel_number = det_item_inner.channel_number;
        }
        t1 =  std::chrono::high_resolution_clock::now();
        for(int i=0; i<ntrials; i++) {
            chmap::DETIdItem det_item_inner;
            if(1);
        }
        t2 =  std::chrono::high_resolution_clock::now();
        
        det_name_idx_inner = det_item.name;
        det_plane_idx_inner = det_item.plane;
        det_segment_inner = det_item.segment;
        det_channel_number_inner = det_item.channel_number;
        det_readout_channel_idx_inner = det_item.readout_channel;
        t0_inv = std::chrono::high_resolution_clock::now();
        for(int i=0; i<ntrials; i++) {
            if(!channel_map_dopeness.getDopeKey_DETtoFE(det_name_idx_inner, det_plane_idx_inner, det_segment_inner, det_readout_channel_idx_inner, det_channel_number_inner, dopeKey_FEtoDET)){
                std::cout << "\tDET info is out of range in getDopeKey_DETtoFE() in the loop. This should not happen since it was checked before the loop." << std::endl;
                break;
            }
            chmap::FEAddrItem fe_item_inner = channel_map_dopeness.getFEAddrItem(dopeKey_FEtoDET);

        }
        t1_inv = std::chrono::high_resolution_clock::now();
        for(int i=0; i<ntrials; i++) {
            chmap::FEAddrItem fe_item_inner;
            if(1);
        }
        t2_inv = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double, std::micro> elapsed_subtract_overhead_loop = (t1 - t0) - (t2 - t1);
        det_item_outer.decode();
        fe_item_outer.decode();
        
        
        std::cout << "\n[in dope_skeleton.cpp] Performed " << ntrials << " trials of getDETItem from " << channel_map_dopeness.getNumberOfChannels() << " channels in " << elapsed_subtract_overhead_loop.count() << " microseconds." << " Overhead: " << std::chrono::duration<double , std::micro>(t2 - t1).count() << " microseconds." << std::endl;
        std::cout << "\tAverage time per getDETItem call: " << (elapsed_subtract_overhead_loop.count() / ntrials) << " microseconds." << std::endl;

        std::chrono::duration<double, std::micro> elapsed_subtract_overhead_loop_inv = (t1_inv - t0_inv) - (t2_inv - t1_inv);
        std::cout << "\n[in dope_skeleton.cpp] Performed " << ntrials << " trials of getFEIItem from " << channel_map_dopeness.getNumberOfChannels() << " channels in " << elapsed_subtract_overhead_loop_inv.count() << " microseconds." << " Overhead: " << std::chrono::duration<double , std::micro>(t2_inv - t1_inv).count() << " microseconds." << std::endl;
        std::cout << "\tAverage time per getFEIItem call: " << (elapsed_subtract_overhead_loop_inv.count() / ntrials) << " microseconds." << std::endl;

        #if OF_BENCHMARK // file out
        of_benchmark << channel_map_dopeness.getNumberOfChannels() << " " << ntrials << " " << std::chrono::duration<double , std::micro>(t1 - t0).count() << " " << std::chrono::duration<double , std::micro>(t2 - t1).count() << " " <<  description <<  std::endl;
        #endif

    }

    return 0;
}