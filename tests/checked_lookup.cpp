#include <chmap/channel_map_dopeness.hpp>
#include <cassert>
#include <fstream>
#include <stdexcept>
#include <string>

int main(int argc, char** argv) {
    assert(argc == 3);
    const std::string mode = argv[2];
    std::ofstream csv(argv[1]);
    csv << "fe.id,fe.channel,fe.data,detector.id,detector.plane,detector.segment,detector.channel,detector.readout,detector.data\n";
    if (mode == "zero") csv << "0,0,0,a,X,0,0,left,0\n";
    else csv << "0x0201,0,0,a,X,0,0,left,0\n0x0201,2,0,a,X,0,2,left,0\n";
    csv.close();
    auto& map = chmap::ChannelMapDopeness::get_instance();
    assert(!map.findDET(2, 1, 0));
    assert(!map.findFE("a", "X", 0, "left", 0));
    if (mode == "zero") {
        try { map.initialize(argv[1], true); }
        catch (const std::runtime_error&) { return 0; }
        return 1;
    }
    map.initialize(argv[1], mode != "forward");
    const auto* det = map.findDET(2, 1, 0);
    assert(det && det->name == 1 && det->getDetectorName() == "a");
    assert(!map.findDET(2, 1, 1)); // Hole within allocated range.
    assert(!map.findDET(2, 1, 3));
    assert(!map.findDET(255, 255, 255));
    uint32_t key;
    assert(map.getDopeKey_FEtoDET(2, 1, 1, key)); // Legacy semantics unchanged.
    assert(!map.setGeometry(chmap::FEAddrItem{2, 1, 1}, std::make_unique<chmap::GeomItemDC>()));
    assert(!map.findFE("a", "X", 0, "left", 1));
    assert(!map.findFE("missing", "X", 0, "left", 0));
    assert(!map.findFE("a", "X", 256, "left", 0));
    assert(!map.findFE("a", "X", 0, "left", 65536));
    assert(!map.findFE("a", "X", -1, "left", 0));
    if (mode == "forward") { assert(!map.findFE(*det)); return 0; }
    const auto* fe = map.findFE(*det);
    assert(fe && fe->ip3rd == 2 && fe->ip4th == 1 && fe->ch == 0);
    assert(map.findFE("a", "X", 0, "left", 0) == fe);
    assert(map.findDET(*fe) == det);
    auto geom = std::make_unique<chmap::GeomItemDC>();
    geom->SetGlobalPosition(1, 2, 3);
    assert(map.setGeometry(*det, std::move(geom)));
    assert(det->getConfiguration() && det->getConfiguration()->getGeometry()->GetGlobalZ() == 3);
    chmap::dictionary::NameIndexDictionary dictionary;
    std::string name;
    dictionary.newWord("a"); dictionary.buildDictionary(true);
    assert(!dictionary.IndexToString(0, name));
    for (int i = 0; i < 255; ++i) dictionary.newWord(std::to_string(i));
    try { dictionary.buildDictionary(true); }
    catch (const std::runtime_error&) { return 0; }
    return 1;
}
