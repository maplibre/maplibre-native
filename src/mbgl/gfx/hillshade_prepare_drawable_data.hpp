#pragma once

#include <mbgl/gfx/drawable_data.hpp>
#include <mbgl/util/tileset.hpp>

#include <memory>

namespace mbgl {

namespace gfx {

class HillshadePrepareDrawableData : public DrawableData {
public:
    HillshadePrepareDrawableData(int32_t stride_, Tileset::DEMEncoding encoding_)
        : stride(stride_), encoding(encoding_) {}

    int32_t stride;
    Tileset::DEMEncoding encoding;
};

using UniqueHillshadePrepareDrawableData = std::unique_ptr<HillshadePrepareDrawableData>;

} // namespace gfx
} // namespace mbgl
