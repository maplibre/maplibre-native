#pragma once

#include <mbgl/style/sources/tile_source.hpp>
#include <mbgl/util/tileset.hpp>
#include <mbgl/util/variant.hpp>

namespace mbgl {

class AsyncRequest;

namespace style {

// NOTE: Any derived class must invalidate `weakFactory` in the destructor
class VectorSource final : public TileSource {
public:
    VectorSource(std::string id,
                 variant<std::string, Tileset> urlOrTileset,
                 std::optional<float> maxZoom = std::nullopt,
                 std::optional<float> minZoom = std::nullopt,
                 Tileset::VectorEncoding encoding = Tileset::VectorEncoding::Mapbox);

    void setTilesetOverrides(Tileset& tileset) override;

    /// @brief Gets the tile urls for this vector source.
    /// @return List of tile urls.
    const std::vector<std::string> getTiles() const;

    /// @brief Sets the tile urls for this vector source.
    /// @param tiles List of tile urls.
    void setTiles(const std::vector<std::string>& tiles);

    bool supportsLayerType(const mbgl::style::LayerTypeInfo*) const override;

    Tileset::VectorEncoding getEncoding() const noexcept { return encoding; }

    mapbox::base::WeakPtr<Source> makeWeakPtr() override { return weakFactory.makeWeakPtr(); }

private:
    std::optional<float> maxZoom;
    std::optional<float> minZoom;
    Tileset::VectorEncoding encoding;
    mapbox::base::WeakPtrFactory<Source> weakFactory{this}; // Must remain last
};

template <>
inline bool Source::is<VectorSource>() const {
    return getType() == SourceType::Vector;
}

} // namespace style
} // namespace mbgl
