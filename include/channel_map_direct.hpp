#ifndef CHANNEL_MAP_DIRECT_HPP_
#define CHANNEL_MAP_DIRECT_HPP_

#include <vector>
#include <string>
#include <filesystem>
#include <unordered_map> // only use in initialize, not in search
#include <variant>
#include "channel_map_simple_item.hpp"
#include "channel_tuple.hpp"
#include "element.hpp"

namespace chmap {

    class ChannelMapDirect {
        public:
            static ChannelMapDirect& get_instance();
            ~ChannelMapDirect();

            /*
            csvにあるFront-Endのキーの範囲を取得した上で、その範囲をカバーするサイズのvectorを用意し、Front-Endのキーをインデックスとして直接アクセスできるようにする。
            */
            double initialize(const std::string& file_path); // キーの充填率を返す
            std::vector<ChannelMapSimpleItem> fItems;
            std::unordered_map<std::string, uint32_t> mapdata_string_simplify_map32;
            std::unordered_map<std::string, uint16_t> mapdata_string_simplify_map16;
            std::vector<ChannelMapSimpleItem_DET> fItemsDET_direct; // fe.idをインデックスとするvector
            std::vector<ChannelMapSimpleItem_FE> fItemsFE; // 実在するfe item
            std::vector<ChannelMapSimpleItem_DET> fItemsDET; // 実在するdet item

            ChannelMapSimpleItem_DET* getDETItem(uint8_t ip3rd, uint8_t ip4th, uint8_t ch);
            void printAllItemsFE();
            void printAllItemsDET();
            void checkDuplicateFEIDs();
            void checkDuplicateFEIDs_summary();
            void printFEid(ChannelMapSimpleItem_FE fe_item);
            void printDETinfo(ChannelMapSimpleItem_DET det_item);
            int getNumberOfChannels() const { return fItems.size(); }
            // void makeDummyEntry(uint32_t maxFillFactor = 100); // ソート済みfItems.fe.idの各要素間を最大maxFillFactor個のダミーエントリで等間隔に埋める。
            // void makeDummyEntry2(double FillRatio = 0.2); // ソート済みfItems.fe.idの各要素間をFillRatioの割合のダミーエントリで等間隔に埋める。
            // uint32_t fileoutAllItems(const std::string& output_dir);

            ChannelMapDirect(const ChannelMapDirect&) = delete; // prevent copy constructor
            ChannelMapDirect& operator=(const ChannelMapDirect&) = delete; // prevent copy assignment
        private:
            uint32_t minId; // for ChannelMapDirect
            uint32_t maxId; // for ChannelMapDirect
            uint32_t sizeId; // for ChannelMapDirect

            std::vector<std::string> split_line(const std::string& line, char delimiter = ',');
            std::vector<std::string> m_header, m_element_type, m_unique_types;
            ChannelMapSimpleItem makeSimpleItem(const std::vector<std::string>& tokens);
            std::size_t getFERank(uint8_t ip3rd, uint8_t ip4th, uint8_t ch);
            void simplify_detector_names();
            uint32_t four_char_to_uint32(char a, char b, char c, char d);
            uint16_t four_char_to_uint16(char a, char b);
            bool isTokenNumeric(const std::string& token);
            uint32_t parse_to32(const std::string& token);
            uint16_t parse_to16(const std::string& token);
            uint8_t parse_to8(const std::string& token);
            // void printFEid(ChannelMapSimpleItem_FE fe_item);
            // void printDETinfo(ChannelMapSimpleItem_DET det_item);

            ChannelMapDirect() = default; // private default constructor


    };// class ChannelMapDirect
}// namespace chmap

#endif // CHANNEL_MAP_DIRECT_HPP_