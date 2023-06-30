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

    using TileMap = std::unordered_multimap<TileLayerGroupTileKey, gfx::UniqueDrawable, TileLayerGroupTileKey::hash>;
    TileMap tileDrawables;
};

LayerGroupBase::LayerGroupBase(int32_t layerIndex_, std::string name_)
    : layerIndex(layerIndex_),
      name(std::move(name_)) {}

TileLayerGroup::TileLayerGroup(int32_t layerIndex_, std::size_t initialCapacity, std::string name_)
    : LayerGroupBase(layerIndex_, std::move(name_)),
      impl(std::make_unique<Impl>(initialCapacity)) {}

TileLayerGroup::~TileLayerGroup() {}

std::size_t TileLayerGroup::getDrawableCount() const {
    return impl->tileDrawables.size();
}

static const gfx::UniqueDrawable no_tile;

std::size_t TileLayerGroup::getDrawableCount(mbgl::RenderPass pass, const OverscaledTileID& id) const {
    const auto range = impl->tileDrawables.equal_range({pass, id});
    return std::distance(range.first, range.second);
}

std::vector<gfx::UniqueDrawable> TileLayerGroup::removeDrawables(mbgl::RenderPass pass, const OverscaledTileID& id) {
    const auto range = impl->tileDrawables.equal_range({pass, id});
    std::vector<gfx::UniqueDrawable> result(std::distance(range.first, range.second));
    std::transform(
        std::make_move_iterator(range.first), std::make_move_iterator(range.second), result.begin(), [](auto&& pair) {
            return std::move(pair.second);
        });
    impl->tileDrawables.erase(range.first, range.second);
    return result;
}

void TileLayerGroup::addDrawable(mbgl::RenderPass pass, const OverscaledTileID& id, gfx::UniqueDrawable&& drawable) {
    impl->tileDrawables.insert(std::make_pair(TileLayerGroupTileKey{pass, id}, std::move(drawable)));
}

void TileLayerGroup::observeDrawables(std::function<void(gfx::Drawable&)> f) {
    for (auto& pair : impl->tileDrawables) {
        if (pair.second) {
            f(*pair.second);
        }
    }
}

void TileLayerGroup::observeDrawables(std::function<void(const gfx::Drawable&)> f) const {
    for (const auto& pair : impl->tileDrawables) {
        if (pair.second) {
            f(*pair.second);
        }
    }
}

void TileLayerGroup::observeDrawables(std::function<void(gfx::UniqueDrawable&)> f) {
    for (auto i = impl->tileDrawables.begin(); i != impl->tileDrawables.end();) {
        auto& drawable = i->second;
        if (drawable) {
            f(drawable);
        }
        if (drawable) {
            // Not removed, keep going
            ++i;
        } else {
            // Removed, take it out of the map
            i = impl->tileDrawables.erase(i);
        }
    }
}

void TileLayerGroup::observeDrawables(mbgl::RenderPass pass,
                                      const OverscaledTileID& tileID,
                                      std::function<void(gfx::Drawable&)>&& f) {
    const auto range = impl->tileDrawables.equal_range({pass, tileID});
    std::for_each(range.first, range.second, [&f](const auto& pair) { f(*pair.second); });
}

void TileLayerGroup::observeDrawables(mbgl::RenderPass pass,
                                      const OverscaledTileID& tileID,
                                      std::function<void(const gfx::Drawable&)>&& f) const {
    const auto range = impl->tileDrawables.equal_range({pass, tileID});
    std::for_each(range.first, range.second, [&f](const auto& pair) { f(*pair.second); });
}

std::size_t TileLayerGroup::clearDrawables() {
    const auto count = impl->tileDrawables.size();
    impl->tileDrawables.clear();
    return count;
}

} // namespace mbgl
