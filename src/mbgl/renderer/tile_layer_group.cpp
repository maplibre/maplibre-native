#include <mbgl/renderer/layer_group.hpp>

#include <mbgl/gfx/upload_pass.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/render_orchestrator.hpp>

#include <unordered_map>
#include <set>

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
    Impl(std::size_t initialCapacity)
        : drawablesByTile(initialCapacity) {}

    using TileMap = std::unordered_multimap<TileLayerGroupTileKey, gfx::UniqueDrawable, TileLayerGroupTileKey::hash>;
    TileMap drawablesByTile;

    using DrawableMap = std::set<gfx::Drawable*, gfx::DrawableLessByPriority>;
    DrawableMap sortedDrawables;
};

LayerGroupBase::LayerGroupBase(int32_t layerIndex_, std::string name_)
    : layerIndex(layerIndex_),
      name(std::move(name_)) {}

TileLayerGroup::TileLayerGroup(int32_t layerIndex_, std::size_t initialCapacity, std::string name_)
    : LayerGroupBase(layerIndex_, std::move(name_)),
      impl(std::make_unique<Impl>(initialCapacity)) {}

TileLayerGroup::~TileLayerGroup() = default;

std::size_t TileLayerGroup::getDrawableCount() const {
    return impl->drawablesByTile.size();
}

static const gfx::UniqueDrawable no_tile;

std::size_t TileLayerGroup::getDrawableCount(mbgl::RenderPass pass, const OverscaledTileID& id) const {
    assert(impl->drawablesByTile.size() == impl->sortedDrawables.size());
    const auto range = impl->drawablesByTile.equal_range({pass, id});
    return std::distance(range.first, range.second);
}

std::vector<gfx::UniqueDrawable> TileLayerGroup::removeDrawables(mbgl::RenderPass pass, const OverscaledTileID& id) {
    assert(impl->drawablesByTile.size() == impl->sortedDrawables.size());
    const auto range = impl->drawablesByTile.equal_range({pass, id});
    std::vector<gfx::UniqueDrawable> result(std::distance(range.first, range.second));
    std::transform(
        std::make_move_iterator(range.first), std::make_move_iterator(range.second), result.begin(), [](auto&& pair) {
            return std::move(pair.second);
        });
    impl->drawablesByTile.erase(range.first, range.second);
    std::for_each(result.begin(), result.end(), [&](const auto& item) {
        const auto hit = impl->sortedDrawables.find(item.get());
        assert(hit != impl->sortedDrawables.end());
        if (hit != impl->sortedDrawables.end()) {
            impl->sortedDrawables.erase(hit);
        }
    });
    return result;
}

void TileLayerGroup::addDrawable(mbgl::RenderPass pass, const OverscaledTileID& id, gfx::UniqueDrawable&& drawable) {
    assert(impl->drawablesByTile.size() == impl->sortedDrawables.size());
    if (drawable) {
        [[maybe_unused]] const auto result = impl->sortedDrawables.insert(drawable.get());
        assert(result.second);
        impl->drawablesByTile.insert(std::make_pair(TileLayerGroupTileKey{pass, id}, std::move(drawable)));
    }
}

std::size_t TileLayerGroup::observeDrawables(const std::function<void(gfx::Drawable&)>&& f) {
    assert(impl->drawablesByTile.size() == impl->sortedDrawables.size());
    for (auto* drawable : impl->sortedDrawables) {
        f(*drawable);
    }
    return impl->sortedDrawables.size();
}

std::size_t TileLayerGroup::observeDrawables(const std::function<void(const gfx::Drawable&)>&& f) const {
    assert(impl->drawablesByTile.size() == impl->sortedDrawables.size());
    for (const auto* drawable : impl->sortedDrawables) {
        f(*drawable);
    }
    return impl->sortedDrawables.size();
}

std::size_t TileLayerGroup::observeDrawablesRemove(const std::function<bool(gfx::Drawable&)>&& f) {
    const auto oldSize = impl->drawablesByTile.size();
    for (auto i = impl->drawablesByTile.begin(); i != impl->drawablesByTile.end();) {
        auto& drawable = i->second;
        if (f(*drawable)) {
            // Not removed, keep going
            ++i;
        } else {
            // Removed, take it out of the collections
            impl->sortedDrawables.erase(drawable.get());
            i = impl->drawablesByTile.erase(i);
        }
        assert(impl->drawablesByTile.size() == impl->sortedDrawables.size());
    }
    return (oldSize - impl->drawablesByTile.size());
}

std::size_t TileLayerGroup::observeDrawables(mbgl::RenderPass pass,
                                             const OverscaledTileID& tileID,
                                             const std::function<void(gfx::Drawable&)>&& f) {
    assert(impl->drawablesByTile.size() == impl->sortedDrawables.size());
    const auto range = impl->drawablesByTile.equal_range({pass, tileID});
    std::for_each(range.first, range.second, [&f](const auto& pair) { f(*pair.second); });
    return std::distance(range.first, range.second);
}

std::size_t TileLayerGroup::observeDrawables(mbgl::RenderPass pass,
                                             const OverscaledTileID& tileID,
                                             const std::function<void(const gfx::Drawable&)>&& f) const {
    assert(impl->drawablesByTile.size() == impl->sortedDrawables.size());
    const auto range = impl->drawablesByTile.equal_range({pass, tileID});
    std::for_each(range.first, range.second, [&f](const auto& pair) { f(*pair.second); });
    return std::distance(range.first, range.second);
}

std::size_t TileLayerGroup::clearDrawables() {
    const auto count = impl->drawablesByTile.size();
    assert(count == impl->sortedDrawables.size());
    impl->sortedDrawables.clear();
    impl->drawablesByTile.clear();
    return count;
}

} // namespace mbgl
