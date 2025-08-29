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

    mapbox::base::WeakPtr<Source> makeWeakPtr() override { return weakFactory.makeWeakPtr(); }

protected:
    // Allows derived classes (e.g. RasterDEMSource) to invalidate weak pointers
    // early in their destructor before their own members are torn down.
    void invalidateWeakPtrsEarly() { weakFactory.invalidateWeakPtrs(); }

private:
    mapbox::base::WeakPtrFactory<Source> weakFactory{this}; // Must remain last
};

template <>
inline bool Source::is<RasterSource>() const {
    return getType() == SourceType::Raster;
}

} // namespace style
} // namespace mbgl
