#include <mbgl/renderer/layer_group.hpp>

#include <mbgl/gfx/upload_pass.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/render_orchestrator.hpp>

#include <unordered_map>

namespace mbgl {

struct TileLayerGroupTileKey {
    mbgl::RenderPass renderPass;
    OverscaledTileID tileID;
    bool operator==(const TileLayerGroupTileKey& other) const {
        return renderPass == other.renderPass && tileID == other.tileID;
    }
    struct hash {
        size_t operator()(const mbgl::TileLayerGroupTileKey& k) const {
            return (std::hash<mbgl::RenderPass>()(k.renderPass) ^ std::hash<OverscaledTileID>()(k.tileID) << 1);
        }
    };
};

struct TileLayerGroup::Impl {
    Impl(std::size_t capacity)
        : tileDrawables(capacity) {}

    using TileMap = std::unordered_map<TileLayerGroupTileKey, gfx::UniqueDrawable, TileLayerGroupTileKey::hash>;
    TileMap tileDrawables;
};

LayerGroup::LayerGroup(int32_t layerIndex_)
    : layerIndex(layerIndex_) {}

TileLayerGroup::TileLayerGroup(int32_t layerIndex_, std::size_t initialCapacity)
    : LayerGroup(layerIndex_),
      impl(std::make_unique<Impl>(initialCapacity)) {}

TileLayerGroup::~TileLayerGroup() {}

static const gfx::UniqueDrawable no_tile;

const gfx::UniqueDrawable& TileLayerGroup::getDrawable(mbgl::RenderPass pass, const OverscaledTileID& id) const {
    const auto hit = impl->tileDrawables.find({pass, id});
    return (hit == impl->tileDrawables.end()) ? no_tile : hit->second;
}

gfx::UniqueDrawable TileLayerGroup::removeDrawable(mbgl::RenderPass pass, const OverscaledTileID& id) {
    const auto hit = impl->tileDrawables.find({pass, id});
    if (hit == impl->tileDrawables.end()) {
        return {};
    }
    auto drawable = std::move(hit->second);
    impl->tileDrawables.erase(hit);
    return drawable;
}

bool TileLayerGroup::addDrawable(mbgl::RenderPass pass, const OverscaledTileID& id, gfx::UniqueDrawable&& drawable) {
    const auto result = impl->tileDrawables.insert(
        std::make_pair(TileLayerGroupTileKey{pass, id}, gfx::UniqueDrawable()));
    if (result.second) {
        return false;
    } else {
        result.first->second = std::move(drawable);
        return true;
    }
}

void TileLayerGroup::render([[maybe_unused]] RenderOrchestrator& orchestrator,
                            [[maybe_unused]] PaintParameters& parameters) {}

} // namespace mbgl
