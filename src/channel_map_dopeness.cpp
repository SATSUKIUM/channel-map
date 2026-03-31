#include "channel_map_dopeness.hpp"
#include "channel_map_simple_item.hpp"
#include "channel_tuple.hpp"
#include "element.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cctype>

#include <variant>

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
namespace chmap {    
    ChannelMapDopeness& ChannelMapDopeness::get_instance() {
        static ChannelMapDopeness instance;
        return instance;
    }// ChannelMapDopeness& ChannelMapDopeness::get_instance()
    ChannelMapDopeness::~ChannelMapDopeness() {
        // destructor
    }// ChannelMapDopeness::~ChannelMapDopeness()
    
    double ChannelMapDopeness::initialize(const std::string& file_path, bool createInvMap) {
        /*
        1. defineDictionary()で長いstringを4文字charにmapする
        2. csvを読んで、FE, DETの情報をそれぞれ持つChannelMapSimpleItemをstd::vector<ChannelMapSimpleItem> fItemsに入れる
        3. fItemsをFEのidの昇順でソートする(不要かも...)
        4. fItemsをスキャンして、dope vectorのサイズを決める

        */
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

        // fItems, fItemsFE, fItemsDETの初期化
        readCSV(file_path); // fill fItems

        #if DEBUG_PRINT
        std::cout << "start sorting fItems by fe.id" << std::endl;
        #endif
        // sort fItems by fe.id
        std::sort(fItems.begin(), fItems.end(), [](const ChannelMapSimpleItem& left, const ChannelMapSimpleItem& right) {
            return left.fe < right.fe; // checkDuplicateFEIDsの狭義弱順序がこの不等号の向きに依存している
        });
        #if DEBUG_PRINT
        std::cout << "finished sorting fItems" << std::endl;
        #endif


        std::cout << "[ChannelMapDopeness::initialize] dope vector initialize start" << std::endl;
        std::cout << "\tnumber of items: " << fItems.size() << std::endl;
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
        #if 1
        std::cout << "min_ip3rd: 0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<uint32_t>(min_ip3rd) << std::dec << std::endl;
        std::cout << "max_ip3rd: 0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<uint32_t>(max_ip3rd) << std::dec << std::endl;
        std::cout << "min_ip4th: 0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<uint32_t>(min_ip4th) << std::dec << std::endl;
        std::cout << "max_ip4th: 0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<uint32_t>(max_ip4th) << std::dec << std::endl;
        std::cout << "min_ch: 0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<uint32_t>(min_ch) << std::dec << std::endl;
        std::cout << "max_ch: 0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<uint32_t>(max_ch) << std::dec << std::endl;
        std::cout << "calculated FE key space: " << (max_ip3rd - min_ip3rd + 1) << " * " << (max_ip4th - min_ip4th + 1) << " * " << (max_ch - min_ch + 1) << std::endl;
        #endif
        sizeSpace_ip3rd = max_ip3rd - min_ip3rd + 1;
        sizeSpace_ip4th = max_ip4th - min_ip4th + 1;
        sizeSpace_ch = max_ch - min_ch + 1;
        sizeSpace_FEKey = sizeSpace_ip3rd * sizeSpace_ip4th * sizeSpace_ch;
        getDopeKey_FE(min_ip3rd, min_ip4th, min_ch, minFEId); // minFEIdを参照で渡している。関数内で代入がある。
        getDopeKey_FE(max_ip3rd, max_ip4th, max_ch, maxFEId); // maxFEIdを参照で渡している。関数内で代入がある。
        // スキャン終わり


        std::cout << "\tFE key space size: " << sizeSpace_FEKey << std::endl;
        std::cout << "\t\tsizeSpace_ip3rd: 0x" << std::setw(2) << std::hex << sizeSpace_ip3rd << " (" << std::setw(2) << std::setfill('0') << static_cast<uint32_t>(min_ip3rd) << " ~ " << std::setw(2) << std::setfill('0') << static_cast<uint32_t>(max_ip3rd) << ")" << std::endl;
        std::cout << "\t\tsizeSpace_ip4th: 0x" << std::setw(2) << std::hex << sizeSpace_ip4th << " (" << std::setw(2) << std::setfill('0') << static_cast<uint32_t>(min_ip4th) << " ~ " << std::setw(2) << std::setfill('0') << static_cast<uint32_t>(max_ip4th) << ")" << std::endl;
        std::cout << "\t\tsizeSpace_ch: 0x" << std::setw(2) << std::hex << sizeSpace_ch << " (" << std::setw(2) << std::setfill('0') << static_cast<uint32_t>(min_ch) << " ~ " << std::setw(2) << std::setfill('0') << static_cast<uint32_t>(max_ch) << ")" << std::endl;

        fill_ratio = static_cast<double>(fItems.size()) / sizeSpace_FEKey;
        std::cout << "\tFE ID range coverage: " << fill_ratio * 100.0 << " %" << std::endl;
        std::vector<ChannelMapSimpleItem_DET> fetodet_dopevector(sizeSpace_FEKey); // fe.idをインデックスとするdope-vectorを用意
        for(const auto& item : fItems){
            uint32_t doped_index;
            if(!getDopeKey_FE( (item.fe.ip3rd) & 0xFF, (item.fe.ip4th) & 0xFF, item.fe.ch & 0xFF, doped_index )) {
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

        std::vector<ChannelMapSimpleItem_FE> fe_items;
        std::vector<ChannelMapSimpleItem_DET> det_items;
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

        #if DEBUG_PRINT
        std::cout << "initialized ChannelMapDopeness with " << fItemsFE.size() << " items." << std::endl;
        #endif

    }// void ChannelMapDopeness::initialize

    void ChannelMapDopeness::readCSV(const std::string& file_path) {
        std::ifstream file(file_path);
        if (!file.is_open()) {
            std::cerr << "file open fail : " << file_path << std::endl;
            std::exit(1);
        }else {
            #if DEBUG_PRINT
            std::cout << "file opened: " << file_path << std::endl;
            #endif
        }


        // read csv header
        std::string line;
        if (std::getline(file, line)) {
            // load header, as template, assuming "fe.id, fe.channel, fe.data, detector.id, detector.plane, detector.segment, detector.channel, detector.data"
            for(const auto& header_part : split_line(line)) {
                if (std::count(m_header.begin(), m_header.end(), header_part) > 0) {
                    std::cerr << "found duplicate header column : " << header_part << std::endl;
                }
                m_header.push_back(header_part);
                auto type = split_line(header_part, '.')[0];
                m_element_type.push_back(type);
                if (std::find(m_unique_types.begin(), m_unique_types.end(), type) == m_unique_types.end()) {
                    m_unique_types.push_back(type);
                }
            }
            #if DEBUG_PRINT
            std::cout << "header loaded: ";
            for(const auto& h : m_header){
                std::cout << h << ", ";
            }
            std::cout << std::endl;
            std::cout << "element types: ";
            for(const auto& t : m_element_type){
                std::cout << t << ", ";
            }
            std::cout << std::endl;
            #endif
        }// if getline(file, line) for loeading header
        #if DEBUG_PRINT
        std::cout << "header loaded" << std::endl;
        #endif
        #if DEBUG_PRINT
        std::cout << "start loading mapdata lines" << std::endl;
        #endif

        // load mapdata lines
        while (std::getline(file, line)) {
            if(line.back() == '\r'){ // for Windwos-style line ending
                #if DEBUG_PRINT
                std::cout << "found Windows-style line ending" << std::endl;
                #endif
                line.pop_back();
            }
            #if DEBUG_PRINT
            std::cout << "loading line: " << line << std::endl;
            #endif

            auto tokens = split_line(line);
            if (tokens.size() != m_header.size()) {
                std::cerr << "bad file format : " << line << std::endl;
                continue;
            }
            ChannelMapSimpleItem item = makeSimpleItem(tokens);
            #if DEBUG_PRINT
            std::cout << "  made ChannelMapSimpleItem: " << std::endl;
            std::cout << "    FE id: 0x" << std::hex << std::setw(8) << std::setfill('0') << item.fe.id << std::dec << std::endl;
            std::cout << "    DET name: 0x" << std::hex << std::setw(8) << std::setfill('0') << item.det.name << std::dec
                      << ", plane: 0x" << std::hex << std::setw(4) << std::setfill('0') << item.det.plane << std::dec
                      << ", segment: " << static_cast<uint32_t>(item.det.segment)
                      << ", channel: 0x" << std::hex << std::setw(8) << std::setfill('0') << item.det.channel << std::dec
                      << std::endl;
            #endif
            fItems.push_back(item);
        }// while getline(file, line) for loading mapdata
        #if DEBUG_PRINT
        std::cout << "finished loading mapdata lines" << std::endl;
        std::cout << "total loaded items: " << fItems.size() << std::endl;
        #endif

    }

    double ChannelMapDopeness::initialize_InvMap() {
        double fill_ratio;

        // scan DET to FE side
        min_name[0] = 0xFF;
        min_name[1] = 0xFF;
        min_name[2] = 0xFF;
        min_name[3] = 0xFF;
        max_name[0] = 0;
        max_name[1] = 0;
        max_name[2] = 0;
        max_name[3] = 0;
        min_plane[0] = 0xFF;
        min_plane[1] = 0xFF;
        max_plane[0] = 0;
        max_plane[1] = 0;
        min_segment = 0xFF;
        max_segment = 0;
        min_channel[0] = 0xFF;
        min_channel[1] = 0xFF;
        min_channel[2] = 0xFF;
        min_channel[3] = 0xFF;
        max_channel[0] = 0;
        max_channel[1] = 0;
        max_channel[2] = 0;
        max_channel[3] = 0;
        for(const auto& item : fItems){
            auto det = item.det;
            for(int i=0; i<4; ++i){
                uint8_t buf = (det.name >> (8*(3-i))) & 0xFF; // nameを左から8bitずつ分割
                if(buf < min_name[i]) min_name[i] = buf;
                if(buf > max_name[i]) max_name[i] = buf;
            }
            for(int i=0; i<2; ++i){
                uint8_t buf = (det.plane >> (8*(1-i))) & 0xFF; // planeを左から8bitずつ分割
                if(buf < min_plane[i]) min_plane[i] = buf;
                if(buf > max_plane[i]) max_plane[i] = buf;
            }
            if(det.segment < min_segment) min_segment = det.segment;
            if(det.segment > max_segment) max_segment = det.segment;
            for(int i=0; i<4; ++i){
                uint8_t buf = (det.channel >> (8*(3-i))) & 0xFF; // channelを左から8bitずつ分割
                if(buf < min_channel[i]) min_channel[i] = buf;
                if(buf > max_channel[i]) max_channel[i] = buf;
            }
        }
        #if 1
        std::cout << "min_name: 0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<uint32_t>(min_name[0]) << " " << std::setw(2) << std::setfill('0') << static_cast<uint32_t>(min_name[1]) << " " << std::setw(2) << std::setfill('0') << static_cast<uint32_t>(min_name[2]) << " " << std::setw(2) << std::setfill('0') << static_cast<uint32_t>(min_name[3]) << std::dec << std::endl;
        std::cout << "max_name: 0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<uint32_t>(max_name[0]) << " " << std::setw(2) << std::setfill('0') << static_cast<uint32_t>(max_name[1]) << " " << std::setw(2) << std::setfill('0') << static_cast<uint32_t>(max_name[2]) << " " << std::setw(2) << std::setfill('0') << static_cast<uint32_t>(max_name[3]) << std::dec << std::endl;
        std::cout << "min_plane: 0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<uint32_t>(min_plane[0]) << " " << std::setw(2) << std::setfill('0') << static_cast<uint32_t>(min_plane[1]) << std::dec << std::endl;
        std::cout << "max_plane: 0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<uint32_t>(max_plane[0]) << " " << std::setw(2) << std::setfill('0') << static_cast<uint32_t>(max_plane[1]) << std::dec << std::endl;
        std::cout << "min_segment: 0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<uint32_t>(min_segment) << std::dec << std::endl;
        std::cout << "max_segment: 0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<uint32_t>(max_segment) << std::dec << std::endl;
        std::cout << "min_channel: 0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<uint32_t>(min_channel[0]) << " " << std::setw(2) << std::setfill('0') << static_cast<uint32_t>(min_channel[1]) << " " << std::setw(2) << std::setfill('0') << static_cast<uint32_t>(min_channel[2]) << " " << std::setw(2) << std::setfill('0') << static_cast<uint32_t>(min_channel[3]) << std::dec << std::endl;
        std::cout << "max_channel: 0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<uint32_t>(max_channel[0]) << " " << std::setw(2) << std::setfill('0') << static_cast<uint32_t>(max_channel[1]) << " " << std::setw(2) << std::setfill('0') << static_cast<uint32_t>(max_channel[2]) << " " << std::setw(2) << std::setfill('0') << static_cast<uint32_t>(max_channel[3]) << std::dec << std::endl;
        #endif

        for(int i=0; i<4; ++i){
            sizeSpace_name[i] = max_name[i] - min_name[i] + 1;
            sizeSpace_part[i] = sizeSpace_name[i];
            min_DET_part[i] = min_name[i];
            max_DET_part[i] = max_name[i];
        }
        for(int i=0; i<2; ++i){
            sizeSpace_plane[i] = max_plane[i] - min_plane[i] + 1;
            sizeSpace_part[i+4] = sizeSpace_plane[i];
            min_DET_part[i+4] = min_plane[i];
            max_DET_part[i+4] = max_plane[i];
        }
        sizeSpace_segment = max_segment - min_segment + 1;
        sizeSpace_part[6] = sizeSpace_segment;
        min_DET_part[6] = min_segment;
        max_DET_part[6] = max_segment;
        for(int i=0; i<4; ++i){
            sizeSpace_channel[i] = max_channel[i] - min_channel[i] + 1;
            sizeSpace_part[i+7] = sizeSpace_channel[i];
            min_DET_part[i+7] = min_channel[i];
            max_DET_part[i+7] = max_channel[i];
        }
        std::cout << "\tDET name space size: " << sizeSpace_name[0] << " * " << sizeSpace_name[1] << " * " << sizeSpace_name[2] << " * " << sizeSpace_name[3] << std::endl;
        std::cout << "\tDET plane space size: " << sizeSpace_plane[0] << " * " << sizeSpace_plane[1] << std::endl;
        std::cout << "\tDET segment space size: " << sizeSpace_segment << std::endl;
        std::cout << "\tDET channel space size: " << sizeSpace_channel[0] << " * " << sizeSpace_channel[1] << " * " << sizeSpace_channel[2] << " * " << sizeSpace_channel[3] << std::endl;
        sizeSpace_DETKey = sizeSpace_name[0] * sizeSpace_name[1] * sizeSpace_name[2] * sizeSpace_name[3] * sizeSpace_plane[0] * sizeSpace_plane[1] * sizeSpace_segment * sizeSpace_channel[0] * sizeSpace_channel[1] * sizeSpace_channel[2] * sizeSpace_channel[3];

    }
    // ↓このコードの本質
    bool ChannelMapDopeness::getDopeKey_FE(uint8_t ip3rd, uint8_t ip4th, uint8_t ch, uint32_t& retKey) const {
        if(ip3rd < min_ip3rd || ip3rd > max_ip3rd || ip4th < min_ip4th || ip4th > max_ip4th || ch < min_ch || ch > max_ch) {
            return false;
        }
        retKey = ( (ip3rd - min_ip3rd) * sizeSpace_ip4th * sizeSpace_ch ) + ( (ip4th - min_ip4th) * sizeSpace_ch ) + (ch - min_ch);
        return true;
    } // std::optional<uint32_t> ChannelMapDopeness::getDopeKey_FE

    uint32_t ChannelMapDopeness::unchecked_getDopeKey_FE(uint8_t ip3rd, uint8_t ip4th, uint8_t ch) const {
        return ( (ip3rd - min_ip3rd) * sizeSpace_ip4th * sizeSpace_ch ) + ( (ip4th - min_ip4th) * sizeSpace_ch ) + (ch - min_ch);
    } // uint32_t ChannelMapDopeness::unchecked_getDopeKey_FE

    bool ChannelMapDopeness::getDopeKey_DET(uint32_t name, uint16_t plane, uint8_t segment, uint32_t channel, uint32_t& retKey) const {
        uint8_t buf;
        // check if the input values are within the defined space
        for(int i=0; i<4; ++i){
            buf = (name >> (8*(3-i))) & 0xFF; // nameを左から8bitずつ分割
            if(buf < min_name[i] || buf > max_name[i]) {
                return false;
            }
        }
        for(int i=0; i<2; ++i){
            buf = (plane >> (8*(1-i))) & 0xFF; // planeを左から8bitずつ分割
            if(buf < min_plane[i] || buf > max_plane[i]) {
                return false;
            }
        }
        buf = segment & 0xFF;
        if(buf < min_segment || buf > max_segment) {
            return false;
        }
        for(int i=0; i<4; ++i){
            buf = (channel >> (8*(3-i))) & 0xFF; // channelを左から8bitずつ分割
            if(buf < min_channel[i] || buf > max_channel[i]) {
                return false;
            }
        }

        // calculate the dope key
        uint8_t ret_part[11];
        for(int i=0; i<4; ++i){
            ret_part[i] = (name >> (8*(3-i))) & 0xFF; // nameを左から8bitずつ分割
        }
        for(int i=0; i<2; ++i){
            ret_part[i+4] = (plane >> (8*(1-i))) & 0xFF; // planeを左から8bitずつ分割
        }
        ret_part[6] = segment & 0xFF;
        for(int i=0; i<4; ++i){
            ret_part[i+7] = (channel >> (8*(3-i))) & 0xFF; // channelを左から8bitずつ分割
        }

        retKey = 0;
        uint32_t cofactor;
        for(int i=0; i<11; ++i){
            for(int j=0; j<11-i - 1; ++j){
                cofactor += sizeSpace_part[i+1 + j];
            }
            retKey += (ret_part[i] - min_DET_part[i]) * cofactor;
        }
        return true;
    }

    std::vector<std::string> ChannelMapDopeness::split_line(const std::string& line, char delimiter) {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream iss(line);
        while (std::getline(iss, token, delimiter)) {
            tokens.push_back(token);
        }
        return tokens;
    }// std::vector<std::string> ChannelMapDopeness::split_line

    ChannelMapSimpleItem ChannelMapDopeness::makeSimpleItem(const std::vector<std::string>& tokens) {
        int len_tokens = tokens.size();
        uint64_t fe_ip_full;
        uint16_t fe_ip_3rd_4th;
        uint8_t fe_channel;
        uint32_t det_name;
        uint16_t det_plane;
        uint8_t det_segment;
        uint32_t det_channel;
        std::string det_name_str;
        std::string det_plane_str;
        std::string det_channel_str;

        int fe_magic = 3; // id, channel, data
        int fe_count = 0;
        int det_magic = 5; // id, plane, segment, channel, data
        int det_count = 0;
        #if DEBUG_PRINT
        std::cout << "making ChannelMapSimpleItem from tokens:" << std::endl;
        for(const auto& t : tokens){
            std::cout << "  " << t << std::endl;
        }
        #endif
        for(int i=0; i<len_tokens; ++i) {
            std::string type = m_element_type[i];
            if(type == "fe") {
                // parse front-end related tokens
                if(fe_count == 0) {
                    #if DEBUG_PRINT
                    std::cout << "parsing token(fe_count == 0): " << tokens[i] << " (string)" << std::endl;
                    std::cout << "This token is interpreted as full FE IP address in uint64_t format " << std::hex << std::stoull(tokens[i], nullptr, 0) << std::dec << std::endl;
                    #endif
                    fe_ip_full = static_cast<uint64_t>(std::stoull(tokens[i], nullptr, 0));
                    fe_ip_3rd_4th = parse_to16(
                        std::to_string( (fe_ip_full) & 0xFFFF )
                    );
                } else if(fe_count == 1) {
                    #if DEBUG_PRINT
                    std::cout << "parsing token(fe_count == 1): " << tokens[i] << " (string)" << std::endl;
                    std::cout << "This token is interpreted as FE channel in uint8_t format " << std::stoul(tokens[i], nullptr, 0) << std::endl;
                    #endif
                    fe_channel = parse_to8(tokens[i]);
                }
                fe_count++;
            } else if(type == "detector") {
                // parse detector related tokens
                if(det_count == 0) {
                    det_name_str = tokens[i];
                    det_name = parse_to32(det_name_str);
                    #if DEBUG_PRINT
                    std::cout << "parsing token(det_count == 0): " << tokens[i] << " (string)" << std::endl;
                    std::cout << "This token is interpreted as detector name in uint32_t format " << std::hex << det_name << std::dec << std::endl;
                    std::cout << "This uint32_t corresponds to chars: "
                              << char((det_name >> 24) & 0xFF)
                              << char((det_name >> 16) & 0xFF)
                              << char((det_name >> 8) & 0xFF)
                              << char(det_name & 0xFF)
                              << std::endl;
                    #endif
                } else if(det_count == 1) {
                    det_plane_str = tokens[i];
                    det_plane = parse_to16(det_plane_str);
                    #if DEBUG_PRINT
                    std::cout << "parsing token(det_count == 1): " << tokens[i] << " (string)" << std::endl;
                    std::cout << "This token is interpreted as detector plane in uint16_t format " << std::hex << det_plane << std::dec << std::endl;
                    std::cout << "This uint16_t corresponds to chars: "
                              << char((det_plane >> 8) & 0xFF)
                              << char(det_plane & 0xFF)
                              << std::endl;
                    #endif
                } else if(det_count == 2) {
                    det_segment = parse_to8(tokens[i]);
                    #if DEBUG_PRINT
                    std::cout << "parsing token(det_count == 2): " << tokens[i] << " (string)" << std::endl;
                    std::cout << "This token is interpreted as detector segment in uint8_t format " << static_cast<uint32_t>(det_segment) << std::endl;
                    #endif
                } else if(det_count == 3) {
                    det_channel_str = tokens[i];
                    det_channel = parse_to32(det_channel_str);
                    #if DEBUG_PRINT
                    std::cout << "parsing token(det_count == 3): " << tokens[i] << " (string)" << std::endl;
                    std::cout << "This token is interpreted as detector channel in uint32_t format " << std::hex << det_channel << std::dec << std::endl;
                    std::cout << "This uint32_t corresponds to chars: "
                              << char((det_channel >> 24) & 0xFF)
                              << char((det_channel >> 16) & 0xFF)
                              << char((det_channel >> 8) & 0xFF)
                              << char(det_channel & 0xFF)
                              << std::endl;
                    #endif
                }
                det_count++;
            } // if type is fe or detector
        } // for loop for tokens

        ChannelMapSimpleItem_FE fe_item(fe_ip_3rd_4th >> 8, fe_ip_3rd_4th & 0xFF, fe_channel);
        ChannelMapSimpleItem_DET det_item(det_name, det_plane, det_segment, det_channel);

        #if DEBUG_PRINT
        std::cout << "constructed ChannelMapSimpleItem_FE: id=" << std::hex << fe_item.id << std::dec << std::endl;
        std::cout << "constructed ChannelMapSimpleItem_DET: name=" << std::hex << det_item.name << std::dec
                  << ", plane=" << std::hex << det_item.plane << std::dec
                  << ", segment=" << static_cast<uint32_t>(det_item.segment)
                  << ", channel=" << std::hex << det_item.channel << std::dec << std::endl;
        #endif
        return ChannelMapSimpleItem{fe_item, det_item};
    }// ChannelMapSimpleItem ChannelMapDopeness::makeSimpleItem

    ChannelMapSimpleItem_DET ChannelMapDopeness::getDETItem(uint32_t doped_index){
        return fItemsFEtoDET_dope[doped_index];
    }// ChannelMapSimpleItem_DET* ChannelMapDopeness::getDETItem

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
            if(!getDopeKey_FE( (item.ip3rd) & 0xFF, (item.ip4th) & 0xFF, item.ch & 0xFF, doped_index)) {
                std::cerr << "これは設計上ありえないことですが、dope keyが範囲外です: " << std::endl;
                item.decode();
                continue;
            }else {

            }
            ChannelMapSimpleItem_DET det_item = getDETItem(doped_index);
            det_item.decode();
        }
    }// void ChannelMapDopeness::printAllItemsDET

    void ChannelMapDopeness::checkDuplicateFEIDs() {
        std::cout << "\n[src/channel_map_dopeness.cpp/checkDuplicateFEIDs] checking sequence of FE IDs for duplicates..." << std::endl;
        for(const auto& item : fItemsFE) {
            auto range = std::equal_range(fItemsFE.begin(), fItemsFE.end(), item,
                [](const ChannelMapSimpleItem_FE& left, const ChannelMapSimpleItem_FE& right) {
                    return left < right;
                } // 狭義弱順序の不等号はfItemsFEをソートする順番に合わせる必要がある。
            );
            size_t count = std::distance(range.first, range.second);
            if(count > 1) {
                std::cout << "\tduplicate FEID found(count: " << count << "): ";
                printFEid(item);
                for(auto it = range.first; it != range.second; ++it){
                    ChannelMapSimpleItem_DET det_item = fItemsDET[std::distance(fItemsFE.begin(), it)];
                    std::cout << "\t\tcorresponding DET info: ";
                    printDETinfo(det_item);
                }
            }
        }
        std::cout << "[src/channel_map_dopeness.cpp/checkDuplicateFEIDs] check completed." << std::endl;
    }// void ChannelMapDopeness::checkDuplicateFEIDs

    void ChannelMapDopeness::checkDuplicateFEIDs_summary(){
        std::cout << "\n[src/channel_map_dopeness.cpp/checkDuplicateFEIDs_summary] checking sequence of FE IDs for duplicates..." << std::endl;
        auto fItemsFE_copy = fItemsFE;
        int duplicate_numGroups = 0;
        int duplicate_totalCount = 0;
        for(const auto& item : fItemsFE_copy){
            auto range = std::equal_range(fItemsFE_copy.begin(), fItemsFE_copy.end(), item, [](const ChannelMapSimpleItem_FE& left, const ChannelMapSimpleItem_FE& right){
                return left < right;
            });
            size_t count = std::distance(range.first, range.second);
            if(count > 1){
                duplicate_numGroups++;
                for(size_t i=0; i<count-1; ++i){// もとのものを残し、重複しているものを削除する
                    auto it = range.first + 1;
                    if(it != range.second){
                        fItemsFE_copy.erase(it);
                        duplicate_totalCount++;
                    }
                    else{
                        std::cout << "自分の考えが正しければ、このメッセージは出力されてはならない。" << std::endl;
                    }
                }
            }
        }
        std::cout << "[src/channel_map_dopeness.cpp/checkDuplicateFEIDs_summary] summary: " << duplicate_numGroups << " groups of duplicates found, total count of duplicates: " << duplicate_totalCount << "(means extra items found)"<< std::endl;
    }// void ChannelMapDopeness::checkDuplicateFEIDs_summary

    void ChannelMapDopeness::printFEid(ChannelMapSimpleItem_FE fe_item) {
        fe_item.decode();
    }// void ChannelMapDopeness::printFEid

    void ChannelMapDopeness::printDETinfo(ChannelMapSimpleItem_DET det_item) {
        det_item.decode();
    }// void ChannelMapDopeness::printDETinfo
}// namespace chmap