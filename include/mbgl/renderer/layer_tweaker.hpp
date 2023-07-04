#pragma once

#include <mbgl/util/immutable.hpp>

#include <array>
#include <memory>

namespace mbgl {
namespace gfx {
class UniformBuffer;
using UniformBufferPtr = std::shared_ptr<UniformBuffer>;
} // namespace gfx
namespace style {
class LayerProperties;
enum class TranslateAnchorType : bool;
} // namespace style
class TransformState;
class LayerGroupBase;
class PaintParameters;
class RenderTree;
class UnwrappedTileID;

using mat4 = std::array<double, 16>;

/**
    Base class for layer tweakers, which manipulate layer group per frame
 */
class LayerTweaker {
protected:
    LayerTweaker(Immutable<style::LayerProperties> properties);

public:
    LayerTweaker() = delete;
    virtual ~LayerTweaker() = default;

    virtual void execute(LayerGroupBase&, const RenderTree&, const PaintParameters&) = 0;

protected:
    static mat4 getTileMatrix(const UnwrappedTileID&,
                              const RenderTree&,
                              const TransformState&,
                              const std::array<float, 2>& translation,
                              style::TranslateAnchorType,
                              bool inViewportPixelUnits,
                              bool aligned = false);

protected:
    Immutable<style::LayerProperties> evaluatedProperties;
};

} // namespace mbgl
