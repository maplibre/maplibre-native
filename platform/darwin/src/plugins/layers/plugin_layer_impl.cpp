#include "plugin_layer_impl.hpp"

namespace mbgl {
namespace style {

PluginLayer::Impl::Impl(const std::string& id_, std::unique_ptr<PluginLayerHost> host_)
    : Layer::Impl(id_, std::string()) {
    host = std::move(host_);
}

bool PluginLayer::Impl::hasLayoutDifference(const Layer::Impl&) const {
    return false;
}

void PluginLayer::Impl::stringifyLayout(rapidjson::Writer<rapidjson::StringBuffer>&) const {}

} // namespace style
} // namespace mbgl
