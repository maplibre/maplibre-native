#pragma once

#include <mbgl/util/interpolate.hpp>
#include <mbgl/util/range.hpp>
#include <mbgl/util/unitbezier.hpp>

namespace mbgl {
namespace style {
namespace expression {

class ExponentialInterpolator {
public:
    ExponentialInterpolator(double base_) noexcept
        : base(base_) {}

    double base;

    double interpolationFactor(const Range<double>& inputLevels, const double input) const noexcept {
        return util::interpolationFactor(
            static_cast<float>(base),
            Range<float>{static_cast<float>(inputLevels.min), static_cast<float>(inputLevels.max)},
            static_cast<float>(input));
    }

    bool operator==(const ExponentialInterpolator& rhs) const noexcept { return base == rhs.base; }
};

class CubicBezierInterpolator {
public:
    CubicBezierInterpolator(double x1_, double y1_, double x2_, double y2_) noexcept
        : ub(x1_, y1_, x2_, y2_) {}

    double interpolationFactor(const Range<double>& inputLevels, const double input) const noexcept {
        return ub.solve(util::interpolationFactor(
                            1.0f,
                            Range<float>{static_cast<float>(inputLevels.min), static_cast<float>(inputLevels.max)},
                            static_cast<float>(input)),
                        1e-6);
    }

    bool operator==(const CubicBezierInterpolator& rhs) const noexcept { return ub == rhs.ub; }

    util::UnitBezier ub;
};

using Interpolator = variant<ExponentialInterpolator, CubicBezierInterpolator>;

} // namespace expression
} // namespace style
} // namespace mbgl
