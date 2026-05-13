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
    /// Width of the per-side neighbour border on the buffered image. The
    /// hillshade prepare pass and the contour algorithm both look up to one
    /// pixel beyond a tile's interior to compute Sobel kernels at the
    /// world boundary between adjacent tiles. With a 2-pixel border every
    /// boundary-aligned output pixel reaches into the neighbour's first
    /// two interior columns/rows, which lets adjacent prepare outputs
    /// share an identical edge value and eliminates the visible 1-pixel
    /// step (seam) at overzoomed tile boundaries.
    static constexpr int32_t border = 3;

    DEMData(const PremultipliedImage& image, Tileset::RasterEncoding encoding);
    void backfillBorder(const DEMData& borderTileData, int8_t dx, int8_t dy);

    int32_t get(int32_t x, int32_t y) const;
    const std::array<float, 4>& getUnpackVector() const;

    const PremultipliedImage* getImage() const { return &*image; }
    const std::shared_ptr<PremultipliedImage>& getImagePtr() const { return image; }

    const int32_t dim;
    const int32_t stride;
    const Tileset::RasterEncoding encoding;

private:
    std::shared_ptr<PremultipliedImage> image;

    size_t idx(const int32_t x, const int32_t y) const {
        assert(x >= -border);
        assert(x < dim + border);
        assert(y >= -border);
        assert(y < dim + border);
        return (y + border) * stride + (x + border);
    }
};

} // namespace mbgl
