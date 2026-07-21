#include "chmap/channel_map_dopeness.hpp"
#include "chmap/item.hpp"
#include "chmap/channel_tuple.hpp"
// #include "chmap/element.hpp"

#include "debugger.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cctype>

#include <variant>

// for std::tie
#include <tuple>

#define DEBUG_PRINT 0
#define DEBUG_PRINT_DUMMY_MAKER 0
#define DEBUG_PRINT_GETFERANK 0

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
/*
関連ソースコード
    - src/channel_map_simple_item.cpp : decode()の実装
    - src/channel_map_simple_rules.cpp : 辞書などのルールの定義
    - src/channel_map_readCSV.cpp : CSVの読み込みとfItemsの初期化
    - include/channel_map_simple_item.hpp
    - include/channel_map_dopeness.hpp
*/
namespace chmap {    
    ChannelMapDopeness& ChannelMapDopeness::get_instance() {
        static ChannelMapDopeness instance;
        return instance;
    }// ChannelMapDopeness& ChannelMapDopeness::get_instance()
    ChannelMapDopeness::~ChannelMapDopeness() {
        // destructor
    }// ChannelMapDopeness::~ChannelMapDopeness()
    
    double ChannelMapDopeness::initialize(const std::string& file_path, bool createInvMap) {
        double fill_ratio;

        defineDictionary(); // prepare detname_simplify_map
        #if DEBUG_PRINT
        std::cout << "str simplify map32:" << std::endl;
        for(const auto& pair : mapdata_string_simplify_map32){
            std::cout << "  " << pair.first << " -> " << std::hex << pair.second << std::dec << std::endl;
        }
        std::cout << "str simplify map16:" << std::endl;
        for(const auto& pair : mapdata_string_simplify_map16){
            std::cout << "  " << pair.first << " -> " << std::hex << pair.second << std::dec << std::endl;
        }
        #endif

        readCSV(file_path); // fill fItems
        #if CHECK_INITIALIZATION
        std::cout << "[ChannelMapDopeness::initialize] readCSV finished, number of items: " << fItems.size() << std::endl;
        #endif

        #if DEBUG_PRINT
        std::cout << "start sorting fItems by fe.id" << std::endl;
        #endif
        // sort fItems by fe.id
        std::sort(fItems.begin(), fItems.end(), [](const ChannelMapSimpleItem& left, const ChannelMapSimpleItem& right) {
            return left.fe < right.fe;
        });
        #if DEBUG_PRINT
        std::cout << "finished sorting fItems" << std::endl;
        #endif

        std::cout << "[ChannelMapDopeness::initialize] dope vector initialize start" << std::endl;
        // どこからどこまで空間を作るかスキャン
        min_ip3rd = 0xFF; // used in getDopeKey_FE()
        min_ip4th = 0xFF; // used in getDopeKey_FE()
        min_ch = 0xFF;    // used in getDopeKey_FE()
        max_ip3rd = 0;    // used in getDopeKey_FE()
        max_ip4th = 0;    // used in getDopeKey_FE()
        max_ch = 0;       // used in getDopeKey_FE()
        uint8_t buf;
        for(const auto& item : fItems){
            auto fe = item.fe;
            buf = (fe.ip3rd) & 0xFF; // ip3rd
            if(buf < min_ip3rd) min_ip3rd = buf;
            if(buf > max_ip3rd) max_ip3rd = buf;
            buf = (fe.ip4th) & 0xFF; // ip4th
            if(buf < min_ip4th) min_ip4th = buf;
            if(buf > max_ip4th) max_ip4th = buf;
            buf = fe.ch & 0xFF; // ch
            if(buf < min_ch) min_ch = buf;
            if(buf > max_ch) max_ch = buf;
        }
        sizeSpace_ip3rd = max_ip3rd - min_ip3rd + 1;
        sizeSpace_ip4th = max_ip4th - min_ip4th + 1;
        sizeSpace_ch = max_ch - min_ch + 1;
        sizeSpace_FEKey = sizeSpace_ip3rd * sizeSpace_ip4th * sizeSpace_ch;
        std::cout << "\tfItems.size(): " << fItems.size() << std::endl;
        getDopeKey_FEtoDET(min_ip3rd, min_ip4th, min_ch, minFEId); // minFEIdを参照で渡している。関数内で代入がある。
        getDopeKey_FEtoDET(max_ip3rd, max_ip4th, max_ch, maxFEId); // maxFEIdを参照で渡している。関数内で代入がある。
        // スキャン終わり
        printFEtoDETscan();
        fill_ratio = static_cast<double>(fItems.size()) / sizeSpace_FEKey;

        std::vector<DETIdItem> fetodet_dopevector(sizeSpace_FEKey); // fe.idをインデックスとするdope-vectorを用意
        for(const auto& item : fItems){
            uint32_t doped_index;
            if(!getDopeKey_FEtoDET( (item.fe.ip3rd) & 0xFF, (item.fe.ip4th) & 0xFF, item.fe.ch & 0xFF, doped_index )) {
                std::cerr << "これは設計上ありえないことですが、dope keyが範囲外です: " << std::endl;
                item.fe.decode();
                continue;
            }
            else{
                #if DEBUG_PRINT
                std::cout << "calculated dope index: 0x" << std::hex << std::setw(8) << std::setfill('0') << doped_index << std::dec << " for FE id: 0x" << std::hex << std::setw(8) << std::setfill('0') << item.fe.id << std::dec << std::endl;
                #endif
            }
            fetodet_dopevector[doped_index] = item.det;
        }
        fItemsFEtoDET_dope = fetodet_dopevector;

        if(createInvMap){
            ChannelMapDopeness::initialize_InvMap();
        }

        std::vector<FEAddrItem> fe_items;
        std::vector<DETIdItem> det_items;
        for(const auto& item : fItems){
            fe_items.push_back(item.fe);
            det_items.push_back(item.det);
        }
        fItemsFE = fe_items;
        fItemsDET = det_items;
        fe_items.clear();
        det_items.clear();

        std::cout << "[ChannelMapDopeness::initialize] dope vector initialize finished" << std::endl;
        return fill_ratio;
    }// void ChannelMapDopeness::initialize

    void ChannelMapDopeness::initialize_InvMap() {
        // dope indexの最大値、最小値をスキャン
        min_name_idx = 0xFF;
        min_plane_idx = 0xFF;
        min_segment = 0xFF;
        min_channel_number = 0xFFFF;
        min_readout_channel_idx = 0xFF;
        max_name_idx = 0;
        max_plane_idx = 0;
        max_segment = 0;
        max_channel_number = 0;
        max_readout_channel_idx = 0;
        uint8_t buf8;
        uint16_t buf16;
        for(const auto& item : fItems){
            auto det = item.det;
            buf8 = det.name & 0xFF; // name_idx
            if(buf8 < min_name_idx) min_name_idx = buf8;
            if(buf8 > max_name_idx) max_name_idx = buf8;
            buf8 = det.plane & 0xFF; // plane_idx
            if(buf8 < min_plane_idx) min_plane_idx = buf8;
            if(buf8 > max_plane_idx) max_plane_idx = buf8;
            buf8 = det.segment & 0xFF; // segment
            if(buf8 < min_segment) min_segment = buf8;
            if(buf8 > max_segment) max_segment = buf8;
            buf16 = det.channel_number & 0xFFFF; // channel_number
            if(buf16 < min_channel_number) min_channel_number = buf16;
            if(buf16 > max_channel_number) max_channel_number = buf16;
            buf8 = det.readout_channel & 0xFF; // readout_channel_idx
            if(buf8 < min_readout_channel_idx) min_readout_channel_idx = buf8;
            if(buf8 > max_readout_channel_idx) max_readout_channel_idx = buf8;
        }
        sizeSpace_name_idx = max_name_idx - min_name_idx + 1;
        sizeSpace_plane_idx = max_plane_idx - min_plane_idx + 1;
        sizeSpace_segment = max_segment - min_segment + 1;
        sizeSpace_channel_number = max_channel_number - min_channel_number + 1;
        sizeSpace_readout_channel_idx = max_readout_channel_idx - min_readout_channel_idx + 1;
        sizeSpace_DETKey = sizeSpace_name_idx * sizeSpace_plane_idx * sizeSpace_segment * sizeSpace_channel_number * sizeSpace_readout_channel_idx;
        getDopeKey_DETtoFE(min_name_idx, min_plane_idx, min_segment, min_channel_number, min_readout_channel_idx, minDETId); // minDETIdを参照で渡している。関数内で代入がある。
        getDopeKey_DETtoFE(max_name_idx, max_plane_idx, max_segment, max_channel_number, max_readout_channel_idx, maxDETId); // maxDETIdを参照で渡している。関数内で代入がある。
        // スキャン終わり
        printDETtoFEscan();

        // 逆引き用のdope vectorの初期化
        std::vector<FEAddrItem> dettofe_dopevector(sizeSpace_DETKey); // det infoをインデックスとする逆引き用dope-vectorを用意
        for(const auto& item : fItems){
            uint32_t doped_index;
            if(!getDopeKey_DETtoFE( (item.det.name) & 0xFF, (item.det.plane) & 0xFF, (item.det.segment) & 0xFF, (item.det.channel_number) & 0xFFFF, (item.det.readout_channel) & 0xFF, doped_index )) {
                std::cerr << "これは設計上ありえないことですが、逆引きのdope keyが範囲外です: " << std::endl;
                item.det.decode();
                continue;
            }
            dettofe_dopevector[doped_index] = item.fe;
        }
        fItemsDETtoFE_dope = dettofe_dopevector;
    } // void ChannelMapDopeness::initialize_InvMap()

    // ↓このコードの本質
    bool ChannelMapDopeness::getDopeKey_FEtoDET(uint8_t ip3rd, uint8_t ip4th, uint8_t ch, uint32_t& retKey) const {
        if(ip3rd < min_ip3rd || ip3rd > max_ip3rd || ip4th < min_ip4th || ip4th > max_ip4th || ch < min_ch || ch > max_ch) {
            return false;
        }
        retKey = ( (ip3rd - min_ip3rd) * sizeSpace_ip4th * sizeSpace_ch ) + ( (ip4th - min_ip4th) * sizeSpace_ch ) + (ch - min_ch);
        return true;
    } // std::optional<uint32_t> ChannelMapDopeness::getDopeKey_FEtoDET

    // ↓このコードの本質
    bool ChannelMapDopeness::getDopeKey_DETtoFE(uint8_t name_idx, uint8_t plane, uint8_t segment, uint16_t channelnumber_idx, uint8_t readout_channel_idx, uint32_t& retKey) const {
        if(name_idx < min_name_idx || name_idx > max_name_idx || plane < min_plane_idx || plane > max_plane_idx || segment < min_segment || segment > max_segment || channelnumber_idx < min_channel_number || channelnumber_idx > max_channel_number || readout_channel_idx < min_readout_channel_idx || readout_channel_idx > max_readout_channel_idx) {
            return false;
        }
        retKey = ( (name_idx - min_name_idx) * sizeSpace_plane_idx * sizeSpace_segment * sizeSpace_channel_number * sizeSpace_readout_channel_idx ) + ( (plane - min_plane_idx) * sizeSpace_segment * sizeSpace_channel_number * sizeSpace_readout_channel_idx ) + ( (segment - min_segment) * sizeSpace_channel_number * sizeSpace_readout_channel_idx ) + ( (channelnumber_idx - min_channel_number) * sizeSpace_readout_channel_idx ) + (readout_channel_idx - min_readout_channel_idx);
        return true;
    } // bool ChannelMapDopeness::getDopeKey_DETtoFE

    DETIdItem& ChannelMapDopeness::getDETItem(uint32_t doped_index){
        return fItemsFEtoDET_dope[doped_index];
    } // DETIdItem ChannelMapDopeness::getDETItem

    FEAddrItem& ChannelMapDopeness::getFEIItem(uint32_t doped_index){
        return fItemsDETtoFE_dope[doped_index];
    } // FEAddrItem ChannelMapDopeness::getFEIItem

    void ChannelMapDopeness::printAllItemsFE() {
        std::cout << "FE items count: " << fItemsFE.size() << std::endl;
        std::cout << "All FE Items:" << std::endl;
        for(auto& item : fItemsFE){
            item.decode();
        }
    }// void ChannelMapDopeness::printAllItemsFE

    void ChannelMapDopeness::printAllItemsDET() {
        std::cout << "DET items count: " << fItemsDET.size() << std::endl;
        std::cout << "All DET Items:" << std::endl;
        for(auto& item : fItemsFE) {
            uint32_t doped_index;
            if(!getDopeKey_FEtoDET( (item.ip3rd) & 0xFF, (item.ip4th) & 0xFF, item.ch & 0xFF, doped_index)) {
                std::cerr << "これは設計上ありえないことですが、dope keyが範囲外です: " << std::endl;
                item.decode();
                continue;
            }else {

            }
            DETIdItem det_item = getDETItem(doped_index);
            det_item.decode();
        }
    }// void ChannelMapDopeness::printAllItemsDET

    void ChannelMapDopeness::printFEid(FEAddrItem fe_item) {
        fe_item.decode();
    }// void ChannelMapDopeness::printFEid

    void ChannelMapDopeness::printDETinfo(DETIdItem det_item) {
        det_item.decode();
    }// void ChannelMapDopeness::printDETinfo

    void ChannelMapDopeness::NameIndexDictionary::newWord(const std::string& str){
        if(std::find(names_str.begin(), names_str.end(), str) == names_str.end()){
            // str is not in names_str, so add it
            names_str.push_back(str);
        }
    } // void ChannelMapDopeness::NameIndexDictionary::newWord

    void ChannelMapDopeness::NameIndexDictionary::sortWords(){
        std::sort(names_str.begin(), names_str.end());
    } // void ChannelMapDopeness::NameIndexDictionary::sortWords

    void ChannelMapDopeness::NameIndexDictionary::buildDictionary(){
        for(size_t i=0; i<names_str.size(); ++i){
            forward_d.emplace_back(names_str[i], static_cast<uint8_t>(i));
            inverse_d.push_back(names_str[i]);
        }
    } // void ChannelMapDopeness::NameIndexDictionary::buildDictionary

    bool ChannelMapDopeness::NameIndexDictionary::invIndex(uint8_t idx, std::string& str) const {
        if(idx < inverse_d.size()){
            str = inverse_d[idx];
            return true;
        }else{
            return false;
        }
    } // bool ChannelMapDopeness::NameIndexDictionary::invIndex

    bool ChannelMapDopeness::NameIndexDictionary::getIndex(const std::string& str, uint8_t& idx) const {
        auto it = std::find_if(forward_d.begin(), forward_d.end(), [&str](const std::pair<std::string, uint8_t>& pair){
            return pair.first == str;
        });
        if(it != forward_d.end()){
            idx = it->second;
            return true;
        }else{
            return false;
        }
    } // bool ChannelMapDopeness::NameIndexDictionary::getIndex

}// namespace chmap