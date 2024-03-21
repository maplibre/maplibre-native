#pragma once

#include <mbgl/renderer/layer_tweaker.hpp>
#include <mbgl/style/layers/line_layer_properties.hpp>

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

#if MLN_RENDER_BACKEND_METAL
    using LinePaintProperties = style::LinePaintProperties;
    using Unevaluated = LinePaintProperties::Unevaluated;
    void setGPUExpressions(Unevaluated::GPUExpressions&&);

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
    gfx::UniformBufferPtr linePropertiesBuffer;
    gfx::UniformBufferPtr lineGradientPropertiesBuffer;
    gfx::UniformBufferPtr linePatternPropertiesBuffer;
    gfx::UniformBufferPtr lineSDFPropertiesBuffer;
    gfx::UniformBufferPtr dynamicBuffer;

#if MLN_RENDER_BACKEND_METAL
    gfx::UniformBufferPtr permutationUniformBuffer;
    gfx::UniformBufferPtr expressionUniformBuffer;

    Unevaluated::GPUExpressions gpuExpressions;
#endif // MLN_RENDER_BACKEND_METAL

    bool simplePropertiesUpdated = true;
    bool gradientPropertiesUpdated = true;
    bool patternPropertiesUpdated = true;
    bool sdfPropertiesUpdated = true;

    bool gpuExpressionsUpdated = true;
};

} // namespace mbgl
