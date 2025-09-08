#pragma once

#include <mbgl/style/expression/expression.hpp>
#include <mbgl/style/expression/is_constant.hpp>
#include <mbgl/style/expression/interpolate.hpp>
#include <mbgl/style/expression/step.hpp>
#include <mbgl/style/expression/find_zoom_curve.hpp>
#include <mbgl/util/bitmask_operations.hpp>
#include <mbgl/util/range.hpp>
#include <mbgl/gfx/gpu_expression.hpp>

#include <optional>

namespace mbgl {
namespace gfx {
class GPUExpression;
using UniqueGPUExpression = std::unique_ptr<GPUExpression>;
} // namespace gfx

namespace style {

class PropertyExpressionBase {
public:
    using Expression = expression::Expression;
    using Dependency = expression::Dependency;
    using ZoomCurvePtr = expression::ZoomCurvePtr;

    PropertyExpressionBase(PropertyExpressionBase&&);
    PropertyExpressionBase(const PropertyExpressionBase&);
    explicit PropertyExpressionBase(std::unique_ptr<Expression>);
    virtual ~PropertyExpressionBase() = default;

    PropertyExpressionBase& operator=(PropertyExpressionBase&&);
    PropertyExpressionBase& operator=(const PropertyExpressionBase&);

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

    /// Build a cached GPU representation of the expression, with the same lifetime as this object.
    gfx::UniqueGPUExpression getGPUExpression(bool intZoom) const;

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
            if (typed) {
                return *typed;
            }
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
