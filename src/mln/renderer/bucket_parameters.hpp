#pragma once

#include <mln/map/mode.hpp>
#include <mln/tile/tile_id.hpp>
#include <mln/util/subdivision_granularity.hpp>

namespace mln {
namespace style {
struct LayerTypeInfo;
} // namespace style

class BucketParameters {
public:
    const OverscaledTileID tileID;
    const MapMode mode;
    const float pixelRatio;
    const style::LayerTypeInfo* layerType;
    const SubdivisionGranularitySetting subdivisionGranularity{};
};

} // namespace mln
