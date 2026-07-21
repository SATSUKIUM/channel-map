#ifndef CHMAP_DETECTOR_CONFIGURATIONS_HPP_
#define CHMAP_DETECTOR_CONFIGURATIONS_HPP_

#include <memory>

#include "chmap/geometry.hpp"
#include "chmap/calibration.hpp"

namespace chmap {
    struct DETConfItem{
            std::unique_ptr<GeomItem> geom = nullptr;
            std::unique_ptr<CalibrationItem> calib = nullptr;
        }; // struct DETConfItem
} // namespace chmap

#endif // CHMAP_DETECTOR_CONFIGURATIONS_HPP_
