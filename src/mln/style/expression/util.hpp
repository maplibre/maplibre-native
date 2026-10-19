#pragma once

#include <mln/style/expression/expression.hpp>
#include <mln/util/color.hpp>

namespace mln {
namespace style {
namespace expression {

Result<Color> rgba(double r, double g, double b, double a);

} // namespace expression
} // namespace style
} // namespace mln
