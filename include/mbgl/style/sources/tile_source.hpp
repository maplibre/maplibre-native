#pragma once

#include <mbgl/style/source.hpp>
#include <mbgl/util/tileset.hpp>
#include <mbgl/util/variant.hpp>

namespace mbgl {

class AsyncRequest;

namespace style {

class TileSource : public Source {
public:
    TileSource(std::string id, variant<std::string, Tileset> urlOrTileset, uint16_t tileSize, SourceType sourceType);
    ~TileSource() override;

    const variant<std::string, Tileset>& getURLOrTileset() const;
    std::optional<std::string> getURL() const;

    uint16_t getTileSize() const;

    class Impl;
    const Impl& impl() const;

    void loadDescription(FileSource&) final;

protected:
    Mutable<Source::Impl> createMutable() const noexcept final;
    virtual void setTilesetOverrides(Tileset& tileset);

private:
    const variant<std::string, Tileset> urlOrTileset;
    std::unique_ptr<AsyncRequest> req;
};

template <>
inline bool Source::is<TileSource>() const {
    return getType() == SourceType::Raster;
}

} // namespace style
} // namespace mbgl
