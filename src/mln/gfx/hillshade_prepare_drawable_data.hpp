#pragma once

#include <mln/gfx/drawable_data.hpp>

#include <array>
#include <memory>

namespace mln {

namespace gfx {

class HillshadePrepareDrawableData : public DrawableData {
public:
    HillshadePrepareDrawableData(int32_t stride_, const std::array<float, 4>& unpack_, uint8_t maxzoom_)
        : stride(stride_),
          unpack(unpack_),
          maxzoom(maxzoom_) {}

    int32_t stride;
    /// Decodes a DEM pixel into its elevation, see `DEMData::getUnpackVector`.
    std::array<float, 4> unpack;
    uint8_t maxzoom;
};

using UniqueHillshadePrepareDrawableData = std::unique_ptr<HillshadePrepareDrawableData>;

} // namespace gfx
} // namespace mln
