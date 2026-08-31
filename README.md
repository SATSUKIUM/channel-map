# channel-map

Channel mapping library between Detector and Readout FEE

Development and test environment: KEKCC Intel Xeon Gold 6230 20C40Tx2 @2.10 GHz, CentOS Linux 7.9.2009, gcc/g++ 8.3.0 (c++17 required)

## Install

Build channel-map.

```sh
cmake -S . -B .build
cmake --build .build --target install
```

Add the following to your project's CMakeLists.txt.

```sh
list(APPEND CMAKE_PREFIX_PATH /path/to/channel-map)
find_package(ChannelMap REQUIRED)
target_link_libraries(YourProject ChannelMap::ChannelMap)
```

For Makefile, the following.

```make
chmap_config	= /path/to/channel-map/bin/chmap-config
CFLAGS			+= $(shell $(chmap_config) --include)
LDFLAGS			+= $(shell $(chmap_config) --libs)
```

## Uninstall

```sh
xargs rm < .build/install_manifest.txt
rm -rf .build
```

## Parameter format

Only CSV is supported.
Write the element title of each column in the heading line.
Titles are classified into *Readout FEE* or *Detector* tuples using the first dot-delimited string.

Elements can be numbers or strings.
Prefixes such as `0xff`, `0b101`, `077`, etc. are interpreted as hexadecimal, binary, or octal numbers, respectively.
If they cannot be interpreted as numbers, they are assumed to be strings.

```test.csv
fe.id,fe.channel,fe.data,detector.id,detector.plane,detector.segment,detector.channel,detector.data
30,262,adc,170,30,0,262,0
30,263,0,170,30,0,263,0
30,264,0,170,30,0,264,0
30,265,0,170,30,0,265,0
30,266,0,170,30,0,266,0
30,267,0,170,30,0,267,0
30,268,0,170,30,0,268,0
30,269,0,170,30,0,269,0xff
30,270,0,170,30,0,270,A
30,271,0,170,30,0,271,C
```

## Usage

Initialize the singleton instance of the `ChannelMap` class
and use the `get` method to get a tuple from *FE* to *Detector* or the other way around.

`std::get<T>` returns the value if the specified type T matches the current type of `std::variant`,
otherwise it throws a `std::bad_variant_access` exception.

`std::get_if<T>` returns a pointer to the value if the specified type T
matches the current type of `std::variant`, nullptr otherwise.

```cpp
#include <channel_map.hpp>

void
your_function()
{
  auto& channel_map = chmap::ChannelMap::get_instance();
  channel_map.initialize("foo.csv");

  // std::get<T>
  {
    chmap::ChannelTuple detector(1, 2, 3, 4, 5);
    const auto& fe = channel_map.get("fe", detector);
    try {
      auto fe_id = std::get<chmap::number_t>(fe["id"]);
      auto fe_ch = std::get<chmap::number_t>(fe["channel"]);
    } catch (const std::bad_variant_access& e) {
      std::cerr << "Bad variand access : " << e << std::endl;
    }
  }

  // std::get_if<T>
  {
    const auto& fe = channel_map.get("fe", {0, "a", 3, "x"});
    if (auto value_as_pointer = std::get_if<chmap::number_t>(&fe[0])) {
      std::cout << "The value is number : *value_as_pointer << std::endl;
    } else if (auto fe_id_as_pointer = std::get_if<std::string>(&fe[0])) {
      std::cout << "The value is string : *value_as_pointer << std::endl;
    } else {
      std::cerr << "The variant does not hold number or string" << std::endl;
    }
  }
    ...
  }
}
```

See also `skeleton.cpp` for implementation details.

# class ChannelMapDopeness
## 使い方
### 全体概要
- class chmap::ChannelMapDopenessが本体
  - 関連: “items”, “dictionary”
- 設計の基本「マップからアイテムを探して取得」
  - フロントエンドのアドレス(IP, CH) —探す—> (実体)検出器の識別子(name, e.t.c.)
  - 検出器の識別子(name, e.t.c.) —探す—> (実体)フロントエンドのアドレス(IP, CH)
- 後から追加された機能: 検出器の識別子とそのdetector configurationsを対応させる
 - アルゴリズムの都合で、検出器の識別子(name, e.t.c.)から(実体)フロントエンドのアドレス(IP, CH)を探してから、 (実体)検出器の識別子(name, e.t.c.)を得る
    - (実体)検出器の識別子(name, e.t.c.)がdetector configurationsのポインタを持っている
### minimum get ready
- csvを用意する
  - フォーマット: (“FEE IP”, “CH”, “void”, “detector name”, “plane”, “segment”, “channel number”, “channel name”, “void”)
  - 1行目のみfe.id fe.channel fe.data detector.id detector.plane detector.segment detector.channel detector.readout detector.data
    - “void”は旧来のchmapの形式が残っているだけで、利用はされていない
    - “segment”と”channel number”にはstringを使えない
- ヘッダのインクルード
    - #include <chmap/channel_map_dopeness.hpp>
### map search
- キーを取得, アイテムを取得
```
uint32_t keyDETtoFE;
DETIdItem itemDETID;
bool keyFound = getDopeKey_FEtoDET(ip3rd, ip4th, ch, keyDETtoFE);
if(keyFound == true){
itemDETID = getDETIdItem(keyDETtoFE);
}
```
### 使用例
```
chmap::ChannelMapDopeness& channel_map_dopeness = chmap::ChannelMapDopeness::get_instance();
isCreateInvMap = true;
channel_map_dopeness.initialize(input_file_path, isCreateInvMap);

{
    // test t1 right channel
    uint8_t test_ip3rd_T1right = 0x02;
    uint8_t test_ip4th_T1right = 0xAA;
    uint8_t test_ch_T1right = 12;

    uint32_t dopeKey_FEtoDET;
    if(!channel_map_dopeness.getDopeKey_FEtoDET(test_ip3rd_T1right, test_ip4th_T1right, test_ch_T1right, dopeKey_FEtoDET)) {
        std::cout << "FEAddrItem for T1 right channel not found" << std::endl;
        return -1;
    }
    const chmap::DETIdItem& detitem = channel_map_dopeness.getDETIdItem(dopeKey_FEtoDET);
    detitem.decode();
}
```

## Itemについて
- 詳細は`include/chmap/item.hpp`を参照
- DETIdItemは検出器の識別子を全て整数で保持
uint8_t name for 検出器名(string)
uint8_t plane for 面名(string)
uint8_t segment はそのままセグメント番号
uint16_t channel_number はそのままチャンネル番号
uint8_t readout_channel for 読み出しチャンネル名(string)
### 辞書の成り立ち&使用例
- CSVをスキャンして、name, plane, channel_nameとしてあり得るstringの一覧を取得する(scanNamesForDictionary(filepath))
- CSVをスキャンして、各行をChannelMapSimpleItemにしてfItemsに格納する
  - makeSimpleItem(tokens)
    - その中で
    - buildFEItemFromStringTokens(tokens)
    - buildDETITemFromStringTokens(tokens)
      - すでに作ったString<—>Indexの辞書を参照しつつDETIdITemを作成

## 情報をどう渡すか: 関数オーバーロード
方法は3つ
- インデックスを知って渡す
  - どうやって知る?
- stringのまま渡す
  - 固有名詞はどこで定義されている?
- DETIdItemを渡す
```
bool getDopeKey_DETtoFE(uint8_t DetectorNameIndex, uint8_t PlaneIndex, uint8_t SegmentNumber, uint8_t ChannelNameIndex, uint16_t ChannelNumber, uint32_t& retKey) const;
bool getDopeKey_DETtoFE(std::string_view DetectorName, std::string_view PlaneName, int SegmentNumber, std::string_view ChannelName, int ChannelNumber, uint32_t& retKey) const;
bool getDopeKey_DETtoFE(const DETIdItem& det_item, uint32_t& retKey) const {
                return getDopeKey_DETtoFE(det_item.name, det_item.plane, det_item.segment, det_item.readout_channel, det_item.channel_number, retKey);
            }
```
### どうやって知る?
- chmap::dictionary::NameIndexDictionaryというクラスがある
- ChannelMapDopenessが検出器名、面名、チャンネル名について保持している
```
dictionary::NameIndexDictionary detname_dictionary;
dictionary::NameIndexDictionary plane_dictionary;
dictionary::NameIndexDictionary readout_channel_dictionary;
```
- Stringに対応するIndexを探してくれたり、Indexに対応するStringを教えてくれたりする
```
namespace chmap::dictionary {
    class NameIndexDictionary {
        public:
            /*
            usage:
            call newWord(), newWord(), ... in order, then call sortWords() once, and finally call buildDictionary() once.
            */
            void newWord(const std::string& str); // just adding new word
            void sortWords(); // sort words
            void buildDictionary(); // assing index to each word on sorted order, and build forward and inverse dictionary
            bool StringToIndex(const std::string& str, uint8_t& idx) const; // from string to index
            bool IndexToString(uint8_t idx, std::string& str) const; // from index to which original string
        private:
            // content of dictionary (accessed only through member func)
            std::vector<std::pair<std::string, uint8_t>> forward_dictionary; // string to index
            std::vector<std::string> inverse_dictionary; // index to string
            std::vector<std::string> names_string; // temporary storage for newWord() before buildDictionary()
    };
```
