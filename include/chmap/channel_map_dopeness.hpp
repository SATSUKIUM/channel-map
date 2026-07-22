#ifndef CHANNEL_MAP_DOPENESS_HPP_
#define CHANNEL_MAP_DOPENESS_HPP_

#include <cstdint>
#include <vector>
#include <string>
#include <string_view>
#include <filesystem>
#include <unordered_map> // only use in initialize, not in DAQ search

#include "chmap/item.hpp"
#include "chmap/dictionary.hpp"


namespace chmap {

    class ChannelMapDopeness {
        public:
            static ChannelMapDopeness& get_instance();
            ~ChannelMapDopeness();

            /*
            FEのキーをdope-vectorのインデックスとすることで、バリューへは直接アクセスできるようにする
            */

            double initialize(const std::string& file_path, bool createInvMap = false); // 返り値はFEキーの充填率
            void initialize_InvMap();

            bool getDopeKey_FEtoDET(uint8_t ip3rd, uint8_t ip4th, uint8_t ch, uint32_t& retKey) const;
            bool getDopeKey_FEtoDET(const FEAddrItem& fe_item, uint32_t& retKey) const { // overload expression
                return getDopeKey_FEtoDET(fe_item.ip3rd, fe_item.ip4th, fe_item.ch, retKey);
            }
            bool getDopeKey_DETtoFE(uint8_t name_idx, uint8_t plane_idx, uint8_t segment, uint16_t channel_number, uint8_t readout_channel_idx, uint32_t& retKey) const;
            bool getDopeKey_DETtoFE(std::string_view det_name, std::string_view det_plane, int segment, std::string_view readout_channel, int channel_number, uint32_t& retKey) const;
            bool getDopeKey_DETtoFE(const DETIdItem& det_item, uint32_t& retKey) const { // overload expression
                return getDopeKey_DETtoFE(det_item.name, det_item.plane, det_item.segment, det_item.channel_number, det_item.readout_channel, retKey);
            }

            const DETIdItem& getDETItem(uint32_t doped_index) const;
            DETIdItem& getDETItem(uint32_t doped_index);
            const FEAddrItem& getFEIItem(uint32_t doped_index) const;
            FEAddrItem& getFEIItem(uint32_t doped_index);

            bool registerDETConfItem(std::string_view det_name, std::string_view det_plane, int segment, std::string_view readout_channel, int channel_number, DETConfItem* detconf);
            bool registerDETConfItem(const DETIdItem& det_item, DETConfItem* detconf);
            bool registerDETConfItem(uint32_t doped_index, DETConfItem* detconf);

            void printAllItemsFE();
            void printAllItemsDET();
            void printFEid(const FEAddrItem& fe_item);
            void printDETinfo(const DETIdItem& det_item);
            int getNumberOfChannels() const { return fItems.size(); }

            ChannelMapDopeness(const ChannelMapDopeness&) = delete; // prevent copy constructor
            ChannelMapDopeness& operator=(const ChannelMapDopeness&) = delete; // prevent copy assignment

            class NameIndexDictionary{
                public:
                    /*
                    usage:
                    execute newWord(), newWord(), ... in order, then execute sortWords() once, and finally execute buildDictionary() once.
                    */
                    void newWord(const std::string& str); // just adding new word
                    void sortWords(); // sort words
                    void buildDictionary(); // assing index to each word on sorted order, and build forward and inverse dictionary
                    bool getIndex(const std::string& str, uint8_t& idx) const; // from string to index
                    bool invIndex(uint8_t idx, std::string& str) const; // from index to which original string
                private:
                    // content of dictionary (accessed only through member func)
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
            DET key: 88bit = 8bit(name index) + 8bit(plane index) + 8bit(segment) + 16bit(channel number) + 8bit(readout channel index)
            */
           uint8_t min_name_idx, min_plane_idx, min_readout_channel_idx;
           uint16_t min_segment, min_channel_number;
           uint8_t max_name_idx, max_plane_idx, max_readout_channel_idx;
           uint16_t max_segment, max_channel_number;
           uint16_t sizeSpace_name_idx, sizeSpace_plane_idx, sizeSpace_readout_channel_idx;
           uint32_t sizeSpace_segment, sizeSpace_channel_number;
           uint32_t sizeSpace_DETKey = 0; // sizeSpace_DETKey = sizeSpace_name_idx * sizeSpace_plane_idx * sizeSpace_segment * sizeSpace_channel_number * sizeSpace_readout_channel_idx
           uint32_t minDETId; // for out of range handling
           uint32_t maxDETId; // for out of range handling
            
            std::vector<ChannelMapSimpleItem> fItems;
            std::vector<FEAddrItem> fItemsFE; // 実在するfe item
            std::vector<DETIdItem> fItemsDET; // 実在するdet item
            std::unordered_map<std::string, uint32_t> mapdata_string_simplify_map32;
            std::unordered_map<std::string, uint16_t> mapdata_string_simplify_map16;


            // for reading csv and initialization
            void readCSV(const std::string& file_path);
            void scanNamesForDictionary(const std::string& file_path); // just collecting unique strings and assigning index is done in this function. The actual dictionary is built in NameIndexDictionary::buildDictionary() after sorting the collected unique strings.
            void removeBOM(std::string& str); // remove BOM if exists in the beginning of the string
            std::vector<std::string> split_line(const std::string& line, char delimiter = ',');
            std::vector<std::string> m_header, m_element_type, m_unique_types;
            ChannelMapSimpleItem makeSimpleItem(const std::vector<std::string>& tokens); // to be {fe.id, fe.channel, fe.data, detector.id, detector.plane, detector.segment, detector.channel, detector.readout, detector.data}
            FEAddrItem makeFEItem(const std::vector<std::string>& tokens); // to be {0xc0a80205, 0x01, 0x00} ipfull, ch, data
            DETIdItem makeDETItem(const std::vector<std::string>& tokens); // to be {0x01, 0x01, 0x01, 0x0001, 0x01} detname_idx, plane_idx, segment, channel_number, readout_channel_idx
            void defineDictionary();
            uint32_t four_char_to_uint32(char a, char b, char c, char d);
            uint16_t two_char_to_uint16(char a, char b);
            bool isTokenNumeric(const std::string& token);
            uint32_t parse_to32(const std::string& token);
            uint16_t parse_to16(const std::string& token);
            uint8_t parse_to8(const std::string& token);
            // for print
            void printFEtoDETscan();
            void printDETtoFEscan();

            std::vector<DETIdItem> fItemsFEtoDET_dope; // dope vectorの実体
            std::vector<FEAddrItem> fItemsDETtoFE_dope; // 逆引き用のdope vectorの実体

            ChannelMapDopeness() = default; // private default constructor
    };// class ChannelMapDopeness
}// namespace chmap

#endif // CHANNEL_MAP_DOPENESS_HPP_