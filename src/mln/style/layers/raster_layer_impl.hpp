#pragma once

#include <mln/style/layer_impl.hpp>
#include <mln/style/layers/raster_layer.hpp>
#include <mln/style/layers/raster_layer_properties.hpp>

namespace mln {
namespace style {

class RasterLayer::Impl : public Layer::Impl {
public:
    using Layer::Impl::Impl;

    bool hasLayoutDifference(const Layer::Impl&) const override;
    void stringifyLayout(rapidjson::Writer<rapidjson::StringBuffer>&) const override;

    RasterPaintProperties::Transitionable paint;

    DECLARE_LAYER_TYPE_INFO;
};

} // namespace style
} // namespace mln
