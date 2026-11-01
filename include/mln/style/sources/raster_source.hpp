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

    bool supportsLayerType(const mln::style::LayerTypeInfo*) const override;

    mapbox::base::WeakPtr<Source> makeWeakPtr() override { return weakFactory.makeWeakPtr(); }

protected:
    void setTilesetOverrides(Tileset& tileset) override;

    // Allows derived classes (e.g. RasterDEMSource) to invalidate weak pointers
    // early in their destructor before their own members are torn down.
    void invalidateWeakPtrsEarly() { weakFactory.invalidateWeakPtrs(); }

private:
    std::optional<uint8_t> minzoomOverride;
    std::optional<uint8_t> maxzoomOverride;

    mapbox::base::WeakPtrFactory<Source> weakFactory{this}; // Must remain last
};

template <>
inline bool Source::is<RasterSource>() const {
    return getType() == SourceType::Raster;
}

} // namespace style
} // namespace mln
