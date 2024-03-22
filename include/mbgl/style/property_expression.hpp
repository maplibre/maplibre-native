#pragma once

#include <mbgl/style/expression/expression.hpp>
#include <mbgl/style/expression/is_constant.hpp>
#include <mbgl/style/expression/interpolate.hpp>
#include <mbgl/style/expression/step.hpp>
#include <mbgl/style/expression/find_zoom_curve.hpp>
#include <mbgl/util/bitmask_operations.hpp>
#include <mbgl/util/range.hpp>

#include <optional>

namespace mbgl {
namespace style {

enum class GPUInterpType : std::uint16_t {
    Step,
    Linear,
    Exponential,
    Bezier
};
enum class GPUOutputType : std::uint16_t {
    Float,
    Color,
};
enum class GPUOptions : std::uint16_t {
    None = 0,
    IntegerZoom = 1 << 0,
    Transitioning = 1 << 1,
};

struct GPUExpression;
using UniqueGPUExpression = std::unique_ptr<const GPUExpression>;
using MutableUniqueGPUExpression = std::unique_ptr<GPUExpression>;

struct alignas(16) GPUExpression {
    static constexpr std::size_t maxStops = 16;

    GPUExpression(GPUExpression&&) = default;
    GPUExpression(const GPUExpression&) = default;
    GPUExpression(const GPUExpression* other)
        : GPUExpression(other ? *other : empty) {}
    GPUExpression& operator=(GPUExpression&&) = default;
    GPUExpression& operator=(const GPUExpression&) = default;

    /* 0 */ const GPUOutputType outputType;
    /* 2 */ const std::uint16_t stopCount;
    /* 4 */ GPUOptions options;
    /* 6 */ GPUInterpType interpolation;

    /* 8 */ union InterpOptions {
        struct Exponential {
            float base;
        } exponential;

        struct Bezier {
            float x1;
            float y1;
            float x2;
            float y2;
        } bezier;
    } interpOptions;

    /* 24 */ float inputs[maxStops];

    /* 24 + (4 * maxStops) = 88 */ union Stops {
        float floats[maxStops];
        float colors[2 * maxStops];
    } stops;

    static MutableUniqueGPUExpression create(GPUOutputType, std::uint16_t stopCount);

    float evaluateFloat(const float zoom) const;
    Color evaluateColor(const float zoom) const;

    template <typename T>
    auto evaluate(const float zoom) const;

    Color getColor(std::size_t index) const;

    static const GPUExpression empty;

private:
    GPUExpression() = default;
    GPUExpression(GPUOutputType type, uint16_t count)
        : outputType(type),
          stopCount(count) {}
};
static_assert(sizeof(GPUExpression) == 32 + (4 + 8) * GPUExpression::maxStops);
static_assert(sizeof(GPUExpression) % 16 == 0);

template <>
inline auto GPUExpression::evaluate<Color>(const float zoom) const {
    return evaluateColor(zoom);
}
template <>
inline auto GPUExpression::evaluate<float>(const float zoom) const {
    return evaluateFloat(zoom);
}

class PropertyExpressionBase {
public:
    using Expression = expression::Expression;
    using Dependency = expression::Dependency;
    using ZoomCurvePtr = expression::ZoomCurvePtr;

    explicit PropertyExpressionBase(std::unique_ptr<Expression>);
    virtual ~PropertyExpressionBase() = default;

    bool isZoomConstant() const noexcept { return isZoomConstant_; }
    bool isFeatureConstant() const noexcept { return isFeatureConstant_; }
    bool isRuntimeConstant() const noexcept { return isRuntimeConstant_; }
    float interpolationFactor(const Range<float>&, float) const noexcept;
    Range<float> getCoveringStops(float, float) const noexcept;
    const Expression& getExpression() const noexcept;

    bool isGPUCapable() const { return isGPUCapable_; }

    bool getUseIntegerZoom() const { return useIntegerZoom_; }
    void setUseIntegerZoom(bool value) { useIntegerZoom_ = value; }

    /// Can be used for aggregating property expressions from multiple properties(layers) into single match / case
    /// expression. May be removed if a better way of aggregation is found.
    std::shared_ptr<const Expression> getSharedExpression() const noexcept;

    UniqueGPUExpression getGPUExpression(bool transitioning, bool intZoom) const;

    Dependency getDependencies() const noexcept { return expression ? expression->dependencies : Dependency::None; }

    const ZoomCurvePtr& getZoomCurve() const { return zoomCurve; }

protected:
    std::shared_ptr<const Expression> expression;

    ZoomCurvePtr zoomCurve;

    bool useIntegerZoom_ = false;
    bool isZoomConstant_;
    bool isFeatureConstant_;
    bool isRuntimeConstant_;

    // If the expression depends on zoom and nothing else, and produces
    // a number or color, we can potentially evaluate it on the GPU
    bool isGPUCapable_;
};

template <class T>
class PropertyExpression final : public PropertyExpressionBase {
    static_assert(std::is_nothrow_move_constructible_v<T>);

public:
    // Second parameter to be used only for conversions from legacy functions.
    PropertyExpression(std::unique_ptr<Expression> expression_, std::optional<T> defaultValue_ = std::nullopt)
        : PropertyExpressionBase(std::move(expression_)),
          defaultValue(std::move(defaultValue_)) {}

    T evaluate(const expression::EvaluationContext& context, T finalDefaultValue = T()) const {
        const expression::EvaluationResult result = expression->evaluate(context);
        if (result) {
            const std::optional<T> typed = expression::fromExpressionValue<T>(*result);
            return typed ? *typed : defaultValue ? *defaultValue : finalDefaultValue;
        }
        return defaultValue ? *defaultValue : finalDefaultValue;
    }

    T evaluate(float zoom) const {
        assert(!isZoomConstant());
        assert(isFeatureConstant());
        return evaluate(expression::EvaluationContext(zoom));
    }

    T evaluate(const GeometryTileFeature& feature, T finalDefaultValue) const {
        assert(isZoomConstant());
        assert(!isFeatureConstant());
        return evaluate(expression::EvaluationContext(&feature), finalDefaultValue);
    }

    T evaluate(const GeometryTileFeature& feature,
               const std::set<std::string>& availableImages,
               T finalDefaultValue) const {
        return evaluate(expression::EvaluationContext(&feature).withAvailableImages(&availableImages),
                        finalDefaultValue);
    }

    T evaluate(const GeometryTileFeature& feature, const CanonicalTileID& canonical, T finalDefaultValue) const {
        return evaluate(expression::EvaluationContext(&feature).withCanonicalTileID(&canonical), finalDefaultValue);
    }

    T evaluate(const GeometryTileFeature& feature,
               const std::set<std::string>& availableImages,
               const CanonicalTileID& canonical,
               T finalDefaultValue) const {
        return evaluate(expression::EvaluationContext(&feature)
                            .withAvailableImages(&availableImages)
                            .withCanonicalTileID(&canonical),
                        finalDefaultValue);
    }

    T evaluate(float zoom, const GeometryTileFeature& feature, T finalDefaultValue) const {
        return evaluate(expression::EvaluationContext(zoom, &feature), finalDefaultValue);
    }

    T evaluate(float zoom,
               const GeometryTileFeature& feature,
               const std::set<std::string>& availableImages,
               T finalDefaultValue) const {
        return evaluate(expression::EvaluationContext(zoom, &feature).withAvailableImages(&availableImages),
                        finalDefaultValue);
    }

    T evaluate(float zoom,
               const GeometryTileFeature& feature,
               const std::set<std::string>& availableImages,
               const CanonicalTileID& canonical,
               T finalDefaultValue) const {
        return evaluate(expression::EvaluationContext(zoom, &feature)
                            .withAvailableImages(&availableImages)
                            .withCanonicalTileID(&canonical),
                        finalDefaultValue);
    }

    T evaluate(float zoom,
               const GeometryTileFeature& feature,
               const CanonicalTileID& canonical,
               T finalDefaultValue) const {
        return evaluate(expression::EvaluationContext(zoom, &feature).withCanonicalTileID(&canonical),
                        finalDefaultValue);
    }

    T evaluate(float zoom, const GeometryTileFeature& feature, const FeatureState& state, T finalDefaultValue) const {
        assert(!isFeatureConstant());
        return evaluate(expression::EvaluationContext(zoom, &feature, &state), finalDefaultValue);
    }

    std::vector<std::optional<T>> possibleOutputs() const {
        return expression::fromExpressionValues<T>(expression->possibleOutputs());
    }

    using Expression = expression::Expression;

    friend bool operator==(const PropertyExpression& lhs, const PropertyExpression& rhs) {
        return *lhs.expression == *rhs.expression;
    }

private:
    std::optional<T> defaultValue;
};

} // namespace style
} // namespace mbgl
