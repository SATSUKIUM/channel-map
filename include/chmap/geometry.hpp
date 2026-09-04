// Base class for geometry items in the channel map

#ifndef CHMAP_GEOMETRY_HPP_
#define CHMAP_GEOMETRY_HPP_

namespace chmap {
    class GeomItem{
        public:
            virtual ~GeomItem() = default;
            void SetGlobalPosition(double x_, double y_, double z_) {
                globalX = x_;
                globalY = y_;
                globalZ = z_;
            }
            void SetResolution(double resX_, double resY_, double resZ_) {
                resolutionX = resX_;
                resolutionY = resY_;
                resolutionZ = resZ_;
            }
            void SetRotationAngles(double tilt_, double rot1_, double rot2_) {
                tiltAngle = tilt_;
                rotAngle1 = rot1_;
                rotAngle2 = rot2_;
            }
            double GetGlobalX() const { return globalX; }
            double GetGlobalY() const { return globalY; }
            double GetGlobalZ() const { return globalZ; }
            double GetResolutionX() const { return resolutionX; }
            double GetResolutionY() const { return resolutionY; }
            double GetResolutionZ() const { return resolutionZ; }
            double GetTiltAngle() const { return tiltAngle; }
            double GetRotAngle1() const { return rotAngle1; }
            double GetRotAngle2() const { return rotAngle2; }

        private:
            uint32_t dopeKey_DET; // getDetItem[dopeKey_DET] return us the corresponding DETIdItem
            double globalZ, globalX, globalY; // [mm]
            double resolutionZ, resolutionX, resolutionY; // [mm]
            double tiltAngle, rotAngle1, rotAngle2; // [deg] (tiltAngle: z軸周りの回転角, rotAngle: ?軸周りの回転角, rotAngle2: ?軸周りの回転角)(回転の順番もここで定義すべき)

    }; // class GeomItem

    class GeomItemDC : public GeomItem {
        public:
            virtual ~GeomItemDC() = default;
            void SetWireGeometry(double centerWireNumber_, double wirePitch_, double offset_) {
                centerWireNumber = centerWireNumber_;
                wirePitch = wirePitch_;
                offset = offset_;
            }

            double GetCenterWireNumber() const { return centerWireNumber; }
            double GetWirePitch() const { return wirePitch; }
            double GetOffset() const { return offset; }
            double GetWirePosition() const {
                return wirePosition;
            }
            void CalcWirePosition(int wireNumber) {
                wirePosition = (wireNumber - centerWireNumber) * wirePitch + offset;
            }
        private:
            double centerWireNumber; // もし1.0なら、中心のワイヤーは1番ワイヤー。0.5なら、中心のワイヤーは1番と2番の間にある。
            double wirePitch; // [mm] 測定軸方向のワイヤ間隔
            double offset; // [mm] 測定軸方向のワイヤのオフセット(微調整のため)

            double wirePosition{DOUBLE_MAX};
    }; // class GeomItemDC
    
} // namespace chmap

#endif // CHMAP_GEOMETRY_HPP_