#include <mbgl/renderer/layer_group.hpp>

#include <mbgl/gfx/upload_pass.hpp>

#include <algorithm>
#include <unordered_map>
#include <set>

namespace mbgl {

LayerGroupBase::LayerGroupBase(int32_t layerIndex_, std::string name_, Type type_)
    : type(type_),
      layerIndex(layerIndex_),
      name(std::move(name_)) {}

TileLayerGroup::TileLayerGroup(int32_t layerIndex_, std::size_t initialCapacity, std::string name_)
    : LayerGroupBase(layerIndex_, std::move(name_), LayerGroupBase::Type::TileLayerGroup) {
    drawablesByTile.reserve(initialCapacity);
}

TileLayerGroup::~TileLayerGroup() = default;

std::size_t TileLayerGroup::getDrawableCount() const {
    return drawablesByTile.size();
}

static const gfx::UniqueDrawable no_tile;

std::size_t TileLayerGroup::getDrawableCount(mbgl::RenderPass pass, const OverscaledTileID& id) const {
    assert(drawablesByTile.size() == sortedDrawables.size());
    const auto range = drawablesByTile.equal_range({pass, id});
    return std::distance(range.first, range.second);
}

std::vector<gfx::UniqueDrawable> TileLayerGroup::removeDrawables(mbgl::RenderPass pass, const OverscaledTileID& id) {
    assert(drawablesByTile.size() == sortedDrawables.size());
    const auto range = drawablesByTile.equal_range({pass, id});
    std::vector<gfx::UniqueDrawable> result(std::distance(range.first, range.second));
    std::transform(
        std::make_move_iterator(range.first), std::make_move_iterator(range.second), result.begin(), [](auto&& pair) {
            return std::move(pair.second);
        });
    drawablesByTile.erase(range.first, range.second);
    std::ranges::for_each(result, [&](const auto& item) {
        const auto hit = sortedDrawables.find(item.get());
        assert(hit != sortedDrawables.end());
        if (hit != sortedDrawables.end()) {
            sortedDrawables.erase(hit);
        }
    });
    return result;
}

void TileLayerGroup::addDrawable(mbgl::RenderPass pass, const OverscaledTileID& id, gfx::UniqueDrawable&& drawable) {
    assert(drawablesByTile.size() == sortedDrawables.size());

    if (drawable) {
        LayerGroupBase::addDrawable(drawable);
        [[maybe_unused]] const auto result = sortedDrawables.insert(drawable.get());
        assert(result.second);
        drawablesByTile.insert(
            std::make_pair(TileLayerGroupTileKey{.renderPass = pass, .tileID = id}, std::move(drawable)));
    }
}

std::size_t TileLayerGroup::clearDrawables() {
    const auto count = drawablesByTile.size();
    assert(count == sortedDrawables.size());
    sortedDrawables.clear();
    drawablesByTile.clear();
    return count;
}

void TileLayerGroup::setStencilTiles(RenderTiles tiles) {
    stencilTiles = std::move(tiles);
}

} // namespace mbgl
