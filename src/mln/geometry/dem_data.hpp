#pragma once

#include <mln/math/clamp.hpp>
#include <mln/util/image.hpp>
#include <mln/util/tileset.hpp>

#include <memory>
#include <array>
#include <cassert>
#include <vector>

namespace mln {

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
        assert(x >= -2);
        assert(x < dim + 2);
        assert(y >= -2);
        assert(y < dim + 2);
        return (y + 2) * stride + (x + 2);
    }
};

} // namespace mln
