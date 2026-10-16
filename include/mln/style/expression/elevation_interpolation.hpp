#pragma once

#include <mln/style/expression/compound_expression.hpp>
#include <mln/style/expression/interpolate.hpp>
#include <mln/util/range.hpp>

#include <optional>

namespace mln {
namespace style {
namespace expression {

/// Experimental, fill-extrusion-color: when an expression's outermost operator is an interpolation over
/// `["elevation"]` (height above the ground in meters), the heights of its first and last stops.
///
/// Expressions of that shape that also read feature data are evaluated per feature at both heights and
/// blended linearly by height on the GPU, the same way zoom-and-feature expressions are blended by zoom.
/// Stops in between and non-linear curves are not honoured, see `isExactElevationInterpolation`.
inline std::optional<Range<float>> elevationInterpolationStops(const Expression& expression) {
    if (expression.getKind() != Kind::Interpolate) {
        return std::nullopt;
    }
    const auto& interpolate = static_cast<const Interpolate&>(expression);
    const auto& input = *interpolate.getInput();
    if (input.getKind() != Kind::CompoundExpression ||
        static_cast<const CompoundExpression&>(input).getOperator() != "elevation" || interpolate.getStopCount() < 2) {
        return std::nullopt;
    }
    std::optional<double> first;
    double last = 0;
    interpolate.eachStop([&](double height, const Expression&) {
        if (!first) {
            first = height;
        }
        last = height;
    });
    return Range<float>{static_cast<float>(*first), static_cast<float>(last)};
}

/// Whether blending the colors at the first and last stop linearly reproduces the expression exactly:
/// a linear interpolation with exactly two stops.
inline bool isExactElevationInterpolation(const Expression& expression) {
    if (!elevationInterpolationStops(expression)) {
        return false;
    }
    const auto& interpolate = static_cast<const Interpolate&>(expression);
    const bool linear = interpolate.getInterpolator().match(
        [](const ExponentialInterpolator& exponential) { return exponential.base == 1.0; },
        [](const CubicBezierInterpolator&) { return false; });
    return linear && interpolate.getStopCount() == 2;
}

} // namespace expression
} // namespace style
} // namespace mln
