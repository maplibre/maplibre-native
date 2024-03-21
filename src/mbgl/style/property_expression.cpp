#include <mbgl/style/property_expression.hpp>
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
                if (outType == GPUOutputType::Float) {
                    assert(value.is<double>());
                    if (value.is<double>()) {
                        auto& stop = expr->stops.floatStops[index++];
                        stop.input = static_cast<float>(input);
                        stop.output = static_cast<float>(value.get<double>());
                    } else {
                        // Evaluation error, cancel
                        expr.reset();
                    }
                } else if (outType == GPUOutputType::Color) {
                    assert(value.is<Color>());
                    if (value.is<Color>()) {
                        auto& stop = expr->stops.colorStops[index++];
                        stop.input = static_cast<float>(input);

                        const auto& color = value.get<Color>();
                        stop.rgba[0] = color.r;
                        stop.rgba[1] = color.g;
                        stop.rgba[2] = color.b;
                        stop.rgba[3] = color.a;
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

MutableUniqueGPUExpression GPUExpression::create(GPUOutputType type, std::uint16_t count) {
    if (1 < count && count <= maxStops) {
        return MutableUniqueGPUExpression{new GPUExpression(type, count)};
    }
    return {};
}

float GPUExpression::evaluateFloat(const float zoom) const {
    const auto upperBound = std::upper_bound(&stops.floatStops[0], &stops.floatStops[stopCount], FloatStop{zoom, 0});
    const auto index = std::distance(&stops.floatStops[0], upperBound);
    if (index == 0) {
        return stops.floatStops[0].output;
    }
    if (index == stopCount) {
        return stops.floatStops[stopCount - 1].output;
    }

    const Range<double> range{stops.floatStops[index - 1].input, stops.floatStops[index].input};
    switch (interpolation) {
        case GPUInterpType::Step:
            return stops.floatStops[index - 1].output;
        default:
            assert(false);
            [[fallthrough]];
        case GPUInterpType::Linear:
            assert(interpOptions.exponential.base == 1.0f);
            [[fallthrough]];
        case GPUInterpType::Exponential: {
            const expression::ExponentialInterpolator interpolator{interpOptions.exponential.base};
            const auto t = interpolator.interpolationFactor(range, zoom);
            return util::Interpolator<float>()(stops.floatStops[index - 1].output, stops.floatStops[index].output, t);
        }
        case GPUInterpType::Bezier:
            assert(false);
            return stops.floatStops[0].output;
    }
}

Color GPUExpression::evaluateColor(const float zoom) const {
    const auto upperBound = std::upper_bound(&stops.colorStops[0], &stops.colorStops[stopCount], ColorStop{zoom, {0}});
    const auto index = std::distance(&stops.colorStops[0], upperBound);
    if (index == 0) {
        return {stops.colorStops[0].rgba};
    }
    if (index == stopCount) {
        return {stops.colorStops[stopCount - 1].rgba};
    }

    const Range<double> range{stops.colorStops[index - 1].input, stops.colorStops[index].input};
    switch (interpolation) {
        case GPUInterpType::Step:
            return stops.colorStops[index - 1].rgba;
        default:
            assert(false);
            [[fallthrough]];
        case GPUInterpType::Linear:
            assert(interpOptions.exponential.base == 1.0f);
            [[fallthrough]];
        case GPUInterpType::Exponential: {
            const expression::ExponentialInterpolator interpolator{interpOptions.exponential.base};
            const auto t = interpolator.interpolationFactor(range, zoom);
            return util::Interpolator<Color>()({stops.colorStops[index - 1].rgba}, {stops.colorStops[index].rgba}, t);
            break;
        }
        case GPUInterpType::Bezier:
            assert(false);
            return {stops.colorStops[0].rgba};
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
