#ifndef CHANNEL_MAP_DOPENESS_HPP_
#define CHANNEL_MAP_DOPENESS_HPP_

#include <cstdint>
#include <vector>
#include <string>
#include <filesystem>
#include <unordered_map> // only use in initialize, not in DAQ search

#include "channel_map_simple_item.hpp"
#include "channel_map_dictionary.hpp"

// #include "channel_tuple.hpp"
// #include "element.hpp"

namespace chmap {

    class ChannelMapDopeness {
        public:
            static ChannelMapDopeness& get_instance();
            ~ChannelMapDopeness();

            /*
            FEのキーをdope-vectorのインデックスとすることで、バリューへは直接アクセスできるようにする
            */
            /*
            DET->FEの「逆引き」を実装してみる
            */
            double initialize(const std::string& file_path, bool createInvMap = false); // 返り値はFEキーの充填率
            void initialize_InvMap();

            ChannelMapSimpleItem_DET getDETItem(uint32_t doped_index);
            ChannelMapSimpleItem_FE getFEIItem(uint32_t rank);
            void printAllItemsFE();
            void printAllItemsDET();
            void checkDuplicateFEIDs();
            void checkDuplicateFEIDs_summary();
            void printFEid(ChannelMapSimpleItem_FE fe_item);
            void printDETinfo(ChannelMapSimpleItem_DET det_item);
            int getNumberOfChannels() const { return fItems.size(); }

            ChannelMapDopeness(const ChannelMapDopeness&) = delete; // prevent copy constructor
            ChannelMapDopeness& operator=(const ChannelMapDopeness&) = delete; // prevent copy assignment

            bool getDopeKey_FEtoDET(uint8_t ip3rd, uint8_t ip4th, uint8_t ch, uint32_t& retKey) const;
            uint32_t unchecked_getDopeKey_FE(uint8_t ip3rd, uint8_t ip4th, uint8_t ch) const;
            bool getRank_DETtoFE(uint32_t name, uint16_t plane, uint8_t segment, uint32_t channel, uint32_t& retKey) const;
            uint32_t unchecked_getRank_DETtoFE(uint32_t name, uint16_t plane, uint8_t segment, uint32_t channel) const;

            class NameIndexDictionary{
                public:
                    /*
                    execute newWord(), newWord(), ... in order, then execute sortWords() once, and finally execute buildDictionary() once.
                    */
                    void newWord(const std::string& str); // just adding new word
                    void sortWords(); // sort words
                    void buildDictionary(); // assing index to each word on sorted order, and build forward and inverse dictionary
                    bool getIndex(const std::string& str, uint8_t& idx) const;
                    bool invIndex(uint8_t idx, std::string& str) const;
                private:
                    std::vector<std::pair<std::string, uint8_t>> forward_d; // string to index
                    std::vector<std::string> inverse_d; // index to string
                    std::vector<std::string> names_str;
            };
            NameIndexDictionary detname_dictionary;
            NameIndexDictionary plane_dictionary;
            NameIndexDictionary readout_channel_dictionary;
        private:

            /*
            FE key: 0x00FFFFFF (ip3rd: 8bit, ip4th: 8bit, ch: 8bit)
            ip3rd, ip4th, chのそれぞれのとりうる値の範囲で張られるdope-vectorの準備
            */
            uint8_t min_ip3rd, min_ip4th, min_ch;
            uint8_t max_ip3rd, max_ip4th, max_ch;
            uint16_t sizeSpace_ip3rd, sizeSpace_ip4th, sizeSpace_ch; // 8bitではオーバーフローしちゃったので、大は小をウンヌン
            uint32_t sizeSpace_FEKey = 0; // sizeSpace_key = sizeSpace_ip3rd * sizeSpace_ip4th * sizeSpace_ch
            uint32_t minFEId; // for out of range handling
            uint32_t maxFEId; // for out of range handling

            /*
            DET key: 88bit = 32bit(name) + 16bit(plane) + 8bit(segment) + 32bit(channel)
            */
            
            std::vector<ChannelMapSimpleItem> fItems;
            std::vector<ChannelMapSimpleItem_FE> fItemsFE; // 実在するfe item
            std::vector<ChannelMapSimpleItem_DET> fItemsDET; // 実在するdet item
            std::unordered_map<std::string, uint32_t> mapdata_string_simplify_map32;
            std::unordered_map<std::string, uint16_t> mapdata_string_simplify_map16;


            // for reading csv and initialization
            void readCSV(const std::string& file_path);
            std::vector<std::string> split_line(const std::string& line, char delimiter = ',');
            std::vector<std::string> m_header, m_element_type, m_unique_types;
            ChannelMapSimpleItem makeSimpleItem(const std::vector<std::string>& tokens);
            void defineDictionary();
            uint32_t four_char_to_uint32(char a, char b, char c, char d);
            uint16_t two_char_to_uint16(char a, char b);
            bool isTokenNumeric(const std::string& token);
            uint32_t parse_to32(const std::string& token);
            uint16_t parse_to16(const std::string& token);
            uint8_t parse_to8(const std::string& token);

            std::vector<ChannelMapSimpleItem_DET> fItemsFEtoDET_dope; // dope vectorの実体
            std::vector<ChannelMapSimpleItem_FE> fItemsDETtoFE_binary; // sorted by DET operator<, for binary search

            ChannelMapDopeness() = default; // private default constructor
    };// class ChannelMapDopeness
}// namespace chmap

#endif // CHANNEL_MAP_DOPENESS_HPP_