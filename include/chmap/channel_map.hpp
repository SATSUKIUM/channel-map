#ifndef CHANNELMAP_CHANNELMAP_H_
#define CHANNELMAP_CHANNELMAP_H_

#include <filesystem>
#include <map>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "channel_tuple.hpp"
#include "debug_print.hpp"
#include "element.hpp"
#include "stopwatch.hpp"

namespace chmap {

class ChannelMap {
public:
  static ChannelMap& get_instance();
  ~ChannelMap() {}

  void initialize(const std::filesystem::path& file_path);
  void initialize_from_csv(const std::string& file_path);
  const ChannelTuple& get(const std::string& target,
                          const ChannelTuple& tuple) const {
    if (m_unique_types[k_a] == target) {
      return b2a(tuple);
    }
    else if (m_unique_types[k_b] == target) {
      return a2b(tuple);
    }
    else return k_null_tuple;
  }
  const std::unordered_map<ChannelTuple, ChannelTuple>
  get(const std::string& target) const {
    if (m_unique_types[k_a] == target) {
      return m_b2a_map;
    }
    else if (m_unique_types[k_b] == target) {
      return m_a2b_map;
    }
    else {
      static std::unordered_map<ChannelTuple, ChannelTuple> null_map;
      return null_map;
    }
  }
  // const ChannelTuple& get(int index, const ChannelTuple& tuple) const {
  //   if (index == k_a) {
  //     return b2a(tuple);
  //   }
  //   else if (index == k_b) {
  //     return a2b(tuple);
  //   }
  //   else return k_null_tuple;
  // }

private:
  ChannelMap();
  ChannelMap(const ChannelMap&) = delete;
  ChannelMap& operator =(const ChannelMap&) = delete;

  const ChannelTuple& a2b(const ChannelTuple& a) const {
    auto itr = m_a2b_map.find(a);
    if (itr != m_a2b_map.end()) {
      return itr->second;
    } else {
      warning << "key not found : " << a << std::endl;
      return k_null_tuple;
    }
  }
  const ChannelTuple& b2a(const ChannelTuple& b) const {
    auto itr = m_b2a_map.find(b);
    if (itr != m_b2a_map.end()) {
      return itr->second;
    } else {
      warning << "key not found : " << b << std::endl;
      return k_null_tuple;
    }
  }
  void make_tuple(const std::vector<std::string>& tokens);
  std::vector<std::string>
  split_line(const std::string& str, char delimiter=',');

  static inline const ChannelTuple k_null_tuple;
  enum ETupleIndex { k_a, k_b, k_number_of_tuples };

  std::vector<std::string> m_header;
  std::vector<std::string> m_element_type;
  std::vector<std::string> m_unique_types;
  std::unordered_map<ChannelTuple, ChannelTuple> m_a2b_map;
  std::unordered_map<ChannelTuple, ChannelTuple> m_b2a_map;
};

inline ChannelMap&
ChannelMap::get_instance()
{
  static ChannelMap instance;
  return instance;
}

}
#endif
