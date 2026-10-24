// Compare equivalent FE lookups including item consumption. Run with a 9-column CSV.
// This is a local microbenchmark, not a DAQ throughput measurement.
#include <chmap/channel_map_dopeness.hpp>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <random>
#include <vector>

int main(int argc, char** argv) {
    if (argc != 2) return 1;
    auto& map = chmap::ChannelMapDopeness::get_instance();
    map.initialize(argv[1], false);
    std::vector<chmap::FEAddrItem> inputs;
    for (unsigned ip = 0; ip < 256; ++ip)
        for (unsigned ch = 0; ch < 256; ++ch)
            if (map.findDET(2, ip, ch)) inputs.emplace_back(2, ip, ch);
    if (inputs.empty()) return 2;
    std::mt19937 rng(42);
    std::vector<chmap::FEAddrItem> queries;
    for (int i = 0; i < 65536; ++i) queries.push_back(inputs[rng() % inputs.size()]);
    volatile uint64_t sink = 0;
    for (int round = 0; round < 6; ++round) {
        const auto start = std::chrono::steady_clock::now();
        uint64_t sum = 0;
        const bool checked = round % 2;
        for (int repeat = 0; repeat < 100; ++repeat)
            for (const auto& fe : queries) {
                if (checked) {
                    if (const auto* det = map.findDET(fe)) sum += det->channel_number;
                } else {
                    uint32_t key;
                    if (map.getDopeKey_FEtoDET(fe, key)) sum += map.getDETIdItem(key).channel_number;
                }
            }
        sink = sum;
        const auto ns = std::chrono::duration<double, std::nano>(
            std::chrono::steady_clock::now() - start).count();
        std::cout << (checked ? "checked" : "legacy") << " ns/lookup: "
                  << ns / (100 * queries.size()) << " checksum: " << std::dec << sink << '\n';
    }
}
