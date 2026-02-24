#include "channel_map_simple.hpp"
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
channel-mapをもとに、インプットとアウトプットの形式の自由度を減らすことにより、機能を限定し、処理の高速化を図る、channel-map-simpleの実装。
    - channel-map-simpleでは、インプットファイルのフォーマットは
        "fe.id, fe.channel, fe.data, detector.id, detector.plane, detector.segment, detector.channel, detector.data"
        のように、3要素と5要素の組み合わせに限定する。
        fe.idは64bit整数のデータを仮定し、その下位16bit(192.168.FF.FF)を使用する。
        fe.channelは最大16bit整数、fe.dataは使用しない。
        detector.idは最大4文字の文字列、detector.planeは最大2文字の文字列、detector.segmentは最大8bit整数、detector.channelは最大4文字の文字列、detector.dataは使用しない。
    - detector.id, detector.plane, detector.channelについては、固定長文字列への変換をハードコードで実装している。
        例えば、detector.idには "utof", "dtof", "ltof", "t0", "t0ref", "t1", "all_charged", "bftref", "bdc", "kldc", "left", "right", "bottom", "upstream", "downstream" が使える。
        また、detector.planeには "X", "U", "V", "Xp", "Up", "Vp" が使える。
    - FE側のIP, CHをまとめてuint32_tのidにし、、detector側のname, plane, segment, channelをそれぞれuint32_t, uint16_t, uint8_t, uint32_tに変換して格納する。
    - そのペアをFE側のidの大小でソートし、vectorに格納する。
    - DAQワードに記載されているFEのIP,CHから作るキーで二分探索を行い、得たrankを用いてdetector側のvectorの要素にアクセスする
*/
/*
    class ChannelMapSimple method list
        public:
            static ChannelMapSimple& get_instance();
            ~ChannelMapSimple();

            void initialize(const std::string& file_path);
        private:
            std::vector<std::string> split_line(const std::string& line, char delimiter = ',');
            std::vector<std::string> m_header, m_element_type, m_unique_types;
            ChannelMapSimpleItem makeSimpleItem(const std::vector<std::string>& tokens);
            void simplify_detector_names();
            uint32_t four_char_to_uint32(char a, char b, char c, char d);
            uint16_t four_char_to_uint16(char a, char b);
            bool isTokenNumeric(const std::string& token);
            uint32_t parse_to32(const std::string& token);
            uint16_t parse_to16(const std::string& token);
            uint8_t parse_to8(const std::string& token);
*/
namespace chmap {    
    ChannelMapSimple& ChannelMapSimple::get_instance() {
        static ChannelMapSimple instance;
        return instance;
    }// ChannelMapSimple& ChannelMapSimple::get_instance()
    ChannelMapSimple::~ChannelMapSimple() {
        // destructor
    }// ChannelMapSimple::~ChannelMapSimple()
    
    void ChannelMapSimple::initialize(const std::string& file_path) {
        simplify_detector_names(); // prepare detname_simplify_map
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

        std::ifstream file(file_path);
        if (!file.is_open()) {
            std::cerr << "file open fail : " << file_path << std::endl;
            std::exit(1);
        }else {
            #if DEBUG_PRINT
            std::cout << "file opened: " << file_path << std::endl;
            #endif
        }

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

        #if DEBUG_PRINT
        std::cout << "start sorting fItems by fe.id" << std::endl;
        #endif
        // sort fItems by fe.id
        std::sort(fItems.begin(), fItems.end(), [](const ChannelMapSimpleItem& left, const ChannelMapSimpleItem& right) {
            return left.fe.id < right.fe.id; // checkDuplicateFEIDsの狭義弱順序がこの不等号の向きに依存している
        });
        #if DEBUG_PRINT
        std::cout << "finished sorting fItems" << std::endl;
        #endif

        std::vector<ChannelMapSimpleItem_FE> fe_items;
        std::vector<ChannelMapSimpleItem_DET> det_items;
        for(const auto& item : fItems){
            fe_items.push_back(item.fe);
            det_items.push_back(item.det);
        }
        fItemsFE = fe_items;
        fItemsDET = det_items;

        #if DEBUG_PRINT
        std::cout << "initialized ChannelMapSimple with " << fItemsFE.size() << " items." << std::endl;
        #endif

    }// void ChannelMapSimple::initialize

    std::vector<std::string> ChannelMapSimple::split_line(const std::string& line, char delimiter) {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream iss(line);
        while (std::getline(iss, token, delimiter)) {
            tokens.push_back(token);
        }
        return tokens;
    }// std::vector<std::string> ChannelMapSimple::split_line

    ChannelMapSimpleItem ChannelMapSimple::makeSimpleItem(const std::vector<std::string>& tokens) {
        int len_tokens = tokens.size();
        uint64_t fe_ip_full;
        uint16_t fe_ip_3rd_4th;
        uint16_t fe_channel;
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
                    std::cout << "This token is interpreted as FE channel in uint16_t format " << std::stoul(tokens[i], nullptr, 0) << std::endl;
                    #endif
                    fe_channel = parse_to16(tokens[i]);
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
        ChannelMapSimpleItem_DET det_item;
        det_item.name = det_name;
        det_item.plane = det_plane;
        det_item.segment = det_segment;
        det_item.channel = det_channel;
        #if DEBUG_PRINT
        std::cout << "constructed ChannelMapSimpleItem_FE: id=" << std::hex << fe_item.id << std::dec << std::endl;
        std::cout << "constructed ChannelMapSimpleItem_DET: name=" << std::hex << det_item.name << std::dec
                  << ", plane=" << std::hex << det_item.plane << std::dec
                  << ", segment=" << static_cast<uint32_t>(det_item.segment)
                  << ", channel=" << std::hex << det_item.channel << std::dec << std::endl;
        #endif
        return ChannelMapSimpleItem{fe_item, det_item};
    }// ChannelMapSimpleItem ChannelMapSimple::makeSimpleItem

    void ChannelMapSimple::simplify_detector_names(){
        // 1st is original name, 2nd is simplified name(uint32_t)
        // if simplified name is shrter than 4 char, fill with space char in the end
        const std::vector<std::pair<std::string, std::string>> detnames = {
            {"utof", "UTOF"},
            {"dtof", "DTOF"},
            {"ltof", "LTOF"},
            {"t0", "T0  "},
            {"t0ref", "T0RF"},
            {"t1", "T1  "},
            {"all_charged", "ALCH"},
            {"bftref", "BFTR"},
            {"bht", "BHT "},
            {"bft", "BFT "},
            {"sft", "SFT "},
            {"bdc", "BDC "},
            {"kldc", "KLDC"},
            {"left", "LEFT"},
            {"right", "RIGT"},
            {"top", "TOP "},
            {"bottom", "BOTM"},
            {"upstream", "UPST"},
            {"downstream", "DOST"},
            {"nil", "NIL "}
        };
        // make simplified map
        for(const auto& name_pair : detnames){
            uint32_t simplified = four_char_to_uint32(
                name_pair.second[0],
                name_pair.second[1],
                name_pair.second[2],
                name_pair.second[3]
            );
            mapdata_string_simplify_map32[name_pair.first] = simplified;
        }

        // 1st is original name, 2nd is simplified name(uint16_t)
        const std::vector<std::pair<std::string, std::string>> detplanes = {
            {"X", "X "},
            {"U", "U "},
            {"V", "V "},
            {"Xp", "XP"},
            {"Up", "UP"},
            {"Vp", "VP"},
            {"nil", "NI"}            
        };
        // make simplified map
        for(const auto& plane_pair : detplanes){
            uint16_t simplified = four_char_to_uint16(
                plane_pair.second[0],
                plane_pair.second[1]
            );
            mapdata_string_simplify_map16[plane_pair.first] = simplified;
        }
    }// void ChannelMapSimple::simplify_detector_names

    uint32_t ChannelMapSimple::four_char_to_uint32(char a, char b, char c, char d) {
        // 4つのcharをuint32_tに変換するルールを規定
        return (uint32_t(uint8_t(a)) << 24) | (uint32_t(uint8_t(b)) << 16) | (uint32_t(uint8_t(c)) << 8) | uint32_t(uint8_t(d));
    }// uint32_t ChannelMapSimple::four_char_to_uint32

    uint16_t ChannelMapSimple::four_char_to_uint16(char a, char b) {
        // 2つのcharをuint16_tに変換するルールを規定
        return (uint16_t(uint8_t(a)) << 8) | uint16_t(uint8_t(b));
    }// uint16_t ChannelMapSimple::four_char_to_uint16

    bool ChannelMapSimple::isTokenNumeric(const std::string& token) {
        // return true if token is numeric
        return !token.empty() && std::all_of(token.begin(), token.end(), ::isdigit);
    } // bool ChannelMapSimple::isTokenNumeric

    uint32_t ChannelMapSimple::parse_to32(const std::string& token) {
        // assuming token is for example "0", "utof", "t0", "all_charged", "200", and parse to "00000000", "55544F46", "54302020", "414C4348", "000000C8" respectively
        if (isTokenNumeric(token)) {
            return static_cast<uint32_t>(std::stoul(token, nullptr, 0));
        } else {
            auto it = mapdata_string_simplify_map32.find(token);// check in detname_simplify_map
            if(it == mapdata_string_simplify_map32.end()) {
                std::cerr << "unknown token for uint32_t conversion: " << token << std::endl;
                std::exit(1);
            } else {
                uint32_t simplified = it->second;
                return simplified;
            }
        }
    }// uint32_t ChannelMapSimple::parse_to32

    uint16_t ChannelMapSimple::parse_to16(const std::string& token) {
        if (isTokenNumeric(token)) {
            return static_cast<uint16_t>(std::stoul(token, nullptr, 0));
        } else {
            auto it = mapdata_string_simplify_map16.find(token);// check in detname_simplify_map
            if(it == mapdata_string_simplify_map16.end()) {
                std::cerr << "unknown token for uint16_t conversion: " << token << std::endl;
                std::exit(1);
            } else {
                uint16_t simplified = it->second;
                return simplified;
            }
        }
    }// uint16_t ChannelMapSimple::parse_to16

    uint8_t ChannelMapSimple::parse_to8(const std::string& token) {
        if (isTokenNumeric(token)) {
            return static_cast<uint8_t>(std::stoul(token, nullptr, 0));
        } else {
            std::cerr << "unknown token for uint8_t conversion: " << token << std::endl;
            std::exit(1);
        }
    }// uint8_t ChannelMapSimple::parse_to8

    size_t ChannelMapSimple::getFERank(uint8_t ip3rd, uint8_t ip4th, uint16_t ch) {
        uint32_t id = (uint32_t(ip3rd) << 24) | (uint32_t(ip4th) << 16) | uint32_t(ch);
        // binary search in fItemsFE, 
        #if DEBUG_PRINT_GETFERANK
        std::cout << "getFERank from " << fItemsFE.size() << " items" << std::endl;
        #endif
        auto it = std::lower_bound(fItemsFE.begin(), fItemsFE.end(), id,
            [](const ChannelMapSimpleItem_FE& item, uint32_t value) {
                return item.id < value;
            }
        );
        if(it != fItemsFE.end() && it->id == id) {
            return std::distance(fItemsFE.begin(), it);
        } else {
            return std::string::npos; // not found
        }
    }// size_t ChannelMapSimple::getFERank

    ChannelMapSimpleItem_DET* ChannelMapSimple::getDETItem(uint8_t ip3rd, uint8_t ip4th, uint16_t ch) {
        auto rank = getFERank(ip3rd, ip4th, ch);
        if(rank != std::string::npos) {
            return &fItemsDET[rank];
        } else {
            return nullptr; // not found
        }
    }// ChannelMapSimpleItem_DET* ChannelMapSimple::getDETItem

    void ChannelMapSimple::printAllItemsFE() {
        std::cout << "FE items count: " << fItemsFE.size() << std::endl;
        std::cout << "All FE Items:" << std::endl;
        for(const auto& item : fItemsFE) {
            std::cout << "  FE id: 0x" << std::hex << std::setw(8) << std::setfill('0') << item.id << std::dec << std::endl;
        }
    }// void ChannelMapSimple::printAllItemsFE
    void ChannelMapSimple::printAllItemsDET() {
        std::cout << "DET items count: " << fItemsDET.size() << std::endl;
        std::cout << "All DET Items:" << std::endl;
        for(const auto& item : fItemsDET) {
            std::cout << "  DET name: " << std::hex << std::setw(8) << std::setfill('0') << item.name << std::dec
                      << ", plane: " << std::hex << std::setw(8) << std::setfill('0') << item.plane << std::dec
                      << ", segment: " << static_cast<uint32_t>(item.segment)
                      << ", channel: " << std::hex << std::setw(8) << std::setfill('0') << item.channel << std::dec
                      << std::endl;
        }
    }// void ChannelMapSimple::printAllItemsDET

    void ChannelMapSimple::checkDuplicateFEIDs() {
        std::cout << "\n[src/channel_map_simple.cpp/checkDuplicateFEIDs] checking sequence of FE IDs for duplicates..." << std::endl;
        for(const auto& item : fItemsFE) {
            auto range = std::equal_range(fItemsFE.begin(), fItemsFE.end(), item,
                [](const ChannelMapSimpleItem_FE& left, const ChannelMapSimpleItem_FE& right) {
                    return left.id < right.id;
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
        std::cout << "[src/channel_map_simple.cpp/checkDuplicateFEIDs] check completed." << std::endl;
    }// void ChannelMapSimple::checkDuplicateFEIDs
    
    void ChannelMapSimple::checkDuplicateFEIDs_summary(){
        std::cout << "\n[src/channel_map_simple.cpp/checkDuplicateFEIDs_summary] checking sequence of FE IDs for duplicates..." << std::endl;
        auto fItemsFE_copy = fItemsFE;
        int duplicate_numGroups = 0;
        int duplicate_totalCount = 0;
        for(const auto& item : fItemsFE_copy){
            auto range = std::equal_range(fItemsFE_copy.begin(), fItemsFE_copy.end(), item, [](const ChannelMapSimpleItem_FE& left, const ChannelMapSimpleItem_FE& right){
                return left.id < right.id;
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
        std::cout << "[src/channel_map_simple.cpp/checkDuplicateFEIDs_summary] summary: " << duplicate_numGroups << " groups of duplicates found, total count of duplicates: " << duplicate_totalCount << "(means extra items found)"<< std::endl;
    }// void ChannelMapSimple::checkDuplicateFEIDs_summary

    void ChannelMapSimple::printFEid(ChannelMapSimpleItem_FE fe_item) {
        fe_item.decode();
    }// void ChannelMapSimple::printFEid

    void ChannelMapSimple::printDETinfo(ChannelMapSimpleItem_DET det_item) {
        det_item.decode();
    }// void ChannelMapSimple::printDETinfo

    void ChannelMapSimple::makeDummyEntry(uint32_t maxFillFactor) {
        std::vector<ChannelMapSimpleItem_FE> new_fe_items;
        std::vector<ChannelMapSimpleItem_DET> new_det_items;

        ChannelMapSimpleItem_DET dummy_det;
        ChannelMapSimpleItem_FE dummy_fe(0, 0, 0);
            // below ChannelMapSimpleItem_DET examlple
            // struct ChannelMapSimpleItem_DET {
            //     uint32_t name;// detector name in 4 char
            //     uint16_t plane;// plane name in 2 char
            //     uint8_t segment;// segment number in 8bit int (0-255)
            //     uint32_t channel;// channel name in 4 char
            // };
        dummy_det.name = parse_to32("nil");
        dummy_det.plane = parse_to16("nil");
        dummy_det.segment = 0;
        dummy_det.channel = parse_to32("nil");

        for(auto it = fItems.begin(); it != fItems.end(); ++it){
            if(it == fItems.begin()){
                uint32_t gap = it->fe.id - 0;
                if(gap < maxFillFactor){
                    if(gap == 0){
                        new_fe_items.push_back(it->fe);
                        new_det_items.push_back(it->det);
                        #if DEBUG_PRINT_DUMMY_MAKER
                        printFEid(it->fe);
                        printDETinfo(it->det);
                        #endif
                        continue; // no gap, no dummy entry needed
                    } // endif(gap == 0)
                    else{
                        for(int i=0; i<gap; ++i){
                            if(i == 0){ // for the original entry
                                new_fe_items.push_back(it->fe);
                                new_det_items.push_back(it->det);
                                #if DEBUG_PRINT_DUMMY_MAKER
                                printFEid(it->fe);
                                printDETinfo(it->det);
                                #endif
                            }
                            else{
                                dummy_fe.id = 0 + i;
                                new_fe_items.push_back(dummy_fe);
                                new_det_items.push_back(dummy_det);
                                #if DEBUG_PRINT_DUMMY_MAKER
                                printFEid(dummy_fe);
                                printDETinfo(dummy_det);
                                #endif
                            }
                        } // done(int i=0; i<gap; ++i)
                    } // endif(gap > 0)
                } // endif(gap < maxFillFactor)
                else{
                    // insert dummy entry
                    for(int i=0; i<maxFillFactor - 1; ++i){
                        if(i == 0){ // for the original entry
                            new_fe_items.push_back(it->fe);
                            new_det_items.push_back(it->det);
                            #if DEBUG_PRINT_DUMMY_MAKER
                            printFEid(it->fe);
                            printDETinfo(it->det);
                            #endif
                        }
                        else{
                            dummy_fe.id = 0 + gap/maxFillFactor * i;
                            new_fe_items.push_back(dummy_fe);
                            new_det_items.push_back(dummy_det);
                            #if DEBUG_PRINT_DUMMY_MAKER
                            printFEid(dummy_fe);
                            printDETinfo(dummy_det);
                            #endif
                        }
                    } // done(int i=0; i<maxFillFactor - 1; ++i)
                } // endif(gap >= maxFillFactor)
            } // endif(it == fItems.begin())
            else if(it != fItems.end() - 1){
                auto original_left = it;
                auto original_right = it + 1;
                uint32_t gap = (original_right->fe.id) - (original_left->fe.id);
                if(gap < maxFillFactor){
                    if(gap == 0){
                        new_fe_items.push_back(original_left->fe);
                        new_det_items.push_back(original_left->det);
                        #if DEBUG_PRINT_DUMMY_MAKER
                        printFEid(original_left->fe);
                        printDETinfo(original_left->det);
                        #endif
                        continue; // no gap, no dummy entry needed
                    } // endif(gap == 0)
                    else{
                        for(int i=0; i<gap; ++i){
                            if(i == 0){ // for the original left entry
                                new_fe_items.push_back(original_left->fe);
                                new_det_items.push_back(original_left->det);
                                #if DEBUG_PRINT_DUMMY_MAKER
                                printFEid(original_left->fe);
                                printDETinfo(original_left->det);
                                #endif
                            }
                            else{
                                dummy_fe.id = original_left->fe.id + i;
                                new_fe_items.push_back(dummy_fe);
                                new_det_items.push_back(dummy_det);
                                #if DEBUG_PRINT_DUMMY_MAKER
                                printFEid(dummy_fe);
                                printDETinfo(dummy_det);
                                #endif
                            }
                        }
                    } // endif(gap > 0)
                } // endif(gap < maxFillFactor)
                else{
                    // insert dummy entry
                    uint32_t gapId = original_right->fe.id - original_left->fe.id;
                    for(int i=0; i<maxFillFactor - 1; ++i){
                        if(i == 0){ // for the original left entry
                            new_fe_items.push_back(original_left->fe);
                            new_det_items.push_back(original_left->det);
                            #if DEBUG_PRINT_DUMMY_MAKER
                            printFEid(original_left->fe);
                            printDETinfo(original_left->det);
                            #endif
                        }
                        else{
                            dummy_fe.id = original_left->fe.id + gapId/maxFillFactor * i;
                            new_fe_items.push_back(dummy_fe);
                            new_det_items.push_back(dummy_det);
                            #if DEBUG_PRINT_DUMMY_MAKER
                            printFEid(dummy_fe);
                            printDETinfo(dummy_det);
                            #endif
                        }
                    } // done(int i=0; i<maxFillFactor - 1; ++i)
                } // endif(gap >= maxFillFactor)
            } // endif(it != fItems.end() - 1)
            else{
                uint32_t gap = sizeof(uint32_t) - (it->fe.id + 1);
                if(gap < maxFillFactor){
                    for(int i=0; i<gap; ++i){
                        if(i == 0){ // for the original entry
                            new_fe_items.push_back(it->fe);
                            new_det_items.push_back(it->det);
                            #if DEBUG_PRINT_DUMMY_MAKER
                            printFEid(it->fe);
                            printDETinfo(it->det);
                            #endif
                        }
                        else{
                            dummy_fe.id = it->fe.id + 1 + i;
                            new_fe_items.push_back(dummy_fe);
                            new_det_items.push_back(dummy_det);
                            #if DEBUG_PRINT_DUMMY_MAKER
                            printFEid(dummy_fe);
                            printDETinfo(dummy_det);
                            #endif
                        }
                    } // done(int i=0; i<gap; ++i)
                } // endif(gap < maxFillFactor)
                else{
                    // insert dummy entry
                    uint32_t gapId = sizeof(uint32_t) - (it->fe.id + 1);
                    for(int i=0; i<maxFillFactor - 1; ++i){
                        if(i == 0){ // for the original entry
                            new_fe_items.push_back(it->fe);
                            new_det_items.push_back(it->det);
                            #if DEBUG_PRINT_DUMMY_MAKER
                            printFEid(it->fe);
                            printDETinfo(it->det);
                            #endif
                        }
                        else{
                            dummy_fe.id = it->fe.id + 1 + gapId/maxFillFactor * i;
                            new_fe_items.push_back(dummy_fe);
                            new_det_items.push_back(dummy_det);
                            #if DEBUG_PRINT_DUMMY_MAKER
                            printFEid(dummy_fe);
                            printDETinfo(dummy_det);
                            #endif
                        }
                    } // done(int i=0; i<maxFillFactor - 1; ++i)
                } // endif(gap >= maxFillFactor)
            } // endif(it == fItems.begin()), else if(it != fItems.end() - 1), else
        } // for(auto it = fItems.begin(); it != fItems.end(); ++it)

        std::vector<ChannelMapSimpleItem> new_fItems;
        auto fe_it = new_fe_items.begin();
        auto det_it = new_det_items.begin();
        for(; fe_it != new_fe_items.end() && det_it != new_det_items.end(); ++fe_it, ++det_it){
            ChannelMapSimpleItem item = { *fe_it, *det_it };
            new_fItems.push_back(item);
        }
        fItems = new_fItems;
        // sort
        std::sort(fItems.begin(), fItems.end(), [](const ChannelMapSimpleItem& left, const ChannelMapSimpleItem& right) {
            return left.fe.id < right.fe.id; // checkDuplicateFEIDsの狭義弱順序がこの不等号の向きに依存している
        });
        fItemsFE.clear();
        fItemsDET.clear();
        for(const auto& item : fItems){
            fItemsFE.push_back(item.fe);
            fItemsDET.push_back(item.det);
        }
    }// void ChannelMapSimple::makeDummyEntry

    void ChannelMapSimple::makeDummyEntry2(double FillRatio) {
        uint32_t originalNumItems = fItems.size();
        uint32_t countDummyEntries = 0;
        std::vector<ChannelMapSimpleItem_FE> new_fe_items;
        std::vector<ChannelMapSimpleItem_DET> new_det_items;

        ChannelMapSimpleItem_DET dummy_det;
        ChannelMapSimpleItem_FE dummy_fe(0, 0, 0);
            // below ChannelMapSimpleItem_DET examlple
            // struct ChannelMapSimpleItem_DET {
            //     uint32_t name;// detector name in 4 char
            //     uint16_t plane;// plane name in 2 char
            //     uint8_t segment;// segment number in 8bit int (0-255)
            //     uint32_t channel;// channel name in 4 char
            // };
        dummy_det.name = parse_to32("nil");
        dummy_det.plane = parse_to16("nil");
        dummy_det.segment = 0;
        dummy_det.channel = parse_to32("nil");

        uint32_t maxFillFactor; // gap*FillRatioで可変
        uint32_t leftId, rightId, gapId;
        for(auto it = fItems.begin(); it != fItems.end(); ++it){
            if(it == fItems.begin()){
                leftId = 0;
                rightId = it->fe.id;
                gapId = it->fe.id - 0;
            } // endif(it == fItems.begin())
            else if(it != fItems.end() - 1){
                auto original_left = it;
                auto original_right = it + 1;
                leftId = original_left->fe.id;
                rightId = original_right->fe.id;
                gapId = rightId - leftId;
            } // endif(it == fItems.begin()), else if(it != fItems.end() - 1)
            else{ // last entry
                leftId = it->fe.id;
                rightId = UINT32_MAX;
                gapId = UINT32_MAX - leftId;
            } // endif(it == fItems.end() - 1)

            new_fe_items.push_back(it->fe);
            new_det_items.push_back(it->det);
            #if 0
            std::cout << "original entry added" << std::endl;
            std::cout << "\tproceeding gap: " << gapId << " between FE id " << std::hex << leftId << std::dec << " and FE id " << std::hex << rightId << std::dec << std::endl;
            std::cout << "\t" << (gapId-1)*FillRatio << " dummy entries will be added in this gap if maxFillFactor is not exceeded." << std::endl;
            #endif
            countDummyEntries++;

            if(gapId <= 1){
                continue; // no gap, no dummy entry needed
            } // endif(gap <= 1)
            else{
                // insert dummy entry
                maxFillFactor = (gapId - 1) * FillRatio;
                if(maxFillFactor < 1){
                    continue; // no dummy entry needed
                }
                for(int i=0; i < maxFillFactor - 1; ++i){
                    if(i == 0){ // for the original entry
                        continue; // already added above
                    }
                    else{
                        dummy_fe.id = leftId + gapId/maxFillFactor * i;
                        new_fe_items.push_back(dummy_fe);
                        new_det_items.push_back(dummy_det);
                        countDummyEntries++;
                        #if DEBUG_PRINT_DUMMY_MAKER
                        printFEid(dummy_fe);
                        printDETinfo(dummy_det);
                        #endif
                    }
                } // done(int i=0; i<maxFillFactor - 1; ++i)
            } // endif(gap > 1)
            if(countDummyEntries % 10000 == 0) {
                std::cout << "\tProgress: " << (countDummyEntries * 100) / (originalNumItems + countDummyEntries) << "% (" << countDummyEntries << " dummy entries added)" << std::endl;
            }
        } // for(auto it = fItems.begin(); it != fItems.end(); ++it)

        std::vector<ChannelMapSimpleItem> new_fItems;
        auto fe_it = new_fe_items.begin();
        auto det_it = new_det_items.begin();
        for(; fe_it != new_fe_items.end() && det_it != new_det_items.end(); ++fe_it, ++det_it){
            ChannelMapSimpleItem item = { *fe_it, *det_it };
            new_fItems.push_back(item);
        }
        new_fe_items.clear();
        new_det_items.clear();
        fItems = new_fItems;
        new_fItems.clear();
        // sort
        std::sort(fItems.begin(), fItems.end(), [](const ChannelMapSimpleItem& left, const ChannelMapSimpleItem& right) {
            return left.fe.id < right.fe.id; // checkDuplicateFEIDsの狭義弱順序がこの不等号の向きに依存している
        });
        fItemsFE.clear();
        fItemsDET.clear();
        for(const auto& item : fItems){
            fItemsFE.push_back(item.fe);
            fItemsDET.push_back(item.det);
        }
    }// void ChannelMapSimple::makeDummyEntry2

    uint32_t ChannelMapSimple::fileoutAllItems(const std::string& filename) {
        std::ofstream outfile(filename);
        if(!outfile.is_open()) {
            std::cerr << "failed to open file for output: " << filename << std::endl;
            return 0;
        }
        int progressInterval = fItems.size() / 10; // 10%ごとに進捗を表示
        uint32_t processedLines = 0;
        #if 0 // raw format output
        for(const auto& item : fItems) {
            outfile << "0x" << std::hex << std::setw(8) << std::setfill('0') << item.fe.id << std::dec
                    << " " << std::hex << std::setw(8) << std::setfill('0') << item.det.name << std::dec
                    << " " << std::hex << std::setw(4) << std::setfill('0') << item.det.plane << std::dec
                    << " " << static_cast<uint32_t>(item.det.segment)
                    << " " << std::hex << std::setw(8) << std::setfill('0') << item.det.channel << std::dec
                    << "\n";
            if(processedLines % progressInterval == 0) {
                std::cout << "Progress: " << (processedLines * 100) / fItems.size() << "% (" << processedLines << "/" << fItems.size() << " items processed)" << std::endl;
            }
            processedLines++;
        }
        #endif
        #if 1 // char format output
        for(const auto& item : fItems) {
            outfile << "FE id: 0x" << std::hex << std::setw(8) << std::setfill('0') << item.fe.id << std::dec;
            outfile << "  DET name: " << std::dec
                    << " (char: " << static_cast<char>((item.det.name >> 24) & 0xFF)
                    << static_cast<char>((item.det.name >> 16) & 0xFF)
                    << static_cast<char>((item.det.name >> 8) & 0xFF)
                    << static_cast<char>(item.det.name & 0xFF)
                    << "),";
            outfile << "  plane: " << std::dec
                    << " (char: " << static_cast<char>((item.det.plane >> 8) & 0xFF)
                    << static_cast<char>(item.det.plane & 0xFF)
                    << "),";
            outfile << "  segment: " << static_cast<uint32_t>(item.det.segment) << ",";
            outfile << "  channel: " << std::dec
                    << " (char: " << static_cast<char>((item.det.channel >> 24) & 0xFF)
                    << static_cast<char>((item.det.channel >> 16) & 0xFF)
                    << static_cast<char>((item.det.channel >> 8) & 0xFF)
                    << static_cast<char>(item.det.channel & 0xFF)
                    << ")" << std::endl;
            if(processedLines % progressInterval == 0) {
                std::cout << "Progress: " << (processedLines * 100) / fItems.size() << "% (" << processedLines << "/" << fItems.size() << " items processed)" << std::endl;
            }
            processedLines++;
        }
        #endif
        #if 0
        // fe only
        for(const auto& item : fItems) {
            outfile << "0x" << std::hex << std::setw(8) << std::setfill('0') << item.fe.id << std::dec << "\n";
            if(processedLines % progressInterval == 0) {
                std::cout << "Progress: " << (processedLines * 100) / fItems.size() << "% (" << processedLines << "/" << fItems.size() << " items processed)" << std::endl;
            }
            processedLines++;
        }
        #endif
        outfile.close();
        return fItems.size();
    }// uint32_t ChannelMapSimple::fileoutAllItems
}// namespace chmap