#include "chmap/item.hpp"
#include "chmap/channel_map_dopeness.hpp"
#include <iostream>
#include <iomanip>
#include <string>

void chmap::CalibrationItem_DCDriftLength::decode() const {
    std::cout << "\tDC Drift Length Calibration: approxOrder = " << approxOrder << ", coefficients = [";
    for (size_t i = 0; i < coeffs.size(); ++i) {
        std::cout << coeffs[i];
        if (i < coeffs.size() - 1) {
            std::cout << ", ";
        }
    }
    std::cout << "]" << std::endl;
    return;
} // void chmap::CalibrationItem_DCDriftLength::decode()

void chmap::CalibrationItem_DCTdcCalib::decode() const {
    std::cout << "\tDC TDC Calibration: offset = " << offset << ", scale = " << scale << std::endl;
    return;
} // void chmap::CalibrationItem_DCTdcCalib::decode()