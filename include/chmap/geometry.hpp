// Base class for geometry items in the channel map

#ifndef CHMAP_GEOMETRY_HPP_
#define CHMAP_GEOMETRY_HPP_

namespace chmap {
    class GeomItem{
        public:
            virtual ~GeomItem() = default;

        private:
            double globalZ, globalX, globalY;
    }; // class GeomItem

    class GeomItemDC : public GeomItem {
        public:
            virtual ~GeomItemDC() = default;
        private:
            double localZ, localX, localY;
    }; // class GeomItemDC
    
} // namespace chmap

#endif // CHMAP_GEOMETRY_HPP_