#include <mbgl/style/property_expression.hpp>

namespace mbgl {
namespace style {

using namespace expression::type;

PropertyExpressionBase::PropertyExpressionBase(std::unique_ptr<expression::Expression> expression_) noexcept
    : expression(std::move(expression_)),
      zoomCurve(expression->all(Dependency::Zoom) ? expression::findZoomCurveChecked(*expression) : nullptr),
      useIntegerZoom_(false),
      isZoomConstant_(expression->none(Dependency::Zoom)),
      isFeatureConstant_(expression->none(Dependency::Feature)),
      isRuntimeConstant_(expression->none(Dependency::Image)),
      isGPUCapable_((expression->dependencies == Dependency::Zoom) && !zoomCurve.is<std::nullptr_t>() &&
                    (expression->getType().is<NumberType>() || expression->getType().is<ColorType>())) {
    assert(isZoomConstant_ == expression::isZoomConstant(*expression));
    assert(isFeatureConstant_ == expression::isFeatureConstant(*expression));
    assert(isRuntimeConstant_ == expression::isRuntimeConstant(*expression));
}
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
