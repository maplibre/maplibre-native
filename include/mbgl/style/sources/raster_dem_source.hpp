#pragma once

#include <mbgl/style/sources/raster_source.hpp>
#include <mbgl/util/tileset.hpp>
#include <mbgl/util/variant.hpp>

namespace mbgl {
namespace style {

struct SourceOptions {
    std::optional<Tileset::RasterEncoding> rasterEncoding = std::nullopt;
    std::optional<Tileset::VectorEncoding> vectorEncoding = std::nullopt;
};

// NOTE: Any derived class must invalidate `weakFactory` in the destructor
class RasterDEMSource final : public RasterSource {
public:
    RasterDEMSource(std::string id,
                    variant<std::string, Tileset> urlOrTileset,
                    uint16_t tileSize,
                    std::optional<SourceOptions> options = std::nullopt);
    ~RasterDEMSource() override;
    bool supportsLayerType(const mbgl::style::LayerTypeInfo*) const override;

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
} // namespace mbgl
