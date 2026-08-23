#ifndef DICTIONARY_HPP_
#define DICTIONARY_HPP_

#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace chmap::dictionary {
    class NameIndexDictionary {
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
            std::vector<std::pair<std::string, uint8_t>> forward_dictionary; // string to index
            std::vector<std::string> inverse_dictionary; // index to string
            std::vector<std::string> names_string; // temporary storage for newWord() before buildDictionary()
    };

    uint8_t queryIndex_name(const std::string_view& name);
    uint8_t queryIndex_plane(const std::string_view& plane);
    uint8_t queryIndex_readout_channel(const std::string_view& RO_channel);

    inline constexpr std::array<std::string_view, 20> name_dictionary = {
        "UTOF", "DTOF", "LTOF", "T0  ", "T0RF", "T1  ", "ALCH", "BFTR", "BHT ", "BFT ", "SFT ", "BDC ", "KLDC", "LEFT", "RIGT", "TOP ", "BOTM", "UPST", "DOST", "NIL "
    };
/*
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
            {"nil", "NIL "}
*/
    inline constexpr std::array<std::string_view, 7> plane_dictionary = {
        "X ", "U ", "V ", "XP", "UP", "VP", "NI"
    };
/*
            {"X", "X "},
            {"U", "U "},
            {"V", "V "},
            {"Xp", "XP"}, // for x prime
            {"Up", "UP"},
            {"Vp", "VP"},
            {"nil", "NI"} 
*/
    inline constexpr std::array<std::string_view, 8> readout_channel_dictionary = {"    ", "LEFT", "RIGT", "TOP ", "BOTM", "UPST", "DOST", "NI  "};
/*
            {"left", "LEFT"},
            {"right", "RIGT"},
            {"top", "TOP "},
            {"bottom", "BOTM"},
            {"upstream", "UPST"},
            {"downstream", "DOST"},
*/
} // namespace chmap::dictionary


#endif // DICTIONARY_HPP_