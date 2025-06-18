#pragma once

#include <mbgl/style/sources/tile_source.hpp>
#include <mbgl/util/tileset.hpp>
#include <mbgl/util/variant.hpp>

namespace mbgl {

class AsyncRequest;

namespace style {

class VectorSource final : public TileSource {
public:
    VectorSource(std::string id,
                 variant<std::string, Tileset> urlOrTileset,
                 std::optional<float> maxZoom = std::nullopt,
                 std::optional<float> minZoom = std::nullopt);
    
    void setTilesetOverrides(Tileset& tileset) override;

    /// @brief Gets the tile urls for this vector source.
    /// @return List of tile urls.
    const std::vector<std::string> getTiles() const;

    /// @brief Sets the tile urls for this vector source.
    /// @param tiles List of tile urls.
    void setTiles(const std::vector<std::string>& tiles);

    bool supportsLayerType(const mbgl::style::LayerTypeInfo*) const override;

private:
    std::optional<float> maxZoom;
    std::optional<float> minZoom;
};

template <>
inline bool Source::is<VectorSource>() const {
    return getType() == SourceType::Vector;
}

} // namespace style
} // namespace mbgl
