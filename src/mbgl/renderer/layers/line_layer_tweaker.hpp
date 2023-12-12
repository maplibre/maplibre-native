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

    LineLayerTweaker(std::string id_, Immutable<style::LayerProperties> properties)
        : LayerTweaker(std::move(id_), properties) {}

    ~LineLayerTweaker() override = default;

    void execute(LayerGroupBase&, const PaintParameters&) override;

protected:
    gfx::UniformBufferPtr linePropertiesBuffer;
    gfx::UniformBufferPtr lineGradientPropertiesBuffer;
    gfx::UniformBufferPtr linePatternPropertiesBuffer;
    gfx::UniformBufferPtr lineSDFPropertiesBuffer;
    gfx::UniformBufferPtr dynamicBuffer;

#if MLN_RENDER_BACKEND_METAL
    gfx::UniformBufferPtr permutationUniformBuffer;
    gfx::UniformBufferPtr expressionUniformBuffer;
#endif // MLN_RENDER_BACKEND_METAL
};

} // namespace mbgl
