#ifndef CHANNELMAP_CHANNELTUPLE_H_
#define CHANNELMAP_CHANNELTUPLE_H_

#include <vector>

#include "element.hpp"

namespace chmap {

class ChannelTuple: public std::vector<element_t> {
public:
  ChannelTuple() = default;
  template<typename... Args> ChannelTuple(Args... args) {
    this->set_element(args...);
  }
  void set_title(std::size_t i, const std::string& t) {
    m_title_t2i[t] = i;
    m_title_i2t[i] = t;
  }
  const std::string& title(std::size_t i) const {
    if (m_title_i2t.find(i) != m_title_i2t.end()) {
      return m_title_i2t.at(i);
    } else {
      static std::string null_str;
      return null_str;
    }
  }
  element_t& operator [](const std::string& key) {
    return std::vector<element_t>::operator [](m_title_t2i.at(key));
  }
  const element_t& operator [](const std::string& key) const {
    return std::vector<element_t>::operator [](m_title_t2i.at(key));
  }
  element_t& at(const std::string& t) {
    return std::vector<element_t>::at(m_title_t2i.at(t));
  }
  const element_t& at(const std::string& t) const {
    return std::vector<element_t>::at(m_title_t2i.at(t));
  }
  using std::vector<element_t>::operator [];
  using std::vector<element_t>::at;

protected:
  template<typename Head, typename... Tail>
  void set_element(Head&& head, Tail&&... tail) {
    this->emplace_back(head);
    this->set_element(std::forward<Tail>(tail)...);
  }
  void set_element() {}

  std::unordered_map<std::string, std::size_t> m_title_t2i;
  std::unordered_map<std::size_t, std::string> m_title_i2t;
};

}

inline std::ostream&
operator <<(std::ostream& ost, const chmap::ChannelTuple& tup) {
  ost << "| ";
  for (int i=0, n=tup.size(); i<n; ++i) {
    ost << tup.title(i) << " " << tup[i];
    if (std::holds_alternative<chmap::number_t>(tup[i])) {
      ost << "(number) | ";
    } else {
      ost << "(string) | ";
    }
  }
  return ost;
}

namespace std {

/*
  follows the boost::hash_combined method
 */

template<>
struct hash<chmap::ChannelTuple> {
  std::size_t operator()(const chmap::ChannelTuple& t) const {
    std::size_t h = 0;
    const std::size_t golden_ratio = 0x9e3779b97f4a7c15LLU;
    for (const auto& e : t) {
      h ^= std::hash<chmap::element_t>()(e)
        + golden_ratio + (h << 12) + (h >> 4);
    }
    return h;
  }
};

template<>
struct hash<std::vector<chmap::ChannelTuple>> {
  std::size_t operator()(const std::vector<chmap::ChannelTuple>& v) const {
    std::size_t h = 0;
    const std::size_t golden_ratio = 0x9e3779b97f4a7c15LLU;
    for (const auto& t : v) {
      h ^= std::hash<chmap::ChannelTuple>()(t)
        + golden_ratio + (h << 12) + (h >> 4);
    }
    return h;
  }
};

}

#endif
