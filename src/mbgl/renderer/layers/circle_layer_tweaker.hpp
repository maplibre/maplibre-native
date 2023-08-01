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

#if MLN_RENDER_BACKEND_METAL
    void setPropertiesAsUniforms(std::vector<std::string>);
    bool hasPropertyAsUniform(std::string_view) const;

#endif // MLN_RENDER_BACKEND_METAL

    void enableOverdrawInspector(bool);

    void execute(LayerGroupBase&, const RenderTree&, const PaintParameters&) override;

protected:
    gfx::UniformBufferPtr paintParamsUniformBuffer;
    gfx::UniformBufferPtr evaluatedPropsUniformBuffer;

#if MLN_RENDER_BACKEND_METAL
    gfx::UniformBufferPtr permutationUniformBuffer;
    gfx::UniformBufferPtr expressionUniformBuffer;

    std::vector<std::string> propertiesAsUniforms;
#endif // MLN_RENDER_BACKEND_METAL

    bool propertiesChanged = true;
    bool overdrawInspector = false;
};

} // namespace mbgl
