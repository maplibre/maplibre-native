#include <mln/renderer/texture_pool.hpp>

#include <mln/gfx/texture2d.hpp>
#include <mln/gfx/context.hpp>

namespace mln {

namespace {
// The width of the target's texture, 0 if it has none. The texture object exists before its memory
// does (that is allocated when the target first bakes), so a target's size is known at once.
uint32_t textureWidth(const std::shared_ptr<RenderTarget>& target) {
    if (!target) return 0;
    const auto& texture = target->getTexture();
    return texture ? texture->getSize().width : 0;
}
} // namespace

TexturePool::TexturePool(uint32_t fullSize_, uint32_t halfSize_)
    : fullSize(fullSize_),
      halfSize(halfSize_) {}

TexturePool::~TexturePool() = default;

TexturePool::Resizes TexturePool::createRenderTargets(gfx::Context& context,
                                                      const std::set<UnwrappedTileID>& cover,
                                                      const std::set<UnwrappedTileID>& fullSizeTiles,
                                                      const Color& backgroundColor,
                                                      size_t maxResizes) {
    const bool held = fullSizeHold > 0;
    if (held) --fullSizeHold;
    Resizes resizes;
    // Targets shrinking go first (so the full-size count only falls before it rises), then the
    // ones growing; tiles without a target get one in the first pass.
    for (const bool growing : {false, true}) {
        for (const auto& id : cover) {
            const auto found = renderTargets.find(id);
            const uint32_t have = found != renderTargets.end() ? textureWidth(found->second) : 0;
            uint32_t wanted = fullSizeTiles.contains(id) ? fullSize : halfSize;
            if (held && wanted == fullSize && have != fullSize) wanted = halfSize;
            // Keep an existing render target; recreating it every frame churns GL
            // texture memory and invalidates the texture bound to the terrain drawable.
            if (have == wanted) {
                if (!growing) found->second->setClearColor(backgroundColor);
                continue;
            }
            if (have != 0) {
                if ((wanted > have) != growing) continue;
                if (maxResizes > 0 && resizes.replaced.size() == maxResizes) {
                    resizes.deferred = true;
                    found->second->setClearColor(backgroundColor);
                    continue;
                }
                resizes.replaced.insert(id);
            }
            // Drape targets render several overlapping draped tiles (a parent standing in for
            // missing children beside already-loaded children); the stencil clips them against each
            // other, as maplibre-gl-js does for its render-to-texture framebuffer.
            auto renderTarget = context.createRenderTarget(
                {wanted, wanted}, gfx::TextureChannelDataType::UnsignedByte, /*stencil=*/true);
            renderTarget->setClearColor(backgroundColor);
            renderTarget->setDrapeTileID(id);
            renderTargets[id] = std::move(renderTarget);
        }
    }
    return resizes;
}

std::set<UnwrappedTileID> TexturePool::tilesAtFullSize() const {
    std::set<UnwrappedTileID> tiles;
    for (const auto& [id, target] : renderTargets) {
        if (textureWidth(target) == fullSize) tiles.insert(id);
    }
    return tiles;
}

bool TexturePool::renderTargetFailed(const UnwrappedTileID& id) {
    renderTargets.erase(id);
    // Not extended by a failure inside the window, so that one that goes on is reported again
    // once the window is over, and once per window.
    const bool first = fullSizeHold == 0;
    if (first) fullSizeHold = failureWindowFrames;
    return first;
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
} // namespace mln
