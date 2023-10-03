#pragma once

#include <mbgl/util/color.hpp>

#include <array>
#include <cstdint>

namespace mbgl {
namespace shaders {

enum class AttributeSource : int32_t {
    Constant,
    PerVertex,
    Computed,
};

enum class ExpressionFunction : int32_t {
    Linear,
    Exponential,
};

struct Expression {
    /* 0 */ ExpressionFunction function;
    /* 4 */
};
static_assert(sizeof(Expression) == 4, "unexpected padding");

struct Attribute {
    /* 0 */ AttributeSource source;
    /* 4 */ Expression expression;
    /* 8 */
};
static_assert(sizeof(Attribute) == 8, "unexpected padding");

constexpr int ExprMaxStops = 16;
struct Expression2 {
    /*   0 */ float zooms[ExprMaxStops];
    /*  64 */ float values[ExprMaxStops];
    /* 128 */ ExpressionFunction function;
    /* 132 */ int32_t stopCount;
    /* 136 */
};
static_assert(sizeof(Expression2) == 136, "unexpected padding");

struct alignas(16) ColorExpression {
    /*   0 */ float zooms[ExprMaxStops];
    /*  64 */ float values[ExprMaxStops][4];
    /* 320 */ ExpressionFunction function;
    /* 324 */ uint32_t stopCount;
    /* 328 */ uint32_t useIntegerZoom;
    /* 332 */ uint32_t pad;
    /* 336 */
};
static_assert(sizeof(ColorExpression) == 21 * 16, "unexpected padding");

struct Attribute2 {
    /*   0 */ Expression2 expression;
    /* 136 */ AttributeSource source;
    /* 140 */
};
static_assert(sizeof(Attribute2) == 140, "unexpected padding");

struct alignas(16) ColorAttribute {
    /*   0 */ ColorExpression expression;
    /* 336 */ AttributeSource source;
    /* 340 */ uint32_t pad1, pad2, pad3;
    /* 352 */
};
static_assert(sizeof(ColorAttribute) == 22 * 16, "unexpected padding");

struct alignas(16) ExpressionInputsUBO {
    // These can use uint64_t in later versions of Metal
    /*  0 */ uint32_t time_lo; /// Current scene time (nanoseconds)
    /*  4 */ uint32_t time_hi;
    /*  8 */ uint32_t frame_lo; /// Current frame count
    /* 12 */ uint32_t frame_hi;
    /* 16 */ float zoom;      /// Current zoom level
    /* 20 */ float zoom_frac; /// double precision zoom
    /* 24 */ float pad1, pad2;
    /* 32 */
};
static_assert(sizeof(ExpressionInputsUBO) == 2 * 16);

} // namespace shaders
} // namespace mbgl
