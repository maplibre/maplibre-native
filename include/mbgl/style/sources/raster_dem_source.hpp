#pragma once

#include <mbgl/style/sources/raster_source.hpp>
#include <mbgl/util/tileset.hpp>
#include <mbgl/util/variant.hpp>

namespace mbgl {

namespace style {

struct RasterDEMOptions {
    std::optional<Tileset::Encoding> encoding = std::nullopt;
};

class RasterDEMSource : public RasterSource {
public:
    RasterDEMSource(std::string id,
                    variant<std::string, Tileset> urlOrTileset,
                    uint16_t tileSize,
                    std::optional<RasterDEMOptions> options = std::nullopt);
    bool supportsLayerType(const mbgl::style::LayerTypeInfo*) const override;

protected:
    void setTilesetOverrides(Tileset& tileset) override;

private:
    std::optional<RasterDEMOptions> options;
};

template <>
inline bool Source::is<RasterDEMSource>() const {
    return getType() == SourceType::RasterDEM;
}

} // namespace style
} // namespace mbgl
