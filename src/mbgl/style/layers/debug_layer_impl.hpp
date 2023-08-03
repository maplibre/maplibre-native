#pragma once

#include <mbgl/style/layer_impl.hpp>
#include <mbgl/style/layers/debug_layer.hpp>
#include <mbgl/style/layers/debug_layer_properties.hpp>
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

class DebugLayer::Impl : public Layer::Impl {
public:
    using Layer::Impl::Impl;
    Impl(const Impl&);

    bool hasLayoutDifference(const Layer::Impl&) const override;
    void stringifyLayout(rapidjson::Writer<rapidjson::StringBuffer>&) const override;

public:
    DebugPaintProperties::Transitionable paint;

    DECLARE_LAYER_TYPE_INFO;
};

} // namespace style
} // namespace mbgl
