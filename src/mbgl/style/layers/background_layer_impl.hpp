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
                    const TransformState&,
                    const PropertyEvaluationParameters&,
                    UniqueChangeRequestVec&) const override;
    void layerRemoved(UniqueChangeRequestVec&) const override;

    /// Generate any changes needed by the layer
    void update(const TransformState&,
                const PropertyEvaluationParameters&,
                UniqueChangeRequestVec&) const override;

private:
    mutable std::mutex mutex;
    mutable gfx::ShaderProgramBasePtr shader;
    mutable std::unordered_map<OverscaledTileID, gfx::DrawablePtr> tileDrawables;

    mutable struct Stats {
        size_t tileDrawablesAdded = 0;
        size_t tileDrawablesRemoved = 0;
    } stats;

public:
    BackgroundPaintProperties::Transitionable paint;

    DECLARE_LAYER_TYPE_INFO;
};

} // namespace style
} // namespace mbgl
