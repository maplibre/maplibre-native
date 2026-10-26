#pragma once

#include <mln/style/sources/tile_source.hpp>
#include <mln/util/tileset.hpp>
#include <mln/util/variant.hpp>

namespace mln {

class AsyncRequest;

namespace style {

// NOTE: Any derived class must invalidate `weakFactory` in the destructor
class VectorSource final : public TileSource {
public:
    VectorSource(std::string id,
                 variant<std::string, Tileset> urlOrTileset,
                 std::optional<float> maxZoom = std::nullopt,
                 std::optional<float> minZoom = std::nullopt,
                 std::optional<Tileset::VectorEncoding> encoding = std::nullopt);

    void setTilesetOverrides(Tileset& tileset) override;

    /// @brief Gets the tile urls for this vector source.
    /// @return List of tile urls.
    const std::vector<std::string> getTiles() const;

    /// @brief Sets the tile urls for this vector source.
    /// @param tiles List of tile urls.
    void setTiles(const std::vector<std::string>& tiles);

    bool supportsLayerType(const mln::style::LayerTypeInfo*) const override;

    /// @brief Gets the encoding explicitly requested for this source.
    /// @return The requested encoding, or `std::nullopt` if none was given, in which case the encoding declared by
    /// the TileJSON (or the Mapbox Vector Tile default) applies.
    std::optional<Tileset::VectorEncoding> getEncoding() const noexcept { return encoding; }

    mapbox::base::WeakPtr<Source> makeWeakPtr() override { return weakFactory.makeWeakPtr(); }

private:
    std::optional<float> maxZoom;
    std::optional<float> minZoom;
    std::optional<Tileset::VectorEncoding> encoding;
    mapbox::base::WeakPtrFactory<Source> weakFactory{this}; // Must remain last
};

template <>
inline bool Source::is<VectorSource>() const {
    return getType() == SourceType::Vector;
}

} // namespace style
} // namespace mln
