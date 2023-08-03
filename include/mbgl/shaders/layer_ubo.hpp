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
    /*  0 */ uint64_t time;  /// Current scene time (nanoseconds)
    /*  8 */ uint64_t frame; /// Current frame count
    /* 16 */ float zoom;     /// Current zoom level
    /* 20 */ float pad1, pad2, pad3;
    /* 32 */
};
static_assert(sizeof(ExpressionInputsUBO) == 2 * 16);

} // namespace shaders
} // namespace mbgl
