#ifndef DICTIONARY_HPP_
#define DICTIONARY_HPP_

#include <cstdint>
#include <string>
#include <vector>
#include <utility>

namespace chmap::dictionary {
    class NameIndexDictionary {
        public:
            /*
            usage:
            call newWord(), newWord(), ... in order, then call sortWords() once, and finally call buildDictionary() once.
            */
            void newWord(const std::string& str); // just adding new word
            void sortWords(); // sort words
            void buildDictionary(bool reserveZero = false); // assing index to each word on sorted order, and build forward and inverse dictionary
            bool StringToIndex(const std::string& str, uint8_t& idx) const; // from string to index
            bool IndexToString(uint8_t idx, std::string& str) const; // from index to which original string
        private:
            // content of dictionary (accessed only through member func)
            bool zero_reserved = false;
            std::vector<std::pair<std::string, uint8_t>> forward_dictionary; // string to index
            std::vector<std::string> inverse_dictionary; // index to string
            std::vector<std::string> names_string; // temporary storage for newWord() before buildDictionary()
    };

} // namespace chmap::dictionary

#endif // DICTIONARY_HPP_
