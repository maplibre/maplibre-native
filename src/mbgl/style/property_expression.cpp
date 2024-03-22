#include <mbgl/style/property_expression.hpp>

#include <mbgl/renderer/paint_property_binder.hpp>
#include <mbgl/util/convert.hpp>

namespace mbgl {
namespace style {

namespace {

using namespace expression;

bool checkGPUCapable(const Expression& expression, const ZoomCurvePtr& zoomCurve) {
    return (expression.dependencies == Dependency::Zoom) && !zoomCurve.is<std::nullptr_t>() &&
           (expression.getType().is<type::NumberType>() || expression.getType().is<type::ColorType>());
}

inline GPUOutputType getOutputType(const Expression& expression) {
    if (expression.getType().is<type::NumberType>()) {
        return GPUOutputType::Float;
    } else {
        return GPUOutputType::Color;
    }
}

inline GPUInterpType getInterpType(const Interpolator& interp) {
    return interp.match(
        [&](const ExponentialInterpolator& exp) {
            return (exp.base == 1.0f) ? GPUInterpType::Linear : GPUInterpType::Exponential;
        },
        [&](const CubicBezierInterpolator&) { return GPUInterpType::Bezier; });
}

inline float getInterpBase(const Interpolator& interp) {
    return interp.match([&](const ExponentialInterpolator& exp) { return static_cast<float>(exp.base); },
                        [&](const CubicBezierInterpolator&) { return 0.0f; });
}

// Produce a function for `eachStop` that extracts a single stop into the target expression
auto addStop(MutableUniqueGPUExpression& expr, GPUOutputType outType, std::size_t& index) {
    return [&, outType](double input, const Expression& output) {
        if (expr) {
            if (const auto result = output.evaluate(EvaluationContext{/*zoom=*/0.0f})) {
                const expression::Value& value = *result;
                expr->inputs[index] = static_cast<float>(input);
                if (outType == GPUOutputType::Float) {
                    assert(value.is<double>());
                    if (value.is<double>()) {
                        expr->stops.floats[index++] = static_cast<float>(value.get<double>());
                    } else {
                        // Evaluation error, cancel
                        expr.reset();
                    }
                } else if (outType == GPUOutputType::Color) {
                    assert(value.is<Color>());
                    if (value.is<Color>()) {
                        const auto& color = attributeValue(value.get<Color>());
                        std::copy(color.begin(), color.end(), &expr->stops.colors[2 * index++]);
                    } else {
                        // Evaluation error, cancel
                        expr.reset();
                    }
                }
            }
        }
    };
};

} // namespace

const GPUExpression GPUExpression::empty = {GPUOutputType::Float, 0};

MutableUniqueGPUExpression GPUExpression::create(GPUOutputType type, std::uint16_t count) {
    if (1 < count && count <= maxStops) {
        return MutableUniqueGPUExpression{new GPUExpression(type, count)};
    }
    return {};
}

float GPUExpression::evaluateFloat(const float zoom) const {
    const auto index = std::distance(&inputs[0], std::upper_bound(&inputs[0], &inputs[stopCount], zoom));
    if (index == 0) {
        return stops.floats[0];
    } else if (index == stopCount) {
        return stops.floats[stopCount - 1];
    }
    switch (interpolation) {
        case GPUInterpType::Step:
            return stops.floats[index - 1];
        default:
            assert(false);
            [[fallthrough]];
        case GPUInterpType::Linear:
            assert(interpOptions.exponential.base == 1.0f);
            [[fallthrough]];
        case GPUInterpType::Exponential: {
            const Range<double> range{inputs[index - 1], inputs[index]};
            const expression::ExponentialInterpolator interpolator{interpOptions.exponential.base};
            const auto t = interpolator.interpolationFactor(range, zoom);
            return util::Interpolator<float>()(stops.floats[index - 1], stops.floats[index], t);
        }
        case GPUInterpType::Bezier:
            assert(false);
            return stops.floats[0];
    }
}

namespace {
std::tuple<float, float> unpack_float(const float packedValue) {
    const int packedIntValue = int(packedValue);
    const int v0 = packedIntValue / 256;
    return {static_cast<float>(v0) / 255, static_cast<float>(packedIntValue - v0 * 256) / 255};
}
std::tuple<float, float, float, float> decode_color(const float encoded[2]) {
    return std::tuple_cat(unpack_float(encoded[0]), unpack_float(encoded[1]));
}
Color toColor(std::tuple<float, float, float, float> rgba) {
    return {std::get<0>(rgba), std::get<1>(rgba), std::get<2>(rgba), std::get<3>(rgba)};
}
} // namespace

Color GPUExpression::getColor(std::size_t index) const {
    return toColor(decode_color(&stops.colors[2 * index]));
}

Color GPUExpression::evaluateColor(const float zoom) const {
    const auto index = std::distance(&inputs[0], std::upper_bound(&inputs[0], &inputs[stopCount], zoom));
    if (index == 0) {
        return getColor(0);
    } else if (index == stopCount) {
        return getColor(stopCount - 1);
    }
    switch (interpolation) {
        case GPUInterpType::Step:
            return getColor(index - 1);
        default:
            assert(false);
            [[fallthrough]];
        case GPUInterpType::Linear:
            assert(interpOptions.exponential.base == 1.0f);
            [[fallthrough]];
        case GPUInterpType::Exponential: {
            const Range<double> range{inputs[index - 1], inputs[index]};
            const expression::ExponentialInterpolator interpolator{interpOptions.exponential.base};
            const auto t = interpolator.interpolationFactor(range, zoom);
            return util::Interpolator<Color>()(getColor(index - 1), getColor(index), t);
            break;
        }
        case GPUInterpType::Bezier:
            assert(false);
            return getColor(0);
    }
}

PropertyExpressionBase::PropertyExpressionBase(std::unique_ptr<expression::Expression> expression_) noexcept
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

UniqueGPUExpression PropertyExpressionBase::getGPUExpression(bool transitioning, bool intZoom) const {
    if (!isGPUCapable_) {
        return {};
    }
    std::size_t index = 0;
    const auto outType = getOutputType(*expression);
    const auto options = ((useIntegerZoom_ || intZoom) ? GPUOptions::IntegerZoom : GPUOptions::None) |
                         (transitioning ? GPUOptions::Transitioning : GPUOptions::None);
    return zoomCurve.match(
        [&](const Step* step) {
            auto expr = GPUExpression::create(outType, step->getStopCount());
            expr->options = options;
            expr->interpolation = GPUInterpType::Step;
            step->eachStop(addStop(expr, outType, index));
            return expr;
        },
        [&](const Interpolate* interp) {
            auto expr = GPUExpression::create(outType, interp->getStopCount());
            expr->options = options;
            expr->interpolation = getInterpType(interp->getInterpolator());
            expr->interpOptions.exponential.base = getInterpBase(interp->getInterpolator());
            interp->eachStop(addStop(expr, outType, index));
            return expr;
        },
        [](std::nullptr_t) { return UniqueGPUExpression{}; });
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
