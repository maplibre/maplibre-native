#pragma once

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
    /*  0 */ float zoom;
    /*  4 */ float time;
    /*  8 */ uint64_t frame;
    /* 16 */
};
static_assert(sizeof(ExpressionInputsUBO) == 1 * 16);

} // namespace shaders
} // namespace mbgl
