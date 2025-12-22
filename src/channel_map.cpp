#include "channel_map.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#include "debug_print.hpp"
#include "channel_tuple.hpp"
#include "stopwatch.hpp"

namespace chmap {

ChannelMap::ChannelMap()
  : m_header(),
    m_element_type(),
    m_unique_types(),
    m_a2b_map(),
    m_b2a_map()
{
}

void
ChannelMap::initialize(const std::filesystem::path& file_path) {
  if (file_path.extension() == ".csv") {
    initialize_from_csv(file_path.string());
  } else {
    error << "anything other than 'csv' is currently unsupported" << std::endl;
  }
}

void
ChannelMap::initialize_from_csv(const std::string& file_path) {
  Stopwatch stopwatch;

  std::ifstream file(file_path);
  if (!file.is_open()) {
    error << "file open fail : " << file_path << std::endl;
    std::exit(1);
  }

  std::string line;
  if (std::getline(file, line)) {
    int i = 0;
    for (const auto& h : split_line(line)){
      if (std::count(m_header.begin(), m_header.end(), h) > 0) {
        error << "found duplicate header : " << h << std::endl;
      }
      m_header.push_back(h);
      auto type = split_line(h, '.')[0];
      m_element_type.push_back(type);
      if (std::find(m_unique_types.begin(), m_unique_types.end(), type)
          == m_unique_types.end()) {
        m_unique_types.push_back(type);
      }
    }
    if (m_unique_types.size() != k_number_of_tuples) {
      error << "bad file format : " << line << std::endl;
      return;
    }
  }

  while (std::getline(file, line)) {
    if(line.back() == '\r'){ // for Windwos-style line ending
      line.pop_back();
    }
    auto tokens = split_line(line);
    if (tokens.size() != m_header.size()) {
      error << "column size = " << tokens.size()
            << " does not match header size = " << m_header.size()
            << ", skip this line : " << line << std::endl;
      continue;
    }
    make_tuple(tokens);
  }
}

void
ChannelMap::make_tuple(const std::vector<std::string>& tokens) {
  std::vector<ChannelTuple> tuples;
  for (const auto& type : m_unique_types) {
    ChannelTuple tuple;
    for (int i=0, n=tokens.size(); i<n; ++i) {
      if (m_element_type[i] != type) continue;
      tuple.set_title(tuple.size(), split_line(m_header[i], '.')[1]);
      tuple.push_back(parse_element(tokens[i]));
    }
    tuples.push_back(tuple);
    debug << m_a2b_map.size() << "\t" << tuple << std::endl;
  }
  const auto& a = tuples[k_a];
  const auto& b = tuples[k_b];
  m_a2b_map[a] = b;
  m_b2a_map[b] = a;
}

std::vector<std::string>
ChannelMap::split_line(const std::string& str, char delimiter) {
  std::vector<std::string> tokens;
  std::string token;
  std::istringstream iss(str);
  while (std::getline(iss, token, delimiter)) {
    tokens.push_back(token);
  }
  return tokens;
}

}
