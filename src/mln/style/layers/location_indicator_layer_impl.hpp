#pragma once

#include <array>
#include <mln/map/transform_state.hpp>
#include <mln/renderer/paint_parameters.hpp>
#include <mln/style/layer_impl.hpp>
#include <mln/style/layer_properties.hpp>
#include <mln/style/layers/location_indicator_layer.hpp>
#include <mln/style/layers/location_indicator_layer_properties.hpp>
#include <mln/util/color.hpp>
#include <mln/util/geo.hpp>
#include <memory>
#include <string>

namespace mln {

class TransformState;

namespace style {

class LocationIndicatorLayer::Impl : public Layer::Impl {
public:
    using Layer::Impl::Impl;

    bool hasLayoutDifference(const Layer::Impl &) const override;
    void stringifyLayout(rapidjson::Writer<rapidjson::StringBuffer> &) const override;

    LocationIndicatorLayoutProperties::Unevaluated layout;
    LocationIndicatorPaintProperties::Transitionable paint;
    DECLARE_LAYER_TYPE_INFO;
};

} // namespace style
} // namespace mln
