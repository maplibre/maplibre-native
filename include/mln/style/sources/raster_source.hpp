#pragma once

#include <mln/style/sources/tile_source.hpp>
#include <mln/util/tileset.hpp>
#include <mln/util/variant.hpp>

namespace mln {
namespace style {

class RasterSource : public TileSource {
public:
    /// `minzoom` / `maxzoom` are the style's inline values: they win over the TileJSON's, as in
    /// maplibre-gl-js.
    RasterSource(std::string id,
                 variant<std::string, Tileset> urlOrTileset,
                 uint16_t tileSize,
                 SourceType sourceType = SourceType::Raster,
                 std::optional<uint8_t> minzoom = std::nullopt,
                 std::optional<uint8_t> maxzoom = std::nullopt);
    ~RasterSource() override;

    bool supportsLayerType(const mln::style::LayerTypeInfo*) const override;

protected:
    void setTilesetOverrides(Tileset& tileset) override;

private:
    std::optional<uint8_t> minzoomOverride;
    std::optional<uint8_t> maxzoomOverride;
};

template <>
inline bool Source::is<RasterSource>() const {
    return getType() == SourceType::Raster;
}

} // namespace style
} // namespace mln
