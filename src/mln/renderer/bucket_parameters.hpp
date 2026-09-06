#pragma once

#include <mln/map/mode.hpp>
#include <mln/tile/tile_id.hpp>

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
    std::shared_ptr<class FileSource> fileSource;
};

} // namespace mln
