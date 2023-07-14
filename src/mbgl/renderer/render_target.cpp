#include <mbgl/renderer/render_target.hpp>

#include <mbgl/gfx/context.hpp>
#include <mbgl/renderer/layer_group.hpp>

namespace mbgl {

bool RenderTarget::addLayerGroup(LayerGroupBasePtr layerGroup, const bool replace) {
    const auto index = layerGroup->getLayerIndex();
    const auto result = layerGroupsByLayerIndex.insert(std::make_pair(index, LayerGroupBasePtr{}));
    if (result.second) {
        // added
        result.first->second = std::move(layerGroup);
        return true;
    } else {
        // not added
        if (replace) {
            result.first->second = std::move(layerGroup);
            return true;
        } else {
            return false;
        }
    }
}

bool RenderTarget::removeLayerGroup(const int32_t layerIndex) {
    const auto hit = layerGroupsByLayerIndex.find(layerIndex);
    if (hit != layerGroupsByLayerIndex.end()) {
        layerGroupsByLayerIndex.erase(hit);
        return true;
    } else {
        return false;
    }
}

size_t RenderTarget::numLayerGroups() const noexcept {
    return layerGroupsByLayerIndex.size();
}

static const LayerGroupBasePtr no_group;

const LayerGroupBasePtr& RenderTarget::getLayerGroup(const int32_t layerIndex) const {
    const auto hit = layerGroupsByLayerIndex.find(layerIndex);
    return (hit == layerGroupsByLayerIndex.end()) ? no_group : hit->second;
}

void RenderTarget::observeLayerGroups(std::function<void(LayerGroupBase&)> f) {
    for (auto& pair : layerGroupsByLayerIndex) {
        if (pair.second) {
            f(*pair.second);
        }
    }
}

void RenderTarget::observeLayerGroups(std::function<void(const LayerGroupBase&)> f) const {
    for (const auto& pair : layerGroupsByLayerIndex) {
        if (pair.second) {
            f(*pair.second);
        }
    }
}

} // namespace mbgl
