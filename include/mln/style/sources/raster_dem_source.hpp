#pragma once

#include <mln/style/sources/raster_source.hpp>
#include <mln/util/tileset.hpp>
#include <mln/util/variant.hpp>

namespace mln {
namespace style {

struct SourceOptions {
    std::optional<Tileset::RasterEncoding> rasterEncoding = std::nullopt;
    std::optional<Tileset::VectorEncoding> vectorEncoding = std::nullopt;
    // The style's inline zoom range, which wins over a fetched TileJSON's, as in maplibre-gl-js:
    // https://github.com/maplibre/maplibre-gl-js/blob/32e555b2c114d6b60da4acd824d272182c656bbe/src/source/load_tilejson.ts#L39
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
