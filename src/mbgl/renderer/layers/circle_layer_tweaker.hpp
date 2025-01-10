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
    CircleLayerTweaker(std::string id_, Immutable<style::LayerProperties> properties)
        : LayerTweaker(std::move(id_), properties) {}
    ~CircleLayerTweaker() override = default;

    void execute(LayerGroupBase&, const PaintParameters&) override;

protected:
    gfx::UniformBufferPtr evaluatedPropsUniformBuffer;

#if MLN_UBO_CONSOLIDATION
    gfx::UniformBufferPtr drawableUniformBuffer;
#endif
};

} // namespace mbgl
