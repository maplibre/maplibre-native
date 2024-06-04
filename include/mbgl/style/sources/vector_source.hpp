#pragma once

#include <mbgl/style/source.hpp>
#include <mbgl/util/tileset.hpp>
#include <mbgl/util/variant.hpp>

namespace mbgl {

class AsyncRequest;

namespace style {

class VectorSource final : public Source {
public:
    VectorSource(std::string id,
                 variant<std::string, Tileset> urlOrTileset,
                 std::optional<float> maxZoom = std::nullopt,
                 std::optional<float> minZoom = std::nullopt);
    ~VectorSource() final;

    const variant<std::string, Tileset>& getURLOrTileset() const;
    std::optional<std::string> getURL() const;

    class Impl;
    const Impl& impl() const;

    void loadDescription(FileSource&) final;

    /// @brief Gets the tile urls for this vector source.
    /// @return List of tile urls.
    const std::vector<std::string> getTiles() const;

    /// @brief Sets the tile urls for this vector source.
    /// @param tiles List of tile urls.
    void setTiles(const std::vector<std::string>& tiles);

    bool supportsLayerType(const mbgl::style::LayerTypeInfo*) const override;

    mapbox::base::WeakPtr<Source> makeWeakPtr() override { return weakFactory.makeWeakPtr(); }

protected:
    Mutable<Source::Impl> createMutable() const noexcept final;

private:
    const variant<std::string, Tileset> urlOrTileset;
    std::unique_ptr<AsyncRequest> req;
    std::optional<float> maxZoom;
    std::optional<float> minZoom;
    mapbox::base::WeakPtrFactory<Source> weakFactory{this};
    // Do not add members here, see `WeakPtrFactory`
};

template <>
inline bool Source::is<VectorSource>() const {
    return getType() == SourceType::Vector;
}

} // namespace style
} // namespace mbgl
