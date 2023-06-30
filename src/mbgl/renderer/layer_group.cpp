#include <mbgl/renderer/layer_group.hpp>

#include <mbgl/gfx/upload_pass.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/render_orchestrator.hpp>

#include <unordered_map>

namespace mbgl {

struct LayerGroup::Impl {
    Impl(std::size_t capacity) { drawables.reserve(capacity); }

    using DrawableCollection = std::vector<gfx::UniqueDrawable>;
    DrawableCollection drawables;
};

LayerGroup::LayerGroup(int32_t layerIndex_, std::size_t initialCapacity, std::string name_)
    : LayerGroupBase(layerIndex_, std::move(name_)),
      impl(std::make_unique<Impl>(initialCapacity)) {}

LayerGroup::~LayerGroup() {}

std::size_t LayerGroup::getDrawableCount() const {
    return impl->drawables.size();
}

void LayerGroup::addDrawable(gfx::UniqueDrawable&& drawable) {
    impl->drawables.emplace_back(std::move(drawable));
}

void LayerGroup::observeDrawables(std::function<void(gfx::Drawable&)> f) {
    for (const auto& item : impl->drawables) {
        if (item) {
            f(*item);
        }
    }
}

void LayerGroup::observeDrawables(std::function<void(const gfx::Drawable&)> f) const {
    for (const auto& item : impl->drawables) {
        if (item) {
            f(*item);
        }
    }
}

void LayerGroup::observeDrawables(std::function<void(gfx::UniqueDrawable&)> f) {
    for (auto i = impl->drawables.begin(); i != impl->drawables.end();) {
        auto& drawable = *i;
        if (drawable) {
            f(drawable);
        }
        if (drawable) {
            // Not removed, keep going
            ++i;
        } else {
            // Removed, take it out of the map
            i = impl->drawables.erase(i);
        }
    }
}

std::size_t LayerGroup::clearDrawables() {
    const auto count = impl->drawables.size();
    impl->drawables.clear();
    return count;
}

} // namespace mbgl
