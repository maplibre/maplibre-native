#pragma once

#include <mbgl/style/layer_impl.hpp>
#include <mbgl/style/layers/background_layer.hpp>
#include <mbgl/style/layers/background_layer_properties.hpp>

#include <memory>

namespace mbgl {
namespace gfx {

class Drawable;
class ShaderProgramBase;
using DrawablePtr = std::shared_ptr<Drawable>;
using ShaderProgramBasePtr = std::shared_ptr<ShaderProgramBase>;

} // namespace gfx

namespace style {

class BackgroundLayer::Impl : public Layer::Impl {
public:
    using Layer::Impl::Impl;

    bool hasLayoutDifference(const Layer::Impl&) const override;
    void stringifyLayout(rapidjson::Writer<rapidjson::StringBuffer>&) const override;

    void layerAdded(PaintParameters&, UniqueChangeRequestVec&) const override;
    void layerRemoved(PaintParameters&, UniqueChangeRequestVec&) const override;

    /// Generate any changes needed by the layer
    void update(UniqueChangeRequestVec&) const override;

private:
    void buildDrawables(UniqueChangeRequestVec&) const;
    
public:
    BackgroundPaintProperties::Transitionable paint;

    mutable gfx::ShaderProgramBasePtr shader;
    mutable gfx::DrawablePtr drawable;
    
    DECLARE_LAYER_TYPE_INFO;
};

} // namespace style
} // namespace mbgl
