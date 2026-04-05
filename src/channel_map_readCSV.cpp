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
#define DEBUG_READCSV 0
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
    void ChannelMapDopeness::scanNamesForDictionary(const std::string& file_path) {
        // called after loading header, before loading mapdata lines
        // data to be assumed "fe.id, fe.channel, fe.data, detector.id, detector.plane, detector.segment, detector.channel, detector.readout, detector.data"
        int fe_magic = 3; // id, channel, data
        int fe_count = 0;
        int det_magic = 6; // id, plane, segment, channel, readout, data
        int det_count = 0;
        std::ifstream file_(file_path);

        std::string line;
        // skip csv header
        if (std::getline(file_, line)) {}
        // scan mapdata lines for collecting unique strings for dictionary
        while(std::getline(file_, line)){
            if(line.back() == '\r'){ // for Windwos-style line ending
                #if DEBUG_PRINT
                std::cout << "found Windows-style line ending" << std::endl;
                #endif
                line.pop_back();
            }
            auto tokens = split_line(line);
            if (tokens.size() != m_header.size()) {
                std::cerr << "bad file format : " << line << std::endl;
                continue;
            }

            for(int i=0; i<tokens.size(); i++){
                if(m_element_type[i] == "fe"){
                    fe_count++;
                }
                else if(m_element_type[i] == "detector"){
                    if(det_count == 0){
                        detname_dictionary.newWord(tokens[i]);
                    }else if(det_count == 1){
                        plane_dictionary.newWord(tokens[i]);
                    }else if(det_count == 4){
                        readout_channel_dictionary.newWord(tokens[i]);
                    }
                    det_count++;
                }
            }
            fe_count = 0;
            det_count = 0;
        } // while std::getline(file_, line) for scanning mapdata lines for collecting unique strings for dictionary
    } // void ChannelMapDopeness::scanNamesForDictionary

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
            // load header, as template, assuming "fe.id, fe.channel, fe.data, detector.id, detector.plane, detector.segment, detector.channel, detector.readout, detector.data"
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
            #if DEBUG_READCSV
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

        scanNamesForDictionary(file_path);
        ChannelMapDopeness::get_instance().detname_dictionary.sortWords();
        ChannelMapDopeness::get_instance().detname_dictionary.buildDictionary();
        ChannelMapDopeness::get_instance().plane_dictionary.sortWords();
        ChannelMapDopeness::get_instance().plane_dictionary.buildDictionary();
        ChannelMapDopeness::get_instance().readout_channel_dictionary.sortWords();
        ChannelMapDopeness::get_instance().readout_channel_dictionary.buildDictionary();

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
            #if DEBUG_READCSV && 0
            std::cout << "split tokens: ";
            for(int i=0; i<tokens.size(); ++i){
                std::cout << tokens[i] << ", ";
            }
            std::cout << std::endl;
            #endif
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

    } // void ChannelMapDopeness::readCSV

    void ChannelMapDopeness::removeBOM(std::string& str) {
        const std::string UTF8_BOM = "\xEF\xBB\xBF";
        if (str.compare(0, UTF8_BOM.size(), UTF8_BOM) == 0) {
            str.erase(0, UTF8_BOM.size());
            #if DEBUG_PRINT
            std::cout << "BOM removed from string: " << str << std::endl;
            #endif
        }
    } // void ChannelMapDopeness::removeBOM

    std::vector<std::string> ChannelMapDopeness::split_line(const std::string& line, char delimiter) {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream iss(line);
        while (std::getline(iss, token, delimiter)) {
            removeBOM(token);
            tokens.push_back(token);
            #if DEBUG_READCSV && 0
            std::cout << "  split token: " << token << std::endl;
            #endif
        }
        return tokens;
    }// std::vector<std::string> ChannelMapDopeness::split_line

    ChannelMapSimpleItem_FE ChannelMapDopeness::makeFEItem(const std::vector<std::string>& tokens) {
        uint64_t fe_ip_full;
        uint16_t fe_ip_3rd_4th;
        uint8_t fe_channel;
        int fe_magic = 3; // id, channel, data
        int fe_count = 0;
        for(int i=0; i<tokens.size(); i++){
            if(fe_count == 0){
                fe_ip_full = static_cast<uint64_t>(std::stoull(tokens[i], nullptr, 0));
                fe_ip_3rd_4th = parse_to16(
                    std::to_string( (fe_ip_full) & 0xFFFF )
                );
            }
            else if(fe_count == 1){
                fe_channel = parse_to8(tokens[i]);
            }
            fe_count++;
        }
        return ChannelMapSimpleItem_FE(fe_ip_3rd_4th >> 8, fe_ip_3rd_4th & 0xFF, fe_channel);
    } // ChannelMapSimpleItem_FE ChannelMapDopeness::makeFEItem

    ChannelMapSimpleItem_DET ChannelMapDopeness::makeDETItem(const std::vector<std::string>& tokens) {
        uint8_t det_name_idx;
        uint8_t det_plane_idx;
        uint8_t det_segment;
        uint16_t det_channel_number;
        uint8_t det_readout_channel_idx;
        std::string det_name_str;
        std::string det_plane_str;
        std::string det_segment_str;
        std::string det_channel_number_str;
        std::string det_readout_channel_str;

        int det_magic = 6; // id, plane, segment, channel, readout, data
        int det_count = 0;
        for(int i=0; i<tokens.size(); i++){
            if(det_count == 0){
                det_name_str = tokens[i];
                if(!detname_dictionary.getIndex(det_name_str, det_name_idx)){
                    std::cerr << "failed to get index for detector name: " << det_name_str << std::endl;
                }
            }else if(det_count == 1){
                det_plane_str = tokens[i];
                if(!plane_dictionary.getIndex(det_plane_str, det_plane_idx)){
                    std::cerr << "failed to get index for detector plane: " << det_plane_str << std::endl;
                }
            }else if(det_count == 2){
                det_segment = parse_to8(tokens[i]);
            }else if(det_count == 3){
                det_channel_number_str = tokens[i];
                det_channel_number = parse_to32(det_channel_number_str);
            }else if(det_count == 4){
                det_readout_channel_str = tokens[i];
                if(!readout_channel_dictionary.getIndex(det_readout_channel_str, det_readout_channel_idx)){
                    std::cerr << "failed to get index for detector readout channel: " << det_readout_channel_str << std::endl;
                }
            }
            det_count++;
        }
        return ChannelMapSimpleItem_DET(det_name_idx, det_plane_idx, det_segment, det_channel_number, det_readout_channel_idx);
    } // ChannelMapSimpleItem_DET ChannelMapDopeness::makeDETItem

    ChannelMapSimpleItem ChannelMapDopeness::makeSimpleItem(const std::vector<std::string>& tokens) {
        int len_tokens = tokens.size();
        uint64_t fe_ip_full;
        uint16_t fe_ip_3rd_4th;
        uint8_t fe_channel;
        uint8_t det_name_idx;
        uint8_t det_plane_idx;
        uint8_t det_segment;
        uint16_t det_channel_number;
        uint8_t det_readout_channel_idx;
        std::string det_name_str;
        std::string det_plane_str;
        std::string det_segment_str;
        std::string det_channel_number_str;
        std::string det_readout_channel_str;

        int fe_magic = 3; // id, channel, data
        int fe_count = 0;
        int det_magic = 6; // id, plane, segment, channel, readout, data
        int det_count = 0;
        #if DEBUG_PRINT
        std::cout << "making ChannelMapSimpleItem from tokens:" << std::endl;
        for(const auto& t : tokens){
            std::cout << "  " << t << std::endl;
        }
        #endif
        std::vector<std::string> fe_tokens;
        std::vector<std::string> det_tokens;
        for(int i=0; i<len_tokens; ++i) {
            #if DEBUG_READCSV
            std::cout << "processing token[" << i << "]: " << tokens[i] << " with element type: \"" << m_element_type[i] << "\"" << std::endl;
            #endif
            if(m_element_type[i] == "fe"){
                fe_tokens.push_back(tokens[i]);
                #if DEBUG_READCSV
                std::cout << "  identified FE token tokens[" << i << "]: " << tokens[i] << std::endl;
                #endif
                fe_count++;
            }
            else if(m_element_type[i] == "detector"){
                det_tokens.push_back(tokens[i]);
                #if DEBUG_READCSV
                std::cout << "  identified DET token tokens[" << i << "]: " << tokens[i] << std::endl;
                #endif
                det_count++;
            }
        }
        #if DEBUG_READCSV
        for(int i=0; i<fe_tokens.size(); ++i){
            std::cout << "  FE token: " << fe_tokens[i] << std::endl;
        }
        for(int i=0; i<det_tokens.size(); ++i){
            std::cout << "  DET token: " << det_tokens[i] << std::endl;
        }
        #endif
        ChannelMapSimpleItem_FE fe_item = makeFEItem(fe_tokens);
        ChannelMapSimpleItem_DET det_item = makeDETItem(det_tokens);


        // for(int i=0; i<len_tokens; ++i) {
        //     std::string type = m_element_type[i];
        //     if(type == "fe") {
        //         // parse front-end related tokens
        //         if(fe_count == 0) {
        //             #if DEBUG_PRINT
        //             std::cout << "parsing token(fe_count == 0): " << tokens[i] << " (string)" << std::endl;
        //             std::cout << "This token is interpreted as full FE IP address in uint64_t format " << std::hex << std::stoull(tokens[i], nullptr, 0) << std::dec << std::endl;
        //             #endif
        //             fe_ip_full = static_cast<uint64_t>(std::stoull(tokens[i], nullptr, 0));
        //             fe_ip_3rd_4th = parse_to16(
        //                 std::to_string( (fe_ip_full) & 0xFFFF )
        //             );
        //         } else if(fe_count == 1) {
        //             #if DEBUG_PRINT
        //             std::cout << "parsing token(fe_count == 1): " << tokens[i] << " (string)" << std::endl;
        //             std::cout << "This token is interpreted as FE channel in uint8_t format " << std::stoul(tokens[i], nullptr, 0) << std::endl;
        //             #endif
        //             fe_channel = parse_to8(tokens[i]);
        //         }
        //         fe_count++;
        //     } else if(type == "detector") {
        //         // parse detector related tokens
        //         if(det_count == 0) {
        //             det_name_str = tokens[i];
        //             if(!detname_dictionary.getIndex(det_name_str, det_name_idx)){
        //                 std::cerr << "failed to get index for detector name: " << det_name_str << std::endl;
        //             }
        //         } else if(det_count == 1) {
        //             det_plane_str = tokens[i];
        //             if(!plane_dictionary.getIndex(det_plane_str, det_plane_idx)){
        //                 std::cerr << "failed to get index for detector plane: " << det_plane_str << std::endl;
        //             }
        //         } else if(det_count == 2) {
        //             det_segment = parse_to8(tokens[i]);
        //         } else if(det_count == 3) {
        //             det_channel_number_str = tokens[i];
        //             det_channel_number = parse_to32(det_channel_number_str);
        //         } else if(det_count == 4) {
        //             det_readout_channel_str = tokens[i];
        //             if(!readout_channel_dictionary.getIndex(det_readout_channel_str, det_readout_channel_idx)){
        //                 std::cerr << "failed to get index for detector readout channel: " << det_readout_channel_str << std::endl;
        //             }
        //         }
        //         det_count++;
        //     } // if type is fe or detector
        // } // for loop for tokens

        // ChannelMapSimpleItem_FE fe_item(fe_ip_3rd_4th >> 8, fe_ip_3rd_4th & 0xFF, fe_channel);
        // ChannelMapSimpleItem_DET det_item(det_name_idx, det_plane_idx, det_segment, det_channel_number, det_readout_channel_idx);

        #if DEBUG_PRINT
        std::cout << "constructed ChannelMapSimpleItem_FE: id=" << std::hex << fe_item.id << std::dec << std::endl;
        std::cout << "constructed ChannelMapSimpleItem_DET: name=" << std::hex << det_item.name << std::dec
                  << ", plane=" << std::hex << det_item.plane << std::dec
                  << ", segment=" << static_cast<uint32_t>(det_item.segment)
                  << ", channel=" << std::hex << det_item.channel << std::dec << std::endl;
        #endif
        return ChannelMapSimpleItem{fe_item, det_item};
    }// ChannelMapSimpleItem ChannelMapDopeness::makeSimpleItem
}
