#pragma once

#include <mln/style/expression/expression.hpp>
#include <mln/util/range.hpp>
#include <memory>
#include <map>

namespace mln {
namespace style {
namespace expression {

/// Return the smallest range of stops that covers the interval [lower, upper]
Range<float> getCoveringStops(const std::map<double, std::unique_ptr<Expression>>& stops,
                              double lower,
                              double upper) noexcept;

} // namespace expression
} // namespace style
} // namespace mln
