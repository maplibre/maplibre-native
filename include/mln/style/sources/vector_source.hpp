#pragma once

#include <mln/style/sources/tile_source.hpp>
#include <mln/util/tileset.hpp>
#include <mln/util/variant.hpp>

namespace mln {

class AsyncRequest;

namespace style {

class VectorSource final : public TileSource {
public:
    VectorSource(std::string id,
                 variant<std::string, Tileset> urlOrTileset,
                 std::optional<float> maxZoom = std::nullopt,
                 std::optional<float> minZoom = std::nullopt,
                 Tileset::VectorEncoding encoding = Tileset::VectorEncoding::Mapbox);
    ~VectorSource() override;

    void setTilesetOverrides(Tileset& tileset) override;

    /// @brief Gets the tile urls for this vector source.
    /// @return List of tile urls.
    std::vector<std::string> getTiles() const;

    /// @brief Sets the tile urls for this vector source.
    /// @param tiles List of tile urls.
    void setTiles(const std::vector<std::string>& tiles);

    bool supportsLayerType(const mln::style::LayerTypeInfo*) const override;

    Tileset::VectorEncoding getEncoding() const noexcept { return encoding; }

private:
    std::optional<float> maxZoom;
    std::optional<float> minZoom;
    Tileset::VectorEncoding encoding;
};

template <>
inline bool Source::is<VectorSource>() const {
    return getType() == SourceType::Vector;
}

} // namespace style
} // namespace mln
