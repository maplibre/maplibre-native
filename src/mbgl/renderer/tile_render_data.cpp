#include <mbgl/renderer/tile_render_data.hpp>

namespace mbgl {

TileRenderData::TileRenderData() = default;

TileRenderData::TileRenderData(std::shared_ptr<TileAtlasTextures> atlasTextures_)
    : atlasTextures(std::move(atlasTextures_)) {}

TileRenderData::~TileRenderData() = default;

static gfx::Texture2DPtr noTexture;

const gfx::Texture2DPtr& TileRenderData::getGlyphAtlasTexture() const {
    return atlasTextures ? atlasTextures->glyph : noTexture;
}

const gfx::Texture2DPtr& TileRenderData::getIconAtlasTexture() const {
    return atlasTextures ? atlasTextures->icon : noTexture;
}

std::optional<ImagePosition> TileRenderData::getPattern(const std::string&) const {
    assert(false);
    return std::nullopt;
}

const LayerRenderData* TileRenderData::getLayerRenderData(const style::Layer::Impl&) const {
    assert(false);
    return nullptr;
}

Bucket* TileRenderData::getBucket(const style::Layer::Impl&) const {
    assert(false);
    return nullptr;
}

} // namespace mbgl
