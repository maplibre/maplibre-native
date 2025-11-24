#pragma once

#include <mbgl/style/expression/parsing_context.hpp>
#include <mbgl/style/expression/type.hpp>
#include <mbgl/style/expression/value.hpp>
#include <mbgl/tile/tile_id.hpp>
#include <mbgl/util/bitmask_operations.hpp>
#include <mbgl/util/color.hpp>
#include <mbgl/util/traits.hpp>
#include <mbgl/util/variant.hpp>

#include <array>
#include <memory>
#include <numeric>
#include <optional>
#include <vector>

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
    EvaluationContext(std::optional<mbgl::Value> accumulated_, GeometryTileFeature const* feature_) noexcept(false)
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

    EvaluationContext(std::optional<float> zoom_,
                  GeometryTileFeature const* feature_,
                  std::optional<double> colorRampParameter_,
                  std::optional<float> elevation_) noexcept
    : zoom(std::move(zoom_)),
      feature(feature_),
      colorRampParameter(std::move(colorRampParameter_)),
      elevation(std::move(elevation_)) {}

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

    EvaluationContext& withElevation(float elevation_) noexcept {
        elevation = elevation_;
        return *this;
    };

    std::optional<float> zoom;
    std::optional<mbgl::Value> accumulated;
    GeometryTileFeature const* feature = nullptr;
    std::optional<double> colorRampParameter;
    std::optional<float> elevation;
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
    using Variant = variant<EvaluationError, T>;

    Result() = default;

    static_assert(std::is_nothrow_constructible_v<Variant>);

    template <typename U>
    VARIANT_INLINE Result(U&& val)
        : Variant(std::forward<U>(val)) {}

    template <typename U>
    VARIANT_INLINE Result(const U& val)
        : Variant(val) {}

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

enum class Dependency : uint32_t {
    None = 0,
    Feature = 1 << 0,  // Data reference
    Image = 1 << 1,    // Image reference (equivalent to not "runtime constant")
    Zoom = 1 << 2,     // Zoom level
    Location = 1 << 3, // Not used yet, "distance-from-center" not supported
    Bind = 1 << 4,     // Create variable binding ("let")
    Var = 1 << 5,      // Use variable binding
    Override = 1 << 6, // Property override
    Elevation = 1 << 7, // Elevation from DEM
    MaskCount = 8,
    All = (1 << MaskCount) - 1,
};

class Interpolate;
class Step;

using ZoomCurveOrError = std::optional<variant<const Interpolate*, const Step*, ParsingError>>;
using ZoomCurvePtr = variant<std::nullptr_t, const Interpolate*, const Step*>;

class Expression {
public:
    Expression(Kind kind_, type::Type type_, Dependency dependencies_)
        : dependencies(dependencies_),
          kind(kind_),
          type(std::move(type_)) {}
    virtual ~Expression() = default;

    virtual EvaluationResult evaluate(const EvaluationContext& params) const = 0;
    virtual void eachChild(const std::function<void(const Expression&)>&) const = 0;

    virtual bool operator==(const Expression&) const = 0;
    bool operator!=(const Expression&) const = default;

    Kind getKind() const noexcept { return kind; };
    const type::Type& getType() const noexcept { return type; };

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

    const Dependency dependencies;

    /// Test the expression's dependencies for any of one or more values
    bool has(Dependency dep) const { return (dependencies & dep); }

protected:
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

    template <typename T>
    static bool childEqual(const std::pair<T, std::unique_ptr<Expression>>& lhs,
                           const std::pair<T, std::unique_ptr<Expression>>& rhs) noexcept
        requires(std::is_scalar_v<T>)
    {
        return lhs.first == rhs.first && *(lhs.second) == *(rhs.second);
    }

    template <typename T, typename = std::enable_if_t<std::is_scalar_v<T>>, typename = void>
    static bool childEqual(const std::pair<T, std::shared_ptr<Expression>>& lhs,
                           const std::pair<T, std::shared_ptr<Expression>>& rhs) noexcept {
        return lhs.first == rhs.first && *(lhs.second) == *(rhs.second);
    }

    template <typename T>
    static bool childEqual(const std::pair<T, std::shared_ptr<Expression>>& lhs,
                           const std::pair<T, std::shared_ptr<Expression>>& rhs) noexcept
        requires(!std::is_scalar_v<T>)
    {
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

    static Dependency depsOf(const std::shared_ptr<Expression>& ex) { return ex ? ex->dependencies : Dependency::None; }
    static Dependency depsOf(const std::unique_ptr<Expression>& ex) { return ex ? ex->dependencies : Dependency::None; }

    template <typename T>
    static Dependency depsOf(const std::optional<T>& ex) {
        return ex ? depsOf(*ex) : Dependency::None;
    }

    /// Combine the dependencies of all the expressions in a container of shared- or unique-pointers to expressions
    template <typename T>
    static constexpr Dependency collectDependencies(const std::vector<T>& container) {
        return std::accumulate(container.begin(), container.end(), Dependency::None, [](auto a, const auto& ex) {
            return a | depsOf(ex);
        });
    }

    template <typename T, typename F>
    static constexpr Dependency collectDependencies(const std::vector<T>& container, F op) {
        return std::accumulate(container.begin(), container.end(), Dependency::None, [&](auto a, const auto& item) {
            return a | op(item);
        });
    }

    template <typename K, typename T>
    static constexpr Dependency collectDependencies(const std::map<K, T>& container) {
        return std::accumulate(container.begin(), container.end(), Dependency::None, [](auto a, const auto& v) {
            return a | depsOf(v.second);
        });
    }

    template <typename K, typename T>
    static constexpr Dependency collectDependencies(const std::unordered_map<K, T>& container) {
        return std::accumulate(container.begin(), container.end(), Dependency::None, [](auto a, const auto& kv) {
            return a | depsOf(kv.second);
        });
    }

private:
    Kind kind;
    type::Type type;
};

} // namespace expression
} // namespace style

namespace util {
std::string toString(style::expression::Dependency);
} // namespace util

std::ostream& operator<<(std::ostream&, style::expression::Dependency);

} // namespace mbgl
