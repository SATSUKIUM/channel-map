#include "chmap/item.hpp"
#include "chmap/channel_map_dopeness.hpp"
#include <iostream>
#include <iomanip>
#include <string>

void chmap::GeomItem::decode() const {
    std::cout << "\tPosition: (x, y, z) = (" << globalX << ", " << globalY << ", " << globalZ << ") [mm]" << std::endl;
    std::cout << "\tResolution: (x, y, z) = (" << resolutionX << ", " << resolutionY << ", " << resolutionZ << ") [mm]" << std::endl;
    std::cout << "\tRotation angles: (tilt, rot1, rot2) = (" << tiltAngle << ", " << rotAngle1 << ", " << rotAngle2 << ") [deg]" << std::endl;

    return;
} // void chmap::GeomItem::decode()

void chmap::GeomItemDC::decode() const {
    GeomItem::decode();
    std::cout << "\tWire geometry: center wire number = " << centerWireNumber << ", wire pitch = " << wirePitch << " [mm], offset = " << offset << " [mm]" << std::endl;
    std::cout << "\tCalculated wire position: " << wirePosition << " [mm]" << std::endl;

    return;
} // void chmap::GeomItemDC::decode()