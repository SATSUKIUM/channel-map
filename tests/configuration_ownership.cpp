#include <chmap/channel_map_dopeness.hpp>
#include <cassert>
#include <cstdlib>
#include <fstream>
#include <type_traits>
#include <utility>

namespace {
int geometryDestroyed = 0, tdcDestroyed = 0, driftDestroyed = 0;
struct Geometry : chmap::GeomItemDC { ~Geometry() override { ++geometryDestroyed; } };
struct Tdc : chmap::CalibrationItem_DCTdcCalib { ~Tdc() override { ++tdcDestroyed; } };
struct Drift : chmap::CalibrationItem_DCDriftLength { ~Drift() override { ++driftDestroyed; } };
void afterMapDestruction() {
    // Two successful geometries (including one added during growth), one rejected
    // duplicate, one rejected empty slot, and one rejected unknown DET.
    assert(geometryDestroyed == 5);
    assert(tdcDestroyed == 2); // One owned and one rejected duplicate.
    assert(driftDestroyed == 2);
}
}

int main(int argc, char** argv) {
    assert(argc == 2 || argc == 3);
    if (argc == 3) {
        std::ofstream csv(argv[1]);
        csv << "fe.id,fe.channel,fe.data,detector.id,detector.plane,detector.segment,detector.channel,detector.readout,detector.data\n"
            << "0x0201,0,0,a,X,0,0,left,0\n";
        csv.close();
        auto& map = chmap::ChannelMapDopeness::get_instance();
        map.initialize(argv[1], false);
        const chmap::FEAddrItem fe{2, 1, 0};
        const auto* det = map.findDET(fe);
        assert(!map.setGeometry(*det, std::make_unique<chmap::GeomItemDC>()));
        assert(map.setGeometry(fe, std::make_unique<chmap::GeomItemDC>()));
        assert(det->getConfiguration()->getGeometry());
        return 0;
    }
    // Registered before singleton construction: callback executes after its destructor.
    std::atexit(afterMapDestruction);
    std::ofstream csv(argv[1]);
    csv << "fe.id,fe.channel,fe.data,detector.id,detector.plane,detector.segment,detector.channel,detector.readout,detector.data\n"
        << "0x0201,0,0,a,X,0,0,left,0\n0x0201,2,0,a,X,0,2,left,0\n";
    csv.close();
    auto& map = chmap::ChannelMapDopeness::get_instance();
    map.initialize(argv[1], true);
    const chmap::FEAddrItem fe{2, 1, 0};
    const auto* det = map.findDET(fe);
    assert(det && !det->getConfiguration());
    assert(!map.setGeometry(fe, nullptr));
    assert(!det->getConfiguration());
    const chmap::DETIdItem copy = *det; // Identifier copies resolve to the canonical slot.
    const chmap::GeomItem* geometry;
    {
        auto value = std::make_unique<Geometry>();
        value->SetGlobalPosition(1, 2, 3);
        geometry = value.get();
        assert(map.setGeometry(copy, std::move(value)));
        assert(!value);
        assert(geometryDestroyed == 0);
    }
    const auto* config = det->getConfiguration();
    assert(config && config->getGeometry() == geometry);
    assert(config->getGeometry()->GetGlobalZ() == 3);
    assert(!config->getTdcCalibration() && !config->getDriftCalibration());
    auto tdc = std::make_unique<Tdc>(); tdc->SetTdcCalibration(4, 5);
    auto drift = std::make_unique<Drift>(); drift->SetApproximation(2, {6, 7});
    assert(map.setTdcCalibration(fe, std::move(tdc)));
    assert(map.setDriftCalibration(copy, std::move(drift)));
    const auto* savedTdc = config->getTdcCalibration();
    const auto* savedDrift = config->getDriftCalibration();
    assert(savedTdc->GetOffset() == 4 && savedDrift->GetDriftLength(2) == 20);
    assert(!map.setGeometry(fe, std::make_unique<Geometry>()));
    assert(!map.setTdcCalibration(copy, std::make_unique<Tdc>()));
    assert(!map.setDriftCalibration(fe, std::make_unique<Drift>()));
    assert(geometryDestroyed == 1 && tdcDestroyed == 1 && driftDestroyed == 1);
    assert(config->getGeometry() == geometry && config->getTdcCalibration() == savedTdc);
    assert(config->getDriftCalibration() == savedDrift);
    assert(!map.setGeometry(chmap::FEAddrItem{2, 1, 1}, std::make_unique<Geometry>()));
    auto unknown = copy; unknown.channel_number = 1;
    assert(!map.setGeometry(unknown, std::make_unique<Geometry>()));
    assert(map.setGeometry(chmap::FEAddrItem{2, 1, 2}, std::make_unique<Geometry>()));
    assert(det->getConfiguration() == config && config->getGeometry() == geometry);
    static_assert(std::is_same_v<decltype(det->getConfiguration()), const chmap::DETConfItem*>);
    static_assert(std::is_same_v<decltype(config->getGeometry()), const chmap::GeomItem*>);
    static_assert(std::is_same_v<decltype(config->getTdcCalibration()), const chmap::CalibrationItem_DCTdcCalib*>);
}
