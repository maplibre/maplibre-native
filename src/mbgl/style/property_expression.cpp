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
    return interp.match([&](const ExponentialInterpolator& interp) { return GPUInterpType::Exponential; },
                        [&](const CubicBezierInterpolator&) { return GPUInterpType::Bezier; });
}

inline float getInterpBase(const Interpolator& interp) {
    return interp.match([&](const ExponentialInterpolator& interp) { return static_cast<float>(interp.base); },
                        [&](const CubicBezierInterpolator&) { return 0.0f; });
}

// Produce a function for `eachStop` that extracts a single stop into the target expression
static auto addStop(MutableUniqueGPUExpression& expr, GPUOutputType outType, std::size_t& index) {
    return [&, outType](double input, const Expression& output) {
        if (expr) {
            if (const auto result = output.evaluate(EvaluationContext{/*zoom=*/0.0f})) {
                const expression::Value& value = *result;
                if (outType == GPUOutputType::Float) {
                    assert(value.is<double>());
                    if (value.is<double>()) {
                        auto& stop = expr->stops.floatStops[index];
                        stop.input = static_cast<float>(input);
                        stop.output = static_cast<float>(value.get<double>());
                    } else {
                        // Evaluation error, cancel
                        expr.reset();
                    }
                } else if (outType == GPUOutputType::Color) {
                    assert(value.is<Color>());
                    if (value.is<Color>()) {
                        auto& stop = expr->stops.colorStops[index];
                        stop.input = static_cast<float>(input);

                        const auto color = util::cast<float>(value.get<Color>().toArray());
                        std::copy(color.begin(), color.end(), stop.output);
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

void GPUExpressionDeleter::operator()(const GPUExpression* expr) {
    delete[] reinterpret_cast<uint8_t*>(const_cast<GPUExpression*>(expr));
}

MutableUniqueGPUExpression GPUExpression::create(GPUOutputType type, std::uint16_t count) {
    if (count < 1) {
        return {};
    }
    const auto size = sizeFor(type, count);
    // allocate
    if (auto* storage = new (std::align_val_t(alignment), std::nothrow) uint8_t[size]) {
        // construct
        new (storage) GPUExpression(type, count);
        // wrap in `unique_ptr`
        return MutableUniqueGPUExpression{reinterpret_cast<GPUExpression*>(storage)};
    }
    return {};
}

std::size_t GPUExpression::stopSize(GPUOutputType type) {
    switch (type) {
        case GPUOutputType::Float:
            return sizeof(FloatStop);
        default:
            assert(false);
            [[fallthrough]];
        case GPUOutputType::Color:
            return sizeof(ColorStop);
    }
}
std::size_t GPUExpression::sizeFor(GPUOutputType type, std::uint16_t count) {
    return sizeof(GPUExpression) + (count - 1) * stopSize(type);
}

PropertyExpressionBase::PropertyExpressionBase(std::unique_ptr<expression::Expression> expression_) noexcept
    : expression(std::move(expression_)),
      zoomCurve(expression->all(Dependency::Zoom) ? expression::findZoomCurveChecked(*expression) : nullptr),
      useIntegerZoom_(false),
      isZoomConstant_(expression->none(Dependency::Zoom)),
      isFeatureConstant_(expression->none(Dependency::Feature)),
      isRuntimeConstant_(expression->none(Dependency::Image)),
      isGPUCapable_(checkGPUCapable(*expression, zoomCurve)) {
    assert(isZoomConstant_ == expression::isZoomConstant(*expression));
    assert(isFeatureConstant_ == expression::isFeatureConstant(*expression));
    assert(isRuntimeConstant_ == expression::isRuntimeConstant(*expression));
}

UniqueGPUExpression PropertyExpressionBase::getGPUExpression() const {
    if (!isGPUCapable_) {
        return {};
    }
    std::size_t index = 0;
    const auto outType = getOutputType(*expression);
    return zoomCurve.match(
        [&](const Step* step) {
            auto expr = GPUExpression::create(outType, step->getStopCount());
            expr->interpolation = GPUInterpType::Linear;
            step->eachStop(addStop(expr, outType, index));
            return expr;
        },
        [&](const Interpolate* interp) {
            auto expr = GPUExpression::create(outType, interp->getStopCount());
            expr->interpolation = getInterpType(interp->getInterpolator());
            expr->expBase = getInterpBase(interp->getInterpolator());
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
