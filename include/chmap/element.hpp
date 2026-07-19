#ifndef CHANNELMAP_ELEMENT_H_
#define CHANNELMAP_ELEMENT_H_

#include <charconv>
#include <fstream>
#include <iostream>
#include <map>
#include <variant>
#include <vector>
#include <string>
#include <string_view>

namespace chmap {

using number_t = int64_t;
using element_t = std::variant<number_t, std::string>;

inline element_t parse_element(const std::string& str) {
  std::size_t idx = 0;
  int base = 10;

  if (str.size() > 2 && str[0] == '0') {
    if (str[1] == 'x' || str[1] == 'X') {
      base = 16;
      idx = 2;
    } else if (str[1] == 'b' || str[1] == 'B') {
      base = 2;
      idx = 2;
    } else if (str[1] >= '0' && str[1] <= '7') {
      base = 8;
      idx = 1;
    }
  } else if (str.size() > 1 && str[0] == '0') {
    base = 8;
    idx = 1;
  }

  number_t result = 0;
  auto [p, ec] = std::from_chars(str.data() + idx, str.data() + str.size(),
                                 result, base);
  if (ec != std::errc()) {
    return str;
  }
  return result;
}

}

inline std::ostream&
operator <<(std::ostream& ost, const chmap::element_t& var) {
  std::visit([&ost](const auto& value) {
    ost << value;
  }, var);
  return ost;
}

#endif
