#pragma once

#include <mln/style/expression/expression.hpp>
#include <mln/util/color.hpp>

namespace mln {
namespace style {
namespace expression {
namespace dsl {

std::unique_ptr<Expression> compound(const char* op, std::vector<std::unique_ptr<Expression>> args);

template <class... Args>
std::unique_ptr<Expression> compound(const char* op, Args&&... args) {
    return compound(op, vec(std::forward<Args>(args)...));
}

} // namespace dsl
} // namespace expression
} // namespace style
} // namespace mln
