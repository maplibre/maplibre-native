#pragma once

#include <mbgl/gfx/drawable_data.hpp>
#include <mbgl/util/tileset.hpp>

#include <memory>

namespace mbgl {

namespace gfx {

class HillshadePrepareDrawableData : public DrawableData {
public:
    HillshadePrepareDrawableData(int32_t stride_, Tileset::RasterEncoding encoding_, uint8_t maxzoom_)
        : stride(stride_),
          encoding(encoding_),
          maxzoom(maxzoom_) {}

    int32_t stride;
    Tileset::RasterEncoding encoding;
    uint8_t maxzoom;
};

using UniqueHillshadePrepareDrawableData = std::unique_ptr<HillshadePrepareDrawableData>;

} // namespace gfx
} // namespace mbgl
