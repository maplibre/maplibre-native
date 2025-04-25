//
//  plugin_layer_impl.cpp
//  App
//
//  Created by Malcolm Toon on 4/25/25.
//

#include "plugin_layer_impl.hpp"
#include <iostream>

namespace mbgl {
namespace style {

PluginLayer::Impl::Impl(std::string layerID, std::string sourceID, LayerTypeInfo layerTypeInfo)
    : Layer::Impl(layerID, sourceID),
      _layerTypeInfo(layerTypeInfo) {
    std::cout << "Init\n";
}

bool PluginLayer::Impl::hasLayoutDifference(const Layer::Impl& other) const {
    // TODO: Implement this
    return false;
    // assert(other.getTypeInfo() == getTypeInfo());
    //    const auto& impl = static_cast<const style::PluginLayer::Impl&>(other);
    //    return filter != impl.filter || visibility != impl.visibility ||
    //    paint.hasDataDrivenPropertyDifference(impl.paint);
}

} // namespace style
} // namespace mbgl
