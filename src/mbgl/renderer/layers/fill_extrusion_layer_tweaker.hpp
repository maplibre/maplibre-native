#pragma once

#include <mbgl/renderer/layer_tweaker.hpp>

#include <string>

namespace mbgl {

/**
    Fill extrusion layer specific tweaker
 */
class FillExtrusionLayerTweaker : public LayerTweaker {
public:
    FillExtrusionLayerTweaker(std::string id_, Immutable<style::LayerProperties> properties)
        : LayerTweaker(std::move(id_), properties) {}

public:
    ~FillExtrusionLayerTweaker() override = default;

    void execute(LayerGroupBase&, const PaintParameters&) override;

protected:
    gfx::UniformBufferPtr evaluatedPropsUniformBuffer;

#if MLN_UBO_CONSOLIDATION
    gfx::UniformBufferPtr drawableUniformBuffer;
    gfx::UniformBufferPtr tilePropsUniformBuffer;
#endif
};

} // namespace mbgl
