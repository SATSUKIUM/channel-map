#ifndef CHMAP_INITIALIZATION_DIAGNOSTICS_HPP_
#define CHMAP_INITIALIZATION_DIAGNOSTICS_HPP_

#include "chmap/item.hpp"
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>

namespace chmap::detail {
    inline void printMapping(std::ostream& out, size_t entry, const ItemPair& item) {
        out << "  entry " << entry + 1 << ": FE=(ip3rd=" << unsigned(item.fe.ip3rd)
            << ", ip4th=" << unsigned(item.fe.ip4th) << ", ch=" << unsigned(item.fe.ch)
            << ") DET=(name=" << std::quoted(item.det.getDetectorName())
            << ", plane=" << std::quoted(item.det.getDetectorPlane())
            << ", segment=" << unsigned(item.det.segment)
            << ", channel=" << item.det.channel_number
            << ", readout=" << std::quoted(item.det.getReadoutChannelName()) << ")\n";
    }

    // Called once after CSV loading, before sorting. Entry numbers refer to loaded
    // records, not physical CSV lines (malformed rows may have been skipped).
    inline void warnDuplicateMappings(const std::vector<ItemPair>& items,
                                      std::ostream& out = std::cerr) {
        std::map<FEAddrItem, size_t> seenFE;
        std::map<DETIdItem, size_t> seenDET;
        for (size_t i = 0; i < items.size(); ++i) {
            const auto fe = seenFE.emplace(items[i].fe, i);
            const auto det = seenDET.emplace(items[i].det, i);
            if (fe.second && det.second) continue;
            std::ostringstream message;
            if (!fe.second) {
                message << "[ChannelMap WARNING] Duplicate FE address (continuing):\n";
                printMapping(message, fe.first->second, items[fe.first->second]);
                printMapping(message, i, items[i]);
            }
            if (!det.second) {
                message << "[ChannelMap WARNING] Duplicate DET identifier (continuing):\n";
                printMapping(message, det.first->second, items[det.first->second]);
                printMapping(message, i, items[i]);
            }
            out << message.str();
        }
    }

    // Array storage estimate only: excludes CSV records, dictionaries, configs,
    // allocator overhead and temporary initialization storage. 1 GB = 10^9 bytes.
    inline void warnMapMemory(uint64_t forwardSlots, uint64_t inverseSlots,
                              std::ostream& out = std::cerr) {
        const uint64_t forwardBytes = forwardSlots * sizeof(DETIdItem);
        const uint64_t inverseBytes = inverseSlots * sizeof(FEAddrItem);
        const uint64_t totalBytes = forwardBytes + inverseBytes;
        if (totalBytes <= 1000000000ULL) return;
        std::ostringstream message;
        message << "[ChannelMap WARNING] Estimated lookup-array memory exceeds 1 GB "
                << "(1000000000 bytes): forward=" << forwardBytes
                << " bytes, inverse=" << inverseBytes << " bytes, total=" << totalBytes
                << " bytes. Allocation will continue.\n";
        out << message.str();
    }
}
#endif
