#include <mbgl/renderer/layer_group.hpp>

#include <mbgl/gfx/upload_pass.hpp>
#include <mbgl/gfx/drawable_tweaker.hpp>
#include <mbgl/renderer/layer_tweaker.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/render_orchestrator.hpp>
#include <mbgl/renderer/render_tree.hpp>

namespace mbgl {

void LayerGroupBase::addDrawable(gfx::UniqueDrawable& drawable) {
    drawable->setLayerIndex(layerIndex);
    // init their tweakers
    for (const auto& tweaker : drawable->getTweakers()) {
        tweaker->init(*drawable);
    }
}

void LayerGroupBase::runTweakers(const RenderTree&, PaintParameters& parameters) {
    for (auto it = layerTweakers.begin(); it != layerTweakers.end();) {
        if (auto tweaker = it->lock()) {
            tweaker->execute(*this, parameters);
            ++it;
        } else {
            it = layerTweakers.erase(it);
        }
    }
}

LayerGroup::LayerGroup(int32_t layerIndex_, std::size_t /*initialCapacity*/, std::string name_)
    : LayerGroupBase(layerIndex_, std::move(name_), LayerGroupBase::Type::LayerGroup) {}

LayerGroup::~LayerGroup() = default;

std::size_t LayerGroup::getDrawableCount() const {
    return drawables.size();
}

void LayerGroup::addDrawable(gfx::UniqueDrawable&& drawable) {
    LayerGroupBase::addDrawable(drawable);
    drawables.emplace(std::move(drawable));
}

std::size_t LayerGroup::clearDrawables() {
    const auto count = drawables.size();
    drawables.clear();
    return count;
}

} // namespace mbgl
