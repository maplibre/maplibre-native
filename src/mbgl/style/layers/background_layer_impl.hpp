#pragma once

#include <mbgl/style/layer_impl.hpp>
#include <mbgl/style/layers/background_layer.hpp>
#include <mbgl/style/layers/background_layer_properties.hpp>
#include <mbgl/tile/tile_id.hpp>

#include <memory>
#include <mutex>
#include <unordered_map>

namespace mbgl {
namespace gfx {

class Drawable;
class ShaderProgramBase;
class ShaderRegistry;

using DrawablePtr = std::shared_ptr<Drawable>;
using ShaderProgramBasePtr = std::shared_ptr<ShaderProgramBase>;

} // namespace gfx

namespace style {

class BackgroundLayer::Impl : public Layer::Impl {
public:
    using Layer::Impl::Impl;
    Impl(const Impl&);

    bool hasLayoutDifference(const Layer::Impl&) const override;
    void stringifyLayout(rapidjson::Writer<rapidjson::StringBuffer>&) const override;

    void layerAdded(gfx::ShaderRegistry&,
                    gfx::Context&,
                    const TransformState&,
                    UniqueChangeRequestVec&) const override;
    void layerRemoved(UniqueChangeRequestVec&) const override;

    /// Generate any changes needed by the layer
    void update(int32_t layerIndex,
                gfx::Context&,
                const TransformState&,
                UniqueChangeRequestVec&) const override;

    void setUnevaluated(BackgroundPaintProperties::Unevaluated value) {
        unevaluatedProperties = value;
    }
    void setEvaluated(Immutable<BackgroundPaintProperties::PossiblyEvaluated> value) {
        evaluatedProperties = std::move(value);
        ++stats.propertyEvaluations;
    }

private:
    mutable std::optional<Color> lastColor;
    mutable int32_t lastLayerIndex = -1;

    std::optional<BackgroundPaintProperties::Unevaluated> unevaluatedProperties;
    // Latest evaluated properties.
    std::optional<Immutable<BackgroundPaintProperties::PossiblyEvaluated>> evaluatedProperties;

public:
    BackgroundPaintProperties::Transitionable paint;

    DECLARE_LAYER_TYPE_INFO;
};

} // namespace style
} // namespace mbgl
