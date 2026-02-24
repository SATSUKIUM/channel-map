#include <cstdlib>
#include <iostream>

#include <channel_map.hpp>

enum EArgs {
  kProcess, kInputCSV, kNArgs
};

int main(int argc, char* argv[]) {
  if (argc < kNArgs) {
    std::cout << "Usage: " << argv[kProcess] << " [InputCSV]" << std::endl;
    return EXIT_SUCCESS;
  }

  std::string input_csv(argv[kInputCSV]);

  auto& channel_map = chmap::ChannelMap::get_instance();
  channel_map.initialize(input_csv);

  /*
    test use for mapdata.csv
  */
  if (input_csv == "mapdata.csv") {
    {
      chmap::ChannelTuple det("utof", 0, 0, "left", 0); // Left readout of UTOF
      const auto& fe = channel_map.get("fe", det);
      std::cout << "det = " << det << std::endl
                << "-> fe = " << fe << std::endl;

      for (const auto& e : fe) {
        std::cout << "element : " << e << std::endl;
      }

      std::cout << "det = " << std::get<std::string>(det[0]) << ", " << std::get<chmap::number_t>(det[1]) << ", " << std::get<chmap::number_t>(det[2]) << ", " << std::get<std::string>(det[3]) << ", " << std::get<chmap::number_t>(det[4]) << std::endl;


      /*
        std::get<T> returns the value if the specified type T
        matches the current type of std::variant,
        otherwise it throws a std::bad_variant_access exception.
      */
      try {
        std::cout << std::string(80, '=') << std::endl
                  << "fe[0] : " << std::get<chmap::number_t>(fe[0])
                  << " fe[id] : " << std::get<chmap::number_t>(fe["id"])
                  << " fe[1] : " << std::get<chmap::number_t>(fe[1])
                  << " fe[channel] : " << std::get<chmap::number_t>(fe["channel"])
                  // << " fe[2] : " << std::get<std::string>(fe[2])
                  // << " fe[data] : " << std::get<std::string>(fe["data"])
                  << " fe[2] : " << std::get<chmap::number_t>(fe[2])
                  << " fe[data] : " << std::get<chmap::number_t>(fe["data"])
                  << std::endl;
        auto value = std::get<chmap::number_t>(fe.at(0));
        std::cout << "fe[0] : " << value << std::endl;
      } catch (const std::bad_variant_access& e) {
        std::cerr << "Bad variant access : " << e.what() << std::endl;
      }

      /*
        std::get_if<T> returns a pointer to the value if the specified
        type T matches the current type of std::variant, nullptr otherwise.
      */
      std::cout << std::string(80, '=') << std::endl
          << "fe[0] : " << *std::get_if<chmap::number_t>(&fe.at(0))
          << " fe[1] : " << *std::get_if<chmap::number_t>(&fe.at(1))
          << " fe[2] : " << *std::get_if<chmap::number_t>(&fe.at(2))
          << std::endl;

      if (auto value = std::get_if<chmap::number_t>(&fe.at(2))) {
        std::cout << "The value is number : " << *value << std::endl;
      } else if (auto value = std::get_if<std::string>(&fe.at(2))) {
        std::cout << "The value is string : " << *value << std::endl;
      } else {
        std::cerr << "The variant does not hold number or string" << std::endl;
      }
    }

    /*
      If the key is missing, a null tuple is returned.
     */
    {
      chmap::ChannelTuple det(0, 1, 2, 3);
      const auto& fe = channel_map.get("fe", det);
      std::cout << "fe = " << fe << std::endl;
    }

    {
      const auto& fe = channel_map.get("fe", {0, "1", 2, "3"});
      std::cout << "fe = " << fe << std::endl;
    }
  }

  return EXIT_SUCCESS;
}
