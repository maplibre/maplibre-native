#pragma once

#include <mbgl/style/sources/tile_source.hpp>
#include <mbgl/util/tileset.hpp>
#include <mbgl/util/variant.hpp>

namespace mbgl {

namespace style {

class RasterSource : public TileSource {
public:
    RasterSource(std::string id,
                 variant<std::string, Tileset> urlOrTileset,
                 uint16_t tileSize,
                 SourceType sourceType = SourceType::Raster);

    bool supportsLayerType(const mbgl::style::LayerTypeInfo*) const override;
};

template <>
inline bool Source::is<RasterSource>() const {
    return getType() == SourceType::Raster;
}

} // namespace style
} // namespace mbgl
