#pragma once

#include <mbgl/util/color.hpp>

#include <array>
#include <cstdint>

#define MLN_STRINGIZE(X) #X

namespace mbgl {
namespace shaders {

enum class AttributeSource : int32_t {
    Constant,
    PerVertex,
    Computed,
};

enum class ExpressionFunction : int32_t {
    Constant,
    Linear,
    Exponential,
};

struct Expression {
    /* 0 */ ExpressionFunction function;
    /* 4 */
};
static_assert(sizeof(Expression) == 4);

struct Attribute {
    /* 0 */ AttributeSource source;
    /* 4 */ Expression expression;
    /* 8 */
};
static_assert(sizeof(Attribute) == 8);

struct alignas(16) ExpressionInputsUBO {
    // These can use uint64_t in later versions of Metal
    /*  0 */ uint32_t time_lo;  /// Current scene time (nanoseconds)
    /*  4 */ uint32_t time_hi;
    /*  8 */ uint32_t frame_lo; /// Current frame count
    /* 12 */ uint32_t frame_hi;
    /* 16 */ float zoom;     /// Current zoom level
    /* 20 */ float zoom_frac; /// double precision zoom
    /* 24 */ float pad1, pad2;
    /* 32 */
};
static_assert(sizeof(ExpressionInputsUBO) == 2 * 16);

} // namespace shaders
} // namespace mbgl
