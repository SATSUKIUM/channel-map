#include "chmap/channel_map_dopeness.hpp"

#include <cstdint>
#include <exception>
#include <iostream>
#include <memory>
#include <utility>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " map.csv\n";
        return 1;
    }
    try {
        auto& map = chmap::ChannelMapDopeness::get_instance();
        const double fill_ratio = map.initialize(argv[1], true);
        std::cout << std::dec << "FE fill ratio: " << fill_ratio << '\n';

        // Example from mapdata_T103_20260404.csv: 192.168.2.170, CH12.
        // nullptr covers both out-of-range addresses and unmapped slots inside the range.
        const auto* det = map.findDET(2, 170, 12);
        if (!det) {
            std::cerr << "Example FEE channel is not registered in this CSV\n";
            return 2;
        }
        det->decode();

        // Reuse the returned item; no second lookup is needed to access its fields.
        const auto* fe = map.findFE(*det);
        if (!fe) {
            std::cerr << "Inverse mapping is not registered\n";
            return 3;
        }
        fe->decode();

        // String lookup uses: name, plane, segment, readout name, channel number.
        const auto* named_fe = map.findFE(det->getDetectorName(), det->getDetectorPlane(),
                                         det->segment, det->getReadoutChannelName(),
                                         det->channel_number);
        if (!named_fe || !(*named_fe == *fe)) return 4;

        auto geom = std::make_unique<chmap::GeomItemDC>();
        geom->SetGlobalPosition(0.0, 0.0, 100.0);
        geom->SetResolution(0.1, 0.1, 0.1);
        geom->SetRotationAngles(0.0, 0.0, 0.0);
        geom->SetWireGeometry(1.0, 5.0, 0.0);
        geom->CalcWirePosition(det->channel_number);
        if (!map.setGeometry(*det, std::move(geom))) return 5;
        if (det->getConfiguration() && det->getConfiguration()->getGeometry())
            std::cout << "Registered geometry Z: "
                      << det->getConfiguration()->getGeometry()->GetGlobalZ() << '\n';
    } catch (const std::exception& e) {
        std::cerr << "Channel map error: " << e.what() << '\n';
        return 1;
    }
}
