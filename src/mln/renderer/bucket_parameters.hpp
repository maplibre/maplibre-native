#pragma once

#include <mln/map/mode.hpp>
#include <mln/tile/tile_id.hpp>
#include <mln/util/feature.hpp>

#include <memory>

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
    // The style's global state, used by "global-state" expressions.
    std::shared_ptr<const GlobalStateMap> globalState = nullptr;
};

} // namespace mln
