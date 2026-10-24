#include <chmap/channel_map_dopeness.hpp>
#include "../src/initializationDiagnostics.hpp"
#include <cassert>
#include <fstream>
#include <sstream>
#include <string>

int main(int argc, char** argv) {
    assert(argc == 2 || argc == 3);
    if (argc == 3) {
        // A tiny CSV whose inverse Cartesian product exceeds 32-bit indexing.
        std::ofstream csv(argv[1]);
        csv << "fe.id,fe.channel,fe.data,detector.id,detector.plane,detector.segment,detector.channel,detector.readout,detector.data\n";
        for (int i = 0; i < 65; ++i)
            csv << "0x0201," << i << ",0,name" << i
                << (i % 2 ? ",X,0,0,left,0\n" : ",Y,255,65535,right,0\n");
        csv.close();
        std::ostringstream warnings;
        auto* previous = std::cerr.rdbuf(warnings.rdbuf());
        bool rejected = false;
        try { chmap::ChannelMapDopeness::get_instance().initialize(argv[1], true); }
        catch (const std::overflow_error&) { rejected = true; }
        std::cerr.rdbuf(previous);
        assert(rejected);
        assert(warnings.str().find("exceeds 1 GB") != std::string::npos);
        return 0;
    }
    // Test the memory policy without allocating gigabytes.
    std::ostringstream memory;
    const uint64_t boundary = 1000000000ULL / sizeof(chmap::DETIdItem);
    chmap::detail::warnMapMemory(boundary, 0, memory);
    assert(memory.str().empty());
    chmap::detail::warnMapMemory(boundary + 1, 0, memory);
    assert(memory.str().find("exceeds 1 GB") != std::string::npos);
    memory.str("");
    chmap::detail::warnMapMemory(0, 333333334ULL, memory);
    assert(memory.str().find("inverse=") != std::string::npos);
    memory.str("");
    chmap::detail::warnMapMemory(boundary, 1, memory); // Combined total, neither alone >1GB.
    assert(memory.str().find("total=") != std::string::npos);
    memory.str("");
    chmap::detail::warnMapMemory(1, 4294967296ULL, memory);
    assert(memory.str().find(std::to_string(4294967296ULL * sizeof(chmap::FEAddrItem))) != std::string::npos);

    std::ofstream csv(argv[1]);
    csv << "fe.id,fe.channel,fe.data,detector.id,detector.plane,detector.segment,detector.channel,detector.readout,detector.data\n"
        << "0x0201,0,0,a,X,0,0,left,0\n"
        << "0x0201,0,0,a,X,0,2,left,0\n" // Duplicate FE.
        << "0x0201,2,0,a,X,0,2,left,0\n" // Duplicate DET.
        << "0x0201,2,0,a,X,0,2,left,0\n"; // Identical record: both warnings.
    csv.close();
    auto& map = chmap::ChannelMapDopeness::get_instance();
    std::ostringstream warnings;
    auto* previous = std::cerr.rdbuf(warnings.rdbuf());
    map.initialize(argv[1], false); // DET duplicates must be reported even without inverse map.
    std::cerr.rdbuf(previous);
    const auto text = warnings.str();
    assert(text.find("Duplicate FE address") != std::string::npos);
    assert(text.find("Duplicate DET identifier") != std::string::npos);
    assert(text.find("entry 1: FE=(ip3rd=2, ip4th=1, ch=0)") != std::string::npos);
    assert(text.find("entry 4:") != std::string::npos);
    assert(text.find("name=\"a\"") != std::string::npos);
    assert(text.find("channel=2") != std::string::npos);
    assert(map.getNumberOfChannels() == 4); // Warnings do not reject/deduplicate records.
    assert(map.findDET(2, 1, 0));
    assert(map.findDET(2, 1, 2));

    std::vector<chmap::ItemPair> clean{{{2, 1, 0}, *map.findDET(2, 1, 0)}};
    std::ostringstream quiet;
    chmap::detail::warnDuplicateMappings(clean, quiet);
    assert(quiet.str().empty());
}
