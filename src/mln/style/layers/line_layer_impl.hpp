#pragma once

#include <mln/style/layer_impl.hpp>
#include <mln/style/layers/line_layer.hpp>
#include <mln/style/layers/line_layer_properties.hpp>

namespace mln {
namespace style {

class LineLayer::Impl : public Layer::Impl {
public:
    using Layer::Impl::Impl;

    bool hasLayoutDifference(const Layer::Impl&) const override;
    void stringifyLayout(rapidjson::Writer<rapidjson::StringBuffer>&) const override;

    expression::Dependency getDependencies() const noexcept override {
        return layout.getDependencies() | paint.getDependencies();
    }

    expression::Dependency getLayoutDependencies() const noexcept override {
        return layout.getDependencies() | Layer::Impl::getLayoutDependencies();
    }

    void collectLayoutGlobalStateRefs(std::set<std::string>& refs) const override {
        layout.collectGlobalStateRefs(refs);
        Layer::Impl::collectLayoutGlobalStateRefs(refs);
    }

    LineLayoutProperties::Unevaluated layout;
    LinePaintProperties::Transitionable paint;

    // Features with a fade zoom fade out in a replaced tile, so the tile stays on screen until they have.
    bool drawsFadingTiles() const noexcept override { return !paint.get<LineFadeZoom>().value.isUndefined(); }

    DECLARE_LAYER_TYPE_INFO;
};

} // namespace style
} // namespace mln
