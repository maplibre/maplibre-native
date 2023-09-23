#pragma once

#include <mbgl/renderer/layer_tweaker.hpp>

#include <string_view>

namespace mbgl {

namespace gfx {
class UniformBuffer;
using UniformBufferPtr = std::shared_ptr<UniformBuffer>;
} // namespace gfx

/**
    Line layer specific tweaker
 */
class LineLayerTweaker : public LayerTweaker {
public:
    enum class LineType {
        Simple,
        Pattern,
        Gradient,
        SDF
    };

public:
    LineLayerTweaker(Immutable<style::LayerProperties> properties)
        : LayerTweaker(properties){};

    ~LineLayerTweaker() override = default;

    void execute(LayerGroupBase&, const RenderTree&, const PaintParameters&) override;

protected:
    gfx::UniformBufferPtr linePropertiesBuffer;
    gfx::UniformBufferPtr lineGradientPropertiesBuffer;
    gfx::UniformBufferPtr linePatternPropertiesBuffer;
    gfx::UniformBufferPtr lineSDFPropertiesBuffer;
};

} // namespace mbgl
