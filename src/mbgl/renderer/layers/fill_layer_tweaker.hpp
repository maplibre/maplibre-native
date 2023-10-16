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

    using Unevaluated = style::FillPaintProperties::Unevaluated;
    using PropertyMask = style::FillPaintProperties::PropertyMaskType;

    /// Apply the current state of the un-evaluated properties.
    /// @return A bitset with 1 where an expression was set up, 0 where the property must still be evaluated.
    PropertyMask updateUnevaluated(const Unevaluated&);

    static const StringIdentity idFillTilePropsUBOName;
    static const StringIdentity idFillInterpolateUBOName;
    static const StringIdentity idFillOutlineInterpolateUBOName;

protected:
    /// Create shader expression representations from  un-evaluated properties where the corresponding mask bit is set.
    /// @return A bitset with 1 where an expression was set up, 0 where the property must still be evaluated.
    PropertyMask buildPropertyExpressions(const Unevaluated&, const PropertyMask&);

    PropertyMask getTransitionMask(const Unevaluated&) const;

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

    PropertyMask expressionMask = {0};
    PropertyMask rebuildExpressionsMask = {~0LL};
#endif // MLN_RENDER_BACKEND_METAL
};

} // namespace mbgl
