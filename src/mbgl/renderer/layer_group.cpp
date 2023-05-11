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

void TileLayerGroup::render([[maybe_unused]] RenderOrchestrator& orchestrator,
                            [[maybe_unused]] PaintParameters& parameters) {
    auto pass = mbgl::RenderPass::Opaque;
    auto id = OverscaledTileID{0, 0, 0, 0, 0};
    auto key = TileLayerGroupTileKey{pass, id};
    impl->tileDrawables.insert(std::make_pair(key, gfx::UniqueDrawable()));
}

} // namespace mbgl
