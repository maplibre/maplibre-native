#pragma once

#include <mbgl/style/layer_impl.hpp>
#include <mbgl/style/layers/background_layer.hpp>
#include <mbgl/style/layers/background_layer_properties.hpp>

#include <memory>

namespace mbgl {
namespace gfx {

class Drawable;
using DrawablePtr = std::shared_ptr<Drawable>;

} // namespace gfx

namespace style {

class BackgroundLayer::Impl : public Layer::Impl {
public:
    using Layer::Impl::Impl;

    bool hasLayoutDifference(const Layer::Impl&) const override;
    void stringifyLayout(rapidjson::Writer<rapidjson::StringBuffer>&) const override;

    /// Generate any changes needed by the layer
    void update(UniqueChangeRequestVec& changes) const override;

    BackgroundPaintProperties::Transitionable paint;

    mutable gfx::DrawablePtr drawable;
    
    DECLARE_LAYER_TYPE_INFO;
};

} // namespace style
} // namespace mbgl
