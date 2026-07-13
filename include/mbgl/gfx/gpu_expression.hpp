#pragma once

#include <mbgl/util/color.hpp>
#include <mbgl/util/variant.hpp>

#include <optional>

namespace mbgl {
namespace style {
namespace expression {
class Expression;
class Interpolate;
class Step;
} // namespace expression
using ZoomCurvePtr = variant<std::nullptr_t, const expression::Interpolate*, const expression::Step*>;
} // namespace style
namespace gfx {

enum class GPUInterpType : std::uint16_t {
    Step,
    Linear,
    Exponential,
    Bezier
};
enum class GPUOutputType : std::uint16_t {
    Float,
    Color,
};
enum class GPUOptions : std::uint16_t {
    None = 0,
    IntegerZoom = 1 << 0,
    Transitioning = 1 << 1,
};

class GPUExpression;
using UniqueGPUExpression = std::unique_ptr<GPUExpression>;

class alignas(16) GPUExpression {
public:
    static constexpr std::size_t maxStops = 16;

    GPUExpression() = delete;
    GPUExpression(GPUOutputType type, uint16_t count)
        : outputType(type),
          stopCount(count) {}
    GPUExpression(GPUExpression&&) = default;
    GPUExpression(const GPUExpression&) = default;
    GPUExpression(const GPUExpression* ptr)
        : GPUExpression(ptr ? *ptr : empty) {}
    GPUExpression& operator=(GPUExpression&&) = delete;
    GPUExpression& operator=(const GPUExpression&) = delete;

    /* 0 */ const GPUOutputType outputType;
    /* 2 */ const std::uint16_t stopCount;
    /* 4 */ GPUOptions options;
    /* 6 */ GPUInterpType interpolation;

    /* 8 */ union InterpOptions {
        struct Exponential {
            float base;
        } exponential;

        struct Bezier {
            float x1;
            float y1;
            float x2;
            float y2;
        } bezier;
    } interpOptions;

    /* 24 */ float inputs[maxStops];

    /* 24 + (4 * maxStops) = 88 */ union Stops {
        float floats[maxStops];
        float colors[2 * maxStops];
    } stops;

    static UniqueGPUExpression create(GPUOutputType, std::uint16_t stopCount);
    static UniqueGPUExpression create(const style::expression::Expression&, const style::ZoomCurvePtr&, bool intZoom);

    float evaluateFloat(const float zoom) const;
    Color evaluateColor(const float zoom) const;

    template <typename T>
    auto evaluate(const float zoom) const;

    Color getColor(std::size_t index) const;

    static const GPUExpression empty;
};
static_assert(sizeof(GPUExpression) == 32 + (4 + 8) * GPUExpression::maxStops);
static_assert(sizeof(GPUExpression) % 16 == 0);

template <>
inline auto GPUExpression::evaluate<Color>(const float zoom) const {
    return evaluateColor(zoom);
}
template <>
inline auto GPUExpression::evaluate<float>(const float zoom) const {
    return evaluateFloat(zoom);
}

} // namespace gfx
} // namespace mbgl
