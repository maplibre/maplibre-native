#pragma once

#include <mbgl/renderer/layer_tweaker.hpp>

#include <string>
#include <vector>

namespace mbgl {

/**
    Circle layer specific tweaker
 */
class CircleLayerTweaker : public LayerTweaker {
public:
    CircleLayerTweaker(Immutable<style::LayerProperties> properties)
        : LayerTweaker(properties){};

public:
    ~CircleLayerTweaker() override = default;

    void setPropertiesAsUniforms(std::vector<std::string>);

    void execute(LayerGroupBase&, const RenderTree&, const PaintParameters&) override;

protected:
    gfx::UniformBufferPtr paintParamsUniformBuffer;
    gfx::UniformBufferPtr evaluatedPropsUniformBuffer;

#if MLN_RENDER_BACKEND_METAL
    gfx::UniformBufferPtr permutationUniformBuffer;
    gfx::UniformBufferPtr expressionUniformBuffer;

    std::vector<std::string> propertiesAsUniforms;
    bool propertiesChanged = true;
#endif // MLN_RENDER_BACKEND_METAL
};

} // namespace mbgl
