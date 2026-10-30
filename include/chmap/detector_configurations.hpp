#ifndef CHMAP_DETECTOR_CONFIGURATIONS_HPP_
#define CHMAP_DETECTOR_CONFIGURATIONS_HPP_

#include <memory>
#include "chmap/geometry.hpp"
#include "chmap/calibration.hpp"

namespace chmap {
    class ChannelMapDopeness;

    // Owned exclusively by ChannelMapDopeness. Consumers borrow read-only data.
    class DETConfItem {
        public:
            const GeomItem* getGeometry() const noexcept { return geometry_.get(); }
            const CalibrationItem_DCTdcCalib* getTdcCalibration() const noexcept {
                return tdcCalibration_.get();
            }
            const CalibrationItem_DCDriftLength* getDriftCalibration() const noexcept {
                return driftCalibration_.get();
            }
        private:
            friend class ChannelMapDopeness;
            std::unique_ptr<GeomItem> geometry_;
            std::unique_ptr<CalibrationItem_DCTdcCalib> tdcCalibration_;
            std::unique_ptr<CalibrationItem_DCDriftLength> driftCalibration_;
    };
}
#endif
