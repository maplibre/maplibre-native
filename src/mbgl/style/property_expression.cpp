#include <mbgl/style/property_expression.hpp>

namespace mbgl {
namespace style {

PropertyExpressionBase::PropertyExpressionBase(std::unique_ptr<expression::Expression> expression_) noexcept
    : expression(std::move(expression_)),
      isZoomConstant_(expression::isZoomConstant(*expression)),
      isFeatureConstant_(expression::isFeatureConstant(*expression)),
      isRuntimeConstant_(expression::isRuntimeConstant(*expression)),
      zoomCurve(isZoomConstant_ ? nullptr : expression::findZoomCurveChecked(*expression)) {}
float PropertyExpressionBase::interpolationFactor(const Range<float>& inputLevels,
                                                  const float inputValue) const noexcept {
    return zoomCurve.match(
        [](std::nullptr_t) {
            assert(false);
            return 0.0f;
        },
        [&](const expression::Interpolate* z) {
            return z->interpolationFactor(Range<double>{inputLevels.min, inputLevels.max}, inputValue);
        },
        [](const expression::Step*) { return 0.0f; });
}

Range<float> PropertyExpressionBase::getCoveringStops(const float lower, const float upper) const noexcept {
    return zoomCurve.match(
        [](std::nullptr_t) -> Range<float> {
            assert(false);
            return {0.0f, 0.0f};
        },
        [&](auto z) { return z->getCoveringStops(lower, upper); });
}

const expression::Expression& PropertyExpressionBase::getExpression() const noexcept {
    return *expression;
}

std::shared_ptr<const expression::Expression> PropertyExpressionBase::getSharedExpression() const noexcept {
    return expression;
}

} // namespace style
} // namespace mbgl
