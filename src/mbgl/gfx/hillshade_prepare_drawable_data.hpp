#pragma once

#include <mbgl/gfx/drawable_data.hpp>
#include <mbgl/util/tileset.hpp>

#include <memory>

namespace mbgl {

namespace gfx {

class HillshadePrepareDrawableData : public DrawableData {
public:
    HillshadePrepareDrawableData(int32_t stride_,
                                 Tileset::DEMEncoding encoding_,
                                 uint8_t maxzoom_,
                                 std::shared_ptr<PremultipliedImage> image_)
        : stride(stride_),
          encoding(encoding_),
          maxzoom(maxzoom_),
          image(image_) {}

    int32_t stride;
    Tileset::DEMEncoding encoding;
    uint8_t maxzoom;
    std::shared_ptr<PremultipliedImage> image;
};

using UniqueHillshadePrepareDrawableData = std::unique_ptr<HillshadePrepareDrawableData>;

} // namespace gfx
} // namespace mbgl
