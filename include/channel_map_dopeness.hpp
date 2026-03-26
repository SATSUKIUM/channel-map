#ifndef CHANNEL_MAP_DOPENESS_HPP_
#define CHANNEL_MAP_DOPENESS_HPP_

#include <optional> // for optional return of getDopeKey_FE
#include <cstdint>
#include <vector>
#include <string>
#include <filesystem>
#include <unordered_map> // only use in initialize, not in DAQ search

#include "channel_map_simple_item.hpp"

#include "channel_tuple.hpp"
#include "element.hpp"

namespace chmap {

    class ChannelMapDopeness {
        public:
            static ChannelMapDopeness& get_instance();
            ~ChannelMapDopeness();

            /*
            FEのキーをdope-vectorのインデックスとすることで、バリューへは直接アクセスできるようにする
            */
            double initialize(const std::string& file_path); // キーの充填率を返す
            std::vector<ChannelMapSimpleItem> fItems;
            std::unordered_map<std::string, uint32_t> mapdata_string_simplify_map32;
            std::unordered_map<std::string, uint16_t> mapdata_string_simplify_map16;
            std::vector<ChannelMapSimpleItem_DET> fItemsDET_direct; // fe.idをインデックスとするvector
            std::vector<ChannelMapSimpleItem_FE> fItemsFE; // 実在するfe item
            std::vector<ChannelMapSimpleItem_DET> fItemsDET; // 実在するdet item

            ChannelMapSimpleItem_DET getDETItem(uint8_t ip3rd, uint8_t ip4th, uint8_t ch);
            void printAllItemsFE();
            void printAllItemsDET();
            void checkDuplicateFEIDs();
            void checkDuplicateFEIDs_summary();
            void printFEid(ChannelMapSimpleItem_FE fe_item);
            void printDETinfo(ChannelMapSimpleItem_DET det_item);
            int getNumberOfChannels() const { return fItems.size(); }
            std::optional<uint32_t> getDopeKey_FE(uint8_t ip3rd, uint8_t ip4th, uint8_t ch);

            ChannelMapDopeness(const ChannelMapDopeness&) = delete; // prevent copy constructor
            ChannelMapDopeness& operator=(const ChannelMapDopeness&) = delete; // prevent copy assignment
        private:
            /*
            FE key: 0x00FFFFFF (ip3rd: 8bit, ip4th: 8bit, ch: 8bit)
            ip3rd, ip4th, chのそれぞれのとりうる値の範囲で張られるdope-vectorの準備
            */
            uint8_t min_ip3rd, min_ip4th, min_ch;
            uint8_t sizeSpace_ip3rd, sizeSpace_ip4th, sizeSpace_ch;
            uint32_t sizeSpace_key = 0; // sizeSpace_key = sizeSpace_ip3rd * sizeSpace_ip4th * sizeSpace_ch
            uint32_t minId; // for out of range handling
            uint32_t maxId; // for out of range handling

            // for reading csv and initialization
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

            std::vector<ChannelMapSimpleItem_DET> fItemsDET_dope; // dope vectorの実体

            ChannelMapDopeness() = default; // private default constructor


    };// class ChannelMapDopeness
}// namespace chmap

#endif // CHANNEL_MAP_DOPENESS_HPP_