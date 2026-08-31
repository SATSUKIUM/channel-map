#ifndef CHMAP_CALIBRATION_HPP_
#define CHMAP_CALIBRATION_HPP_

#include <vector>

namespace chmap {
    class CalibrationItem {
        public:
            virtual ~CalibrationItem() = default;
        private:
            uint32_t dopeKey_DET; // getDetItem[dopeKey_DET] return us the corresponding DETIdItem
    };

    class CalibrationItem_DCDriftLength : public CalibrationItem {
        public:
            virtual ~CalibrationItem_DCDriftLength() = default;
            void SetApproximation(int approxOrder_, const std::vector<double>& coeffs_) {
                approxOrder = approxOrder_;
                coeffs = coeffs_;
            }

            int GetApproximationOrder() const { return approxOrder; }
            const std::vector<double>& GetCoefficients() const { return coeffs; }
        private:
            int approxOrder; // the number of coefficients for polynomial approximation of drift length
            std::vector<double> coeffs; // coefficients for polynomial approximation of drift length
            // dLen(t) = coeffs[0] + coeffs[1]*t + coeffs[2]*t^2 + ... + coeffs[n]*t^n
    };

    class CalibrationItem_DCTdcCalib : public CalibrationItem {
        public:
            virtual ~CalibrationItem_DCTdcCalib() = default;
            void SetTdcCalibration(double offset_, double scale_) {
                offset = offset_;
                scale = scale_;
            }

            double GetOffset() const { return offset; }
            double GetScale() const { return scale; }
        private:
            double offset, scale; // relative time = (absolute time * scale) + offset
    };
} // namespace chmap

#endif // CHMAP_CALIBRATION_HPP_