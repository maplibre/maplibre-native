#pragma once

#include <mbgl/renderer/layer_tweaker.hpp>
#include <mbgl/style/layers/line_layer_properties.hpp>

#if MLN_RENDER_BACKEND_METAL
#include <mbgl/shaders/line_layer_ubo.hpp>
#endif // MLN_RENDER_BACKEND_METAL

#include <string>

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
        Gradient,
        Pattern,
        SDF
    };

    LineLayerTweaker(std::string id_, Immutable<style::LayerProperties> properties)
        : LayerTweaker(std::move(id_), properties) {}

    ~LineLayerTweaker() override = default;

    void execute(LayerGroupBase&, const PaintParameters&) override;

#if MLN_RENDER_BACKEND_METAL
    using LinePaintProperties = style::LinePaintProperties;
    using Unevaluated = LinePaintProperties::Unevaluated;
    void updateGPUExpressions(const Unevaluated&, TimePoint now);

    template <typename T>
    static constexpr std::size_t propertyIndex() {
        return LinePaintProperties::Tuple<LinePaintProperties::PropertyTypes>::getIndex<T>();
    }
#endif // MLN_RENDER_BACKEND_METAL

private:
    template <typename Property>
    auto evaluate(const PaintParameters& parameters) const;

#if MLN_RENDER_BACKEND_METAL
    template <typename Result>
    std::optional<Result> gpuEvaluate(const LinePaintProperties::PossiblyEvaluated&,
                                      const PaintParameters&,
                                      const std::size_t index) const;
#endif // MLN_RENDER_BACKEND_METAL

protected:
    gfx::UniformBufferPtr evaluatedPropsUniformBuffer;

#if MLN_RENDER_BACKEND_METAL
    gfx::UniformBufferPtr expressionUniformBuffer;
    Unevaluated::GPUExpressions gpuExpressions;
    shaders::LineExpressionMask expressionMask = shaders::LineExpressionMask::None;
    bool gpuExpressionsUpdated = true;
#endif // MLN_RENDER_BACKEND_METAL
};

} // namespace mbgl
