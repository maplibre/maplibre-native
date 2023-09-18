#pragma once

#include <mbgl/renderer/layer_tweaker.hpp>
#include <mbgl/util/string_indexer.hpp>

#include <string>

namespace mbgl {

/**
    Fill layer specific tweaker
 */
class FillLayerTweaker : public LayerTweaker {
public:
    FillLayerTweaker(std::string id_, Immutable<style::LayerProperties> properties)
        : LayerTweaker(std::move(id_), properties) {}

public:
    ~FillLayerTweaker() override = default;

    void execute(LayerGroupBase&, const RenderTree&, const PaintParameters&) override;

    static const StringIdentity idFillTilePropsUBOName;
    static const StringIdentity idFillInterpolateUBOName;
    static const StringIdentity idFillOutlineInterpolateUBOName;

private:
    gfx::UniformBufferPtr fillPropsUniformBuffer;
    gfx::UniformBufferPtr fillOutlinePropsUniformBuffer;
    gfx::UniformBufferPtr fillPatternPropsUniformBuffer;
    gfx::UniformBufferPtr fillOutlinePatternPropsUniformBuffer;

    gfx::UniformBufferPtr fillPermutationUniformBuffer;
    gfx::UniformBufferPtr fillOutlinePermutationUniformBuffer;
    gfx::UniformBufferPtr fillPatternPermutationUniformBuffer;
    gfx::UniformBufferPtr fillOutlinePatternPermutationUniformBuffer;
#if MLN_RENDER_BACKEND_METAL
    gfx::UniformBufferPtr expressionUniformBuffer;
#endif // MLN_RENDER_BACKEND_METAL
};

} // namespace mbgl
