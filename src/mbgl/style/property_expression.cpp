#include <mbgl/style/property_expression.hpp>

#include <mbgl/renderer/paint_property_binder.hpp>
#include <mbgl/util/convert.hpp>

#include <mbgl/gfx/gpu_expression.hpp>

namespace mbgl {
namespace style {

namespace {

using namespace expression;
bool checkGPUCapable(const Expression& expression, const ZoomCurvePtr& zoomCurve) {
    return (expression.dependencies == Dependency::Zoom) && !zoomCurve.is<std::nullptr_t>() &&
           (expression.getType().is<type::NumberType>() || expression.getType().is<type::ColorType>());
}
} // namespace

PropertyExpressionBase::PropertyExpressionBase(std::unique_ptr<expression::Expression> expression_)
    : expression(std::move(expression_)),
      zoomCurve(expression->has(Dependency::Zoom) ? expression::findZoomCurveChecked(*expression) : nullptr),
      useIntegerZoom_(false),
      isZoomConstant_(!expression->has(Dependency::Zoom)),
      isFeatureConstant_(!expression->has(Dependency::Feature)),
      isRuntimeConstant_(!expression->has(Dependency::Image)),
      isGPUCapable_(checkGPUCapable(*expression, zoomCurve)) {
    assert(isZoomConstant_ == expression::isZoomConstant(*expression));
    assert(isFeatureConstant_ == expression::isFeatureConstant(*expression));
    assert(isRuntimeConstant_ == expression::isRuntimeConstant(*expression));
}

PropertyExpressionBase::PropertyExpressionBase(PropertyExpressionBase&& other)
    : expression(std::move(other.expression)),
      zoomCurve(std::move(other.zoomCurve)),
      useIntegerZoom_(other.useIntegerZoom_),
      isZoomConstant_(other.isZoomConstant_),
      isFeatureConstant_(other.isFeatureConstant_),
      isRuntimeConstant_(other.isRuntimeConstant_),
      isGPUCapable_(other.isGPUCapable_) {}

PropertyExpressionBase::PropertyExpressionBase(const PropertyExpressionBase& other)
    : expression(other.expression),
      zoomCurve(other.zoomCurve),
      useIntegerZoom_(other.useIntegerZoom_),
      isZoomConstant_(other.isZoomConstant_),
      isFeatureConstant_(other.isFeatureConstant_),
      isRuntimeConstant_(other.isRuntimeConstant_),
      isGPUCapable_(other.isGPUCapable_) {}

PropertyExpressionBase& PropertyExpressionBase::operator=(PropertyExpressionBase&& other) {
    expression = std::move(other.expression);
    zoomCurve = other.zoomCurve;
    useIntegerZoom_ = other.useIntegerZoom_;
    isZoomConstant_ = other.isZoomConstant_;
    isFeatureConstant_ = other.isFeatureConstant_;
    isRuntimeConstant_ = other.isRuntimeConstant_;
    isGPUCapable_ = other.isGPUCapable_;
    return *this;
}

PropertyExpressionBase& PropertyExpressionBase::operator=(const PropertyExpressionBase& other) {
    expression = other.expression;
    zoomCurve = other.zoomCurve;
    useIntegerZoom_ = other.useIntegerZoom_;
    isZoomConstant_ = other.isZoomConstant_;
    isFeatureConstant_ = other.isFeatureConstant_;
    isRuntimeConstant_ = other.isRuntimeConstant_;
    isGPUCapable_ = other.isGPUCapable_;
    return *this;
}

gfx::UniqueGPUExpression PropertyExpressionBase::getGPUExpression(bool intZoom) const {
    return isGPUCapable_ ? gfx::GPUExpression::create(*expression, zoomCurve, useIntegerZoom_ || intZoom)
                         : gfx::UniqueGPUExpression{};
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
