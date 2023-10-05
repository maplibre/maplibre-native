#pragma once

#include <mbgl/renderer/layer_tweaker.hpp>
#include <mbgl/shaders/layer_ubo.hpp>
#include <mbgl/util/string_indexer.hpp>

#if MLN_RENDER_BACKEND_METAL
#include <mbgl/style/layers/fill_layer_properties.hpp>
#endif // MLN_RENDER_BACKEND_METAL

#include <string>

namespace mbgl {

/**
    Fill layer specific tweaker
 */
class FillLayerTweaker : public LayerTweaker {
public:
    FillLayerTweaker(std::string id_, Immutable<style::LayerProperties> properties)
        : LayerTweaker(std::move(id_), properties) {}
    ~FillLayerTweaker() override = default;

    void execute(LayerGroupBase&, const PaintParameters&) override;

#if MLN_RENDER_BACKEND_METAL
    void buildAttributeExpressions(const style::FillPaintProperties::Unevaluated&);
#endif // MLN_RENDER_BACKEND_METAL

    static const StringIdentity idFillTilePropsUBOName;
    static const StringIdentity idFillInterpolateUBOName;
    static const StringIdentity idFillOutlineInterpolateUBOName;

protected:
    shaders::ExpressionAttribute getAttribute(const StringIdentity attrNameID,
                                              const std::optional<shaders::Expression>&);
    shaders::ColorAttribute getAttribute(const StringIdentity attrNameID,
                                         const std::optional<shaders::ColorExpression>&);

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

    std::optional<shaders::ColorExpression> fillColorExpr;
    std::optional<shaders::Expression> opacityExpr;
#endif // MLN_RENDER_BACKEND_METAL
};

} // namespace mbgl
