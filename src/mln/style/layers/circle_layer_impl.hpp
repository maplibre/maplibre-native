#pragma once

#include <mln/style/layer_impl.hpp>
#include <mln/style/layers/circle_layer.hpp>
#include <mln/style/layers/circle_layer_properties.hpp>

namespace mln {
namespace style {

class CircleLayer::Impl : public Layer::Impl {
public:
    using Layer::Impl::Impl;

    bool hasLayoutDifference(const Layer::Impl&) const override;
    void stringifyLayout(rapidjson::Writer<rapidjson::StringBuffer>&) const override;

    expression::Dependency getLayoutDependencies() const noexcept override {
        return layout.getDependencies() | Layer::Impl::getLayoutDependencies();
    }

    void collectLayoutGlobalStateRefs(std::set<std::string>& refs) const override {
        layout.collectGlobalStateRefs(refs);
        Layer::Impl::collectLayoutGlobalStateRefs(refs);
    }

    CircleLayoutProperties::Unevaluated layout;
    CirclePaintProperties::Transitionable paint;

    DECLARE_LAYER_TYPE_INFO;
};

} // namespace style
} // namespace mln
