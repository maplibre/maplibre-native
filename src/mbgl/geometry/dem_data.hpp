#pragma once

#include <mbgl/math/clamp.hpp>
#include <mbgl/util/image.hpp>
#include <mbgl/util/tileset.hpp>

#include <memory>
#include <array>
#include <cassert>
#include <vector>

namespace mbgl {

class DEMData {
public:
    DEMData(const PremultipliedImage& image, Tileset::RasterEncoding encoding);
    void backfillBorder(const DEMData& borderTileData, int8_t dx, int8_t dy);

    int32_t get(int32_t x, int32_t y) const;
    const std::array<float, 4>& getUnpackVector() const;

    const PremultipliedImage* getImage() const { return &*image; }
    const std::shared_ptr<PremultipliedImage>& getImagePtr() const { return image; }

    /// Lowest elevation in the tile, in meters, excluding the backfilled border.
    int32_t getMinElevation() const { return minElevation; }
    /// Highest elevation in the tile, in meters, excluding the backfilled border.
    int32_t getMaxElevation() const { return maxElevation; }

    const int32_t dim;
    const int32_t stride;
    const Tileset::RasterEncoding encoding;

private:
    std::shared_ptr<PremultipliedImage> image;
    int32_t minElevation = 0;
    int32_t maxElevation = 0;

    size_t idx(const int32_t x, const int32_t y) const {
        assert(x >= -1);
        assert(x < dim + 1);
        assert(y >= -1);
        assert(y < dim + 1);
        return (y + 1) * stride + (x + 1);
    }
};

} // namespace mbgl
