#include <mbgl/renderer/group_by_layout.hpp>
#include <mbgl/util/instrumentation.hpp>
#include <mbgl/util/containers.hpp>
#include <mbgl/style/layer_properties.hpp>

#include <string>

namespace mbgl {

std::string createLayoutKey(const style::Layer::Impl& impl) {
    using namespace style::conversion;
    std::string key;
    key += std::to_string(reinterpret_cast<uint64_t>(impl.getTypeInfo()));
    key += impl.source;
    key += impl.sourceLayer;
    key += std::to_string(impl.maxZoom);
    key += std::to_string(impl.minZoom);
    key += std::to_string(static_cast<uint32_t>(impl.visibility));
    return key;
}

GroupMap groupLayers(const std::vector<Immutable<style::LayerProperties>>& layers) {
    MLN_TRACE_FUNC();

    mbgl::unordered_map<std::string, std::vector<Immutable<style::LayerProperties>>> groupMap;
    groupMap.reserve(layers.size());

    for (auto layer : layers) {
        // this part of the key is unique per filter
        auto layoutFilterKey = 0;
        while (true) {
            auto layoutKey = createLayoutKey(*layer->baseImpl) + std::to_string(layoutFilterKey);
            if (groupMap.contains(layoutKey)) {
                if (groupMap[layoutKey].front()->baseImpl->filter != layer->baseImpl->filter) {
                    ++layoutFilterKey;
                    continue;
                }
            }
            groupMap[layoutKey].push_back(std::move(layer));
            break;
        }
    }
    return groupMap;
}

} // namespace mbgl
