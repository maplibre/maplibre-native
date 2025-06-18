#pragma once

#include <mbgl/style/sources/raster_source.hpp>
#include <mbgl/util/tileset.hpp>
#include <mbgl/util/variant.hpp>

namespace mbgl {

class AsyncRequest;

namespace style {

class RasterDEMSource : public RasterSource {
public:
    RasterDEMSource(std::string id,
                    variant<std::string, Tileset> urlOrTileset,
                    uint16_t tileSize,
                    std::optional<Tileset::DEMEncoding> encoding = std::nullopt);

    void loadDescription(FileSource& fileSource) override;

    bool supportsLayerType(const mbgl::style::LayerTypeInfo*) const override;

private:
    std::optional<Tileset::DEMEncoding> encoding;
};

template <>
inline bool Source::is<RasterDEMSource>() const {
    return getType() == SourceType::RasterDEM;
}

} // namespace style
} // namespace mbgl
