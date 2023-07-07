#include <mbgl/renderer/layer_group.hpp>

#include <mbgl/gfx/upload_pass.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/render_orchestrator.hpp>

#include <set>

namespace mbgl {

struct LayerGroup::Impl {
    using DrawableCollection = std::set<gfx::UniqueDrawable, gfx::DrawableLessByPriority>;
    DrawableCollection drawables;
};

LayerGroup::LayerGroup(int32_t layerIndex_, std::size_t /*initialCapacity*/, std::string name_)
    : LayerGroupBase(layerIndex_, std::move(name_)),
      impl(std::make_unique<Impl>()) {}

LayerGroup::~LayerGroup() = default;

std::size_t LayerGroup::getDrawableCount() const {
    return impl->drawables.size();
}

void LayerGroup::addDrawable(gfx::UniqueDrawable&& drawable) {
    impl->drawables.emplace(std::move(drawable));
}

void LayerGroup::observeDrawables(const std::function<void(gfx::Drawable&)>&& f) {
    for (const auto& item : impl->drawables) {
        if (item) {
            f(*item);
        }
    }
}

void LayerGroup::observeDrawables(const std::function<void(const gfx::Drawable&)>&& f) const {
    for (const auto& item : impl->drawables) {
        if (item) {
            f(*item);
        }
    }
}

std::size_t LayerGroup::observeDrawablesRemove(const std::function<bool(gfx::Drawable&)>&& f) {
    decltype(impl->drawables) newSet;
    const auto oldSize = impl->drawables.size();
    while (!impl->drawables.empty()) {
        // set members are immutable, since changes could affect its position, so extract each item
        gfx::UniqueDrawable drawable = std::move(impl->drawables.extract(impl->drawables.begin()).value());
        if (f(*drawable)) {
            // Not removed, keep it, but in a new set so that if the key value
            // has increased, we don't see it again during this iteration.
            newSet.emplace_hint(newSet.end(), std::move(drawable));
        }
    }
    std::swap(impl->drawables, newSet);
    return (oldSize - impl->drawables.size());
}

std::size_t LayerGroup::clearDrawables() {
    const auto count = impl->drawables.size();
    impl->drawables.clear();
    return count;
}

} // namespace mbgl
