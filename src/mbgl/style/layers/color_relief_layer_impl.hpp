#pragma once

#include <mbgl/style/layer_impl.hpp>
#include <mbgl/style/layers/color_relief_layer.hpp>
#include <mbgl/style/layers/color_relief_layer_properties.hpp>

namespace mbgl {
namespace style {

bool ColorReliefLayer::Impl::hasLayoutDifference(const Layer::Impl&) const {
    return false;
}

void ColorReliefLayer::Impl::stringifyLayout(rapidjson::Writer<rapidjson::StringBuffer>&) const {
    // No layout properties for color-relief
}

} // namespace style
} // namespace mbgl
