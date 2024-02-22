#pragma once

#include <mbgl/style/expression/parsing_context.hpp>
#include <mbgl/style/expression/type.hpp>
#include <mbgl/style/expression/value.hpp>
#include <mbgl/tile/tile_id.hpp>
#include <mbgl/util/color.hpp>
#include <mbgl/util/variant.hpp>

#include <array>
#include <vector>
#include <memory>
#include <optional>

namespace mbgl {

class GeometryTileFeature;

namespace style {
namespace expression {

class EvaluationError {
public:
    std::string message;
};

class EvaluationContext {
public:
    EvaluationContext() = default;
    explicit EvaluationContext(float zoom_) noexcept
        : zoom(zoom_) {}
    explicit EvaluationContext(GeometryTileFeature const* feature_) noexcept
        : feature(feature_) {}
    EvaluationContext(float zoom_, GeometryTileFeature const* feature_) noexcept
        : zoom(zoom_),
          feature(feature_) {}
    EvaluationContext(std::optional<mbgl::Value> accumulated_, GeometryTileFeature const* feature_) noexcept
        : accumulated(std::move(accumulated_)),
          feature(feature_) {}
    EvaluationContext(float zoom_, GeometryTileFeature const* feature_, const FeatureState* state_) noexcept
        : zoom(zoom_),
          feature(feature_),
          featureState(state_) {}
    EvaluationContext(std::optional<float> zoom_,
                      GeometryTileFeature const* feature_,
                      std::optional<double> colorRampParameter_) noexcept
        : zoom(std::move(zoom_)),
          feature(feature_),
          colorRampParameter(std::move(colorRampParameter_)) {}

    EvaluationContext& withFormattedSection(const Value* formattedSection_) noexcept {
        formattedSection = formattedSection_;
        return *this;
    };

    EvaluationContext& withFeatureState(const FeatureState* featureState_) noexcept {
        featureState = featureState_;
        return *this;
    };

    EvaluationContext& withAvailableImages(const std::set<std::string>* availableImages_) noexcept {
        availableImages = availableImages_;
        return *this;
    };

    EvaluationContext& withCanonicalTileID(const mbgl::CanonicalTileID* canonical_) noexcept {
        canonical = canonical_;
        return *this;
    };

    std::optional<float> zoom;
    std::optional<mbgl::Value> accumulated;
    GeometryTileFeature const* feature = nullptr;
    std::optional<double> colorRampParameter;
    // Contains formatted section object, std::unordered_map<std::string, Value>.
    const Value* formattedSection = nullptr;
    const FeatureState* featureState = nullptr;
    const std::set<std::string>* availableImages = nullptr;
    const mbgl::CanonicalTileID* canonical = nullptr;
};

template <typename T>
class Result : private variant<EvaluationError, T> {
public:
    using variant<EvaluationError, T>::variant;
    using Value = T;

    Result() = default;

    template <typename U>
    VARIANT_INLINE Result(U&& val) noexcept
        : variant<EvaluationError, T>(val) {}

    explicit operator bool() const noexcept { return this->template is<T>(); }

    // optional does some type trait magic for this one, so this might
    // be problematic as is.
    const T* operator->() const noexcept {
        assert(this->template is<T>());
        return std::addressof(this->template get<T>());
    }

    T* operator->() noexcept {
        assert(this->template is<T>());
        return std::addressof(this->template get<T>());
    }

    T& operator*() noexcept {
        assert(this->template is<T>());
        return this->template get<T>();
    }

    const T& operator*() const noexcept {
        assert(this->template is<T>());
        return this->template get<T>();
    }

    const EvaluationError& error() const noexcept {
        assert(this->template is<EvaluationError>());
        return this->template get<EvaluationError>();
    }
};

class EvaluationResult : public Result<Value> {
public:
    using Result::Result; // NOLINT

    EvaluationResult() = default;

    EvaluationResult(const std::array<double, 4>& arr)
        : Result(toExpressionValue(arr)) {}

    // used only for the special (private) "error" expression
    EvaluationResult(const type::ErrorType&) noexcept { assert(false); }
};

/**
    Expression is an abstract class that serves as an interface and base class
    for particular expression implementations.

    CompoundExpression implements the majority of expressions in the spec by
    inferring the argument and output from a simple function (const T0& arg0,
    const T1& arg1, ...) -> Result<U> where T0, T1, ..., U are member types of
    mbgl::style::expression::Value.

    The other Expression subclasses (Let, Curve, Match, etc.) exist in order to
    implement expressions that need specialized parsing, type checking, or
    evaluation logic that can't be handled by CompoundExpression's inference
    mechanism.

    Each Expression subclass also provides a static
    ParseResult ExpressionClass::parse(const V&, ParsingContext),
    which handles parsing a style-spec JSON representation of the expression.
*/

enum class Kind : int32_t {
    Coalesce,
    CompoundExpression,
    Literal,
    At,
    Interpolate,
    Assertion,
    Length,
    Step,
    Let,
    Var,
    CollatorExpression,
    Coercion,
    Match,
    Error,
    Case,
    Any,
    All,
    Comparison,
    FormatExpression,
    FormatSectionOverride,
    NumberFormat,
    ImageExpression,
    In,
    Within,
    Distance,
    IndexOf,
    Slice
};

class Expression {
public:
    Expression(Kind kind_, type::Type type_)
        : kind(kind_),
          type(std::move(type_)) {}
    virtual ~Expression() = default;

    virtual EvaluationResult evaluate(const EvaluationContext& params) const = 0;
    virtual void eachChild(const std::function<void(const Expression&)>&) const = 0;

    virtual bool operator==(const Expression&) const noexcept = 0;
    bool operator!=(const Expression& rhs) const noexcept { return !operator==(rhs); }

    Kind getKind() const noexcept { return kind; };
    type::Type getType() const noexcept { return type; };

    EvaluationResult evaluate(std::optional<float> zoom,
                              const Feature& feature,
                              std::optional<double> colorRampParameter) const;
    EvaluationResult evaluate(std::optional<float> zoom,
                              const Feature& feature,
                              std::optional<double> colorRampParameter,
                              const std::set<std::string>& availableImages) const;
    EvaluationResult evaluate(std::optional<float> zoom,
                              const Feature& feature,
                              std::optional<double> colorRampParameter,
                              const std::set<std::string>& availableImages,
                              const CanonicalTileID& canonical) const;
    EvaluationResult evaluate(std::optional<mbgl::Value> accumulated, const Feature& feature) const;

    /**
     * Statically analyze the expression, attempting to enumerate possible
     * outputs. Returns an array of values plus the sentinel null optional
     * value, used to indicate that the complete set of outputs is statically
     * undecidable.
     */
    virtual std::vector<std::optional<Value>> possibleOutputs() const = 0;

    virtual mbgl::Value serialize() const {
        std::vector<mbgl::Value> serialized;
        serialized.emplace_back(getOperator());
        eachChild([&](const Expression& child) { serialized.emplace_back(child.serialize()); });
        return serialized;
    };

    virtual std::string getOperator() const = 0;

protected:
    using ExprEquality = decltype(&Expression::operator==);
    static_assert(std::is_nothrow_invocable_v<ExprEquality, const Expression&, const Expression&>);

    template <typename T>
    static bool childrenEqual(const T& lhs, const T& rhs) noexcept {
        if (lhs.size() != rhs.size()) return false;
        for (auto leftChild = lhs.begin(), rightChild = rhs.begin(); leftChild != lhs.end();
             leftChild++, rightChild++) {
            if (!Expression::childEqual(*leftChild, *rightChild)) {
                return false;
            }
        }
        return true;
    }

    static bool childEqual(const std::unique_ptr<Expression>& lhs, const std::unique_ptr<Expression>& rhs) noexcept {
        return *lhs == *rhs;
    }

    template <typename T, typename = std::enable_if_t<std::is_scalar_v<T>>>
    static bool childEqual(const std::pair<T, std::unique_ptr<Expression>>& lhs,
                           const std::pair<T, std::unique_ptr<Expression>>& rhs) noexcept {
        return lhs.first == rhs.first && *(lhs.second) == *(rhs.second);
    }

    template <typename T, typename = std::enable_if_t<std::is_scalar_v<T>>, typename = void>
    static bool childEqual(const std::pair<T, std::shared_ptr<Expression>>& lhs,
                           const std::pair<T, std::shared_ptr<Expression>>& rhs) noexcept {
        return lhs.first == rhs.first && *(lhs.second) == *(rhs.second);
    }

    template <typename T, typename = std::enable_if_t<!std::is_scalar_v<T>>>
    static bool childEqual(const std::pair<T, std::shared_ptr<Expression>>& lhs,
                           const std::pair<T, std::shared_ptr<Expression>>& rhs) noexcept {
#if __clang_major__ > 16
        // On older compilers, this assignment doesn't do overload resolution
        // in the same way that `lhs==rhs` does and produces ambiguity errors.
        bool (*equality_method)(const T&, const T&) noexcept = &std::operator==;
        static_assert(std::is_nothrow_invocable_v<decltype(equality_method), const T&, const T&>);
#endif
        return lhs.first == rhs.first && *(lhs.second) == *(rhs.second);
    }

    static bool childEqual(const std::pair<std::unique_ptr<Expression>, std::unique_ptr<Expression>>& lhs,
                           const std::pair<std::unique_ptr<Expression>, std::unique_ptr<Expression>>& rhs) noexcept {
        return *(lhs.first) == *(rhs.first) && *(lhs.second) == *(rhs.second);
    }

private:
    Kind kind;
    type::Type type;
};

} // namespace expression
} // namespace style
} // namespace mbgl
