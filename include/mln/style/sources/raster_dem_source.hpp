#pragma once

#include <mln/style/sources/raster_source.hpp>
#include <mln/util/tileset.hpp>
#include <mln/util/variant.hpp>

namespace mln {
namespace style {

struct SourceOptions {
    std::optional<Tileset::RasterEncoding> rasterEncoding = std::nullopt;
    std::optional<Tileset::VectorEncoding> vectorEncoding = std::nullopt;
    // Inline style overrides applied on top of a fetched TileJSON, matching maplibre-gl-js
    // (`extend(tileJSON, options)` in load_tilejson.ts). A raster-dem source served from an
    // archive whose TileJSON stops at its deepest stored zoom can declare a higher `maxzoom`
    // so terrain meshes and drapes at the display zoom while the DEM overzooms.
    std::optional<uint8_t> minzoom = std::nullopt;
    std::optional<uint8_t> maxzoom = std::nullopt;
};

// NOTE: Any derived class must invalidate `weakFactory` in the destructor
class RasterDEMSource final : public RasterSource {
public:
    RasterDEMSource(std::string id,
                    variant<std::string, Tileset> urlOrTileset,
                    uint16_t tileSize,
                    std::optional<SourceOptions> options = std::nullopt);
    ~RasterDEMSource() override;
    bool supportsLayerType(const mln::style::LayerTypeInfo*) const override;

protected:
    void setTilesetOverrides(Tileset& tileset) override;

private:
    std::optional<SourceOptions> options;
};

template <>
inline bool Source::is<RasterDEMSource>() const {
    return getType() == SourceType::RasterDEM;
}

} // namespace style
} // namespace mln
