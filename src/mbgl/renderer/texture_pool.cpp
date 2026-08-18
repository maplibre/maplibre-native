#include <mbgl/renderer/texture_pool.hpp>
#include <mbgl/gfx/context.hpp>
#include <mbgl/util/logging.hpp>

namespace mbgl {
TexturePool::TexturePool(uint32_t tilesize)
    : tileSize(tilesize) {}

TexturePool::~TexturePool() {}

void TexturePool::createRenderTarget(gfx::Context& context, const UnwrappedTileID& id, const Color& backgroundColor) {
    // Keep an existing render target; recreating it every frame churns GL
    // texture memory and invalidates the texture bound to the terrain drawable
    if (auto it = renderTargets.find(id); it != renderTargets.end() && it->second) {
        it->second->setClearColor(backgroundColor);
        return;
    }
    // Drape targets render several overlapping draped tiles (a parent standing in for missing
    // children beside already-loaded children); the stencil clips them against each other,
    // as maplibre-gl-js does for its render-to-texture framebuffer.
    auto renderTarget = context.createRenderTarget(
        {tileSize, tileSize}, gfx::TextureChannelDataType::UnsignedByte, /*stencil=*/true);
    renderTarget->setClearColor(backgroundColor);
    renderTarget->setDrapeTileID(id);
    renderTargets[id] = std::move(renderTarget);
}

void TexturePool::removeStaleRenderTargets(const std::set<UnwrappedTileID>& currentTiles) {
    for (auto it = renderTargets.begin(); it != renderTargets.end();) {
        if (!currentTiles.contains(it->first)) {
            it = renderTargets.erase(it);
        } else {
            ++it;
        }
    }
}

std::shared_ptr<RenderTarget> TexturePool::getRenderTarget(const UnwrappedTileID& id) const {
    return renderTargets.contains(id) ? renderTargets.at(id) : nullptr;
}

std::shared_ptr<RenderTarget> TexturePool::getRenderTargetAncestorOrDescendant(
    const UnwrappedTileID& id, std::optional<UnwrappedTileID>& terrainTileID) const {
    terrainTileID = std::nullopt;
    std::shared_ptr<RenderTarget> bestRenderTarget;
    for (const auto& [tileID, renderTarget] : renderTargets) {
        if (tileID == id || tileID.isChildOf(id) || id.isChildOf(tileID)) {
            if (!terrainTileID || tileID.canonical.z > terrainTileID->canonical.z) {
                terrainTileID = tileID;
                bestRenderTarget = renderTarget;
            }
        }
    }
    return bestRenderTarget;
}
} // namespace mbgl
