#pragma once

#include <mbgl/renderer/layer_tweaker.hpp>
#include <mbgl/shaders/layer_ubo.hpp>
#include <mbgl/style/layers/fill_layer_properties.hpp>
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
    ~FillLayerTweaker() override = default;

    void execute(LayerGroupBase&, const PaintParameters&) override;

    using PropertyExpressionMask = style::FillPaintProperties::PropertyMaskType;

    /// Create shader expression representations from suitable un-evaluated properties.
    /// @return A bitset with 1 where an expression was set up, 0 where the property must still be evaluated.
    PropertyExpressionMask buildPropertyExpressions(const style::FillPaintProperties::Unevaluated&);

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

    std::optional<shaders::ColorExpression> fillColorExpr;
    std::optional<shaders::Expression> opacityExpr;
#endif // MLN_RENDER_BACKEND_METAL
};

} // namespace mbgl
