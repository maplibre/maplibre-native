#pragma once

#include <mbgl/renderer/possibly_evaluated_property_value.hpp>
#include <mbgl/renderer/property_evaluation_parameters.hpp>
#include <mbgl/renderer/transition_parameters.hpp>
#include <mbgl/style/color_ramp_property_value.hpp>
#include <mbgl/style/conversion/stringify.hpp>
#include <mbgl/style/transition_options.hpp>
#include <mbgl/util/convert.hpp>
#include <mbgl/util/indexed_tuple.hpp>
#include <mbgl/util/ignore.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/util/constants.hpp>

#include <bitset>
#include <mbgl/util/unitbezier.hpp>
#include <tuple>

namespace mbgl {

class GeometryTileFeature;

namespace style {

template <class Value>
class Transitioning {
public:
    Transitioning() = default;

    explicit Transitioning(Value value_)
        : value(std::move(value_)) {}

    Transitioning(Value value_, Transitioning<Value> prior_, const TransitionOptions& transition, TimePoint now)
        : begin(now + transition.delay.value_or(Duration::zero())),
          end(begin + transition.duration.value_or(Duration::zero())),
          ease(transition.ease.value_or(util::DEFAULT_TRANSITION_EASE)),
          value(std::move(value_)) {
        if (transition.isDefined()) {
            prior = {std::move(prior_)};
        }
    }

    template <class Evaluator>
    auto evaluate(const Evaluator& evaluator, TimePoint now) const {
        auto finalValue = value.evaluate(evaluator);
        if (!prior) {
            // No prior value.
            return finalValue;
        } else if (now >= end) {
            // Transition from prior value is now complete.
            prior = {};
            return finalValue;
        } else if (value.isDataDriven()) {
            // Transitions to data-driven properties are not supported.
            // We snap immediately to the data-driven value so that, when we
            // perform layout, we see the data-driven function and can use it to
            // populate vertex buffers.
            prior = {};
            return finalValue;
        } else if (now < begin) {
            // Transition hasn't started yet.
            return prior->get().evaluate(evaluator, now);
        } else {
            // Interpolate between recursively-calculated prior value and final.
            float t = std::chrono::duration<float>(now - begin) / (end - begin);
            return util::interpolate(
                prior->get().evaluate(evaluator, now), finalValue, static_cast<float>(ease.solve(t, 0.001)));
        }
    }

    bool hasTransition() const noexcept { return bool(prior); }

    bool isTransitioning(TimePoint now) const {
        return hasTransition() && (begin <= now) && (now < end) && !value.isDataDriven();
    }

    bool isUndefined() const noexcept { return value.isUndefined(); }

    Value& getValue() noexcept { return value; }
    const Value& getValue() const noexcept { return value; }

private:
    mutable std::optional<mapbox::util::recursive_wrapper<Transitioning<Value>>> prior;
    TimePoint begin;
    TimePoint end;
    util::UnitBezier ease = util::DEFAULT_TRANSITION_EASE;
    Value value;
};

template <class Value>
class Transitionable {
public:
    Value value;
    TransitionOptions options;

    Transitioning<Value> transition(const TransitionParameters& params, Transitioning<Value> prior) const {
        return Transitioning<Value>(value, std::move(prior), options.reverseMerge(params.transition), params.now);
    }
};

template <class P>
struct IsDataDriven : std::integral_constant<bool, P::IsDataDriven> {};

template <class P>
struct IsOverridable : std::integral_constant<bool, P::IsOverridable> {};

template <class Ps>
struct ConstantsMask;

template <class... Ps>
struct ConstantsMask<TypeList<Ps...>> {
    template <class Properties>
    static unsigned long getMask(const Properties& properties) {
        const auto result = std::apply(
            [](auto... v) { return (v | ...); },
            std::make_tuple(
                0ul,
                (((properties.template get<Ps>().isConstant()) ? (1ul << (TypeIndex<Ps, Ps...>::value)) : 0ul))...));
        return result;
    }
};

template <class... Ps>
class Properties {
public:
    /*
        For style properties we implement a two step evaluation process: if you
       have a zoom level, you can evaluate a set of unevaluated property values,
       producing a set of possibly evaluated values, where undefined, constant,
       or camera function values have been fully evaluated, and source or
       composite function values have not.

        Once you also have a particular feature, you can evaluate that set of
       possibly evaluated values fully, producing a set of fully evaluated
       values.

        This is in theory maximally efficient in terms of avoiding repeated
       evaluation of camera functions, though it's more of a historical accident
       than a purposeful optimization.
    */

    using PropertyTypes = TypeList<Ps...>;
    using TransitionableTypes = TypeList<typename Ps::TransitionableType...>;
    using UnevaluatedTypes = TypeList<typename Ps::UnevaluatedType...>;
    using PossiblyEvaluatedTypes = TypeList<typename Ps::PossiblyEvaluatedType...>;
    using EvaluatedTypes = TypeList<typename Ps::Type...>;

    using DataDrivenProperties = FilteredTypeList<PropertyTypes, IsDataDriven>;
    using OverridableProperties = FilteredTypeList<PropertyTypes, IsOverridable>;

    using Dependency = expression::Dependency;

    template <class TypeList>
    using Tuple = IndexedTuple<PropertyTypes, TypeList>;

    class Evaluated : public Tuple<EvaluatedTypes> {
    public:
        template <class... Us>
        Evaluated(Us&&... us)
            : Tuple<EvaluatedTypes>(std::forward<Us>(us)...) {}
    };

    class PossiblyEvaluated : public Tuple<PossiblyEvaluatedTypes> {
    public:
        template <class... Us>
        PossiblyEvaluated(Us&&... us)
            : Tuple<PossiblyEvaluatedTypes>(std::forward<Us>(us)...) {}

        template <class T>
        static T evaluate(float, const GeometryTileFeature&, const T& t, const T&) {
            return t;
        }

        template <class T>
        static T evaluate(float, const GeometryTileFeature&, const CanonicalTileID&, const T& t, const T&) {
            return t;
        }

        template <class T>
        static T evaluate(float z,
                          const GeometryTileFeature& feature,
                          const PossiblyEvaluatedPropertyValue<T>& v,
                          const T& defaultValue) {
            return v.match([&](const T& t) { return t; },
                           [&](const PropertyExpression<T>& t) { return t.evaluate(z, feature, defaultValue); });
        }

        template <class T>
        static T evaluate(float z,
                          const GeometryTileFeature& feature,
                          const PossiblyEvaluatedPropertyValue<T>& v,
                          const T& defaultValue,
                          const std::set<std::string>& availableImages) {
            return v.match(
                [&](const T& t) { return t; },
                [&](const PropertyExpression<T>& t) { return t.evaluate(z, feature, availableImages, defaultValue); });
        }

        template <class T>
        static T evaluate(float z,
                          const GeometryTileFeature& feature,
                          const PossiblyEvaluatedPropertyValue<T>& v,
                          const T& defaultValue,
                          const std::set<std::string>& availableImages,
                          const CanonicalTileID& canonical) {
            return v.match([&](const T& t) { return t; },
                           [&](const PropertyExpression<T>& t) {
                               return t.evaluate(z, feature, availableImages, canonical, defaultValue);
                           });
        }

        template <class T>
        static T evaluate(float z,
                          const GeometryTileFeature& feature,
                          const CanonicalTileID& canonical,
                          const PossiblyEvaluatedPropertyValue<T>& v,
                          const T& defaultValue) {
            return v.match(
                [&](const T& t) { return t; },
                [&](const PropertyExpression<T>& t) { return t.evaluate(z, feature, canonical, defaultValue); });
        }

        template <class T>
        static T evaluate(float z,
                          const GeometryTileFeature& feature,
                          const FeatureState& state,
                          const PossiblyEvaluatedPropertyValue<T>& v,
                          const T& defaultValue) {
            return v.match([&](const T& t) { return t; },
                           [&](const PropertyExpression<T>& t) { return t.evaluate(z, feature, state, defaultValue); });
        }

        template <class P>
        auto evaluate(float z, const GeometryTileFeature& feature) const {
            return evaluate(z, feature, this->template get<P>(), P::defaultValue());
        }

        template <class P>
        auto evaluate(float z, const GeometryTileFeature& feature, const CanonicalTileID& canonical) const {
            return evaluate(z, feature, canonical, this->template get<P>(), P::defaultValue());
        }

        template <class P>
        auto evaluate(float z, const GeometryTileFeature& feature, const FeatureState& state) const {
            return evaluate(z, feature, state, this->template get<P>(), P::defaultValue());
        }

        template <class P>
        auto evaluate(float z, const GeometryTileFeature& feature, const std::set<std::string>& availableImages) const {
            return evaluate(z, feature, this->template get<P>(), P::defaultValue(), availableImages);
        }

        template <class P>
        auto evaluate(float z,
                      const GeometryTileFeature& feature,
                      const std::set<std::string>& availableImages,
                      const CanonicalTileID& canonical) const {
            return evaluate(z, feature, this->template get<P>(), P::defaultValue(), availableImages, canonical);
        }

        Evaluated evaluate(float z, const GeometryTileFeature& feature) const {
            return Evaluated{evaluate<Ps>(z, feature)...};
        }

        /// Extract dependencies from a possibly-evaluated property which may have an expression.
        template <class P>
        Dependency getDependencies(const P&) const noexcept {
            return Dependency::None;
        }
        template <class P>
        Dependency getDependencies(const PossiblyEvaluatedPropertyValue<P>& v) const noexcept {
            return v.getDependencies();
        }
        template <class P>
        Dependency getDependencies(const PossiblyEvaluatedPropertyValue<Faded<P>>& v) const noexcept {
            return v.getDependencies();
        }

        Dependency getDependencies() const noexcept {
            Dependency result = Dependency::None;
            util::ignore({(result |= getDependencies(this->template get<Ps>()))...});
            return result;
        }

        unsigned long constantsMask() const noexcept { return ConstantsMask<DataDrivenProperties>::getMask(*this); }
    };

    class Unevaluated : public Tuple<UnevaluatedTypes> {
    public:
        template <class... Us>
        Unevaluated(Us&&... us)
            : Tuple<UnevaluatedTypes>(std::forward<Us>(us)...) {}

        bool hasTransition() const {
            bool result = false;
            util::ignore({result |= this->template get<Ps>().hasTransition()...});
            return result;
        }

        template <class P>
        auto evaluate(const PropertyEvaluationParameters& parameters) const {
            using Evaluator = typename P::EvaluatorType;
            return this->template get<P>().evaluate(Evaluator(parameters, P::defaultValue()), parameters.now);
        }

        PossiblyEvaluated evaluate(const PropertyEvaluationParameters& parameters) const {
            return PossiblyEvaluated{evaluate<Ps>(parameters)...};
        }

        /// Evaluate the property if necessary, or produce a copy of the previous value if appropriate
        template <class P>
        auto maybeEvaluate(const PropertyEvaluationParameters& parameters,
                           const typename P::EvaluatorType::ResultType& oldResult) const {
            using Evaluator = typename P::EvaluatorType;
            const auto& property = this->template get<P>();
            const bool needEvaluate = parameters.layerChanged || parameters.hasCrossfade || property.hasTransition() ||
                                      (parameters.zoomChanged && (getDependencies(property) & Dependency::Zoom));
            return needEvaluate ? property.evaluate(Evaluator(parameters, P::defaultValue()), parameters.now)
                                : oldResult;
        }

        /// Optionally evaluate each property or produce a copy of the previous value, if appropriate.
        PossiblyEvaluated evaluate(const PropertyEvaluationParameters& parameters,
                                   const PossiblyEvaluated& previous) const {
            return PossiblyEvaluated{maybeEvaluate<Ps>(parameters, previous.template get<Ps>())...};
        }

        template <class Writer>
        void stringify(Writer& writer) const {
            writer.StartObject();
            util::ignore({(conversion::stringify<Ps>(writer, this->template get<Ps>()), 0)...});
            writer.EndObject();
        }

        unsigned long constantsMask() const { return ConstantsMask<DataDrivenProperties>::getMask(*this); }

        /// Get the combined dependencies of any contained expressions
        constexpr Dependency getDependencies() const noexcept {
            Dependency result = Dependency::None;
            util::ignore({(result |= getDependencies(this->template get<Ps>()))...});
            return result;
        }

        using GPUExpressions = std::array<gfx::UniqueGPUExpression, UnevaluatedTypes::TypeCount>;

        /// Update the GPU expressions, if applicable, for each item in the tuple.
        /// @return true if any are updated (including being cleared)
        bool updateGPUExpressions(Unevaluated::GPUExpressions& exprs, TimePoint now) const {
            const auto results = util::to_array(std::make_tuple(updateGPUExpression<Ps>(exprs, now)...));
            return std::any_of(results.begin(), results.end(), [](bool x) { return x; });
        }

    protected:
        // gather dependencies for each type that can appear in this tuple

        template <class P>
        Dependency getDependencies(const PropertyValue<P>& v) const noexcept {
            return v.getDependencies();
        }

        template <class P>
        Dependency getDependencies(const Transitioning<P>& v) const noexcept {
            return v.getValue().getDependencies();
        }

        template <typename P>
        bool updateGPUExpression(Unevaluated::GPUExpressions& exprs, TimePoint now) const {
            constexpr auto index = TypeIndex<P, Ps...>::value;
            constexpr bool forceIntZoom = P::EvaluatorType::useIntegerZoom;
            return updateGPUExpression(exprs[index], this->template get<P>(), now, forceIntZoom);
        }

        template <class P>
        static bool updateGPUExpression(gfx::UniqueGPUExpression& expr,
                                        const PropertyValue<P>& val,
                                        TimePoint /*now*/,
                                        bool intZoom) {
            if (val.isExpression() && val.asExpression().isGPUCapable()) {
                if (!expr) {
                    expr = val.asExpression().getGPUExpression(intZoom);
                    return true;
                }
            } else if (expr) {
                // Previously set and shouldn't be
                expr.reset();
                return true;
            }
            return false;
        }
        template <class P>
        static bool updateGPUExpression(gfx::UniqueGPUExpression& expr,
                                        const Transitioning<P>& val,
                                        TimePoint now,
                                        bool intZoom) {
            if (val.isTransitioning(now)) {
                if (expr) {
                    expr.reset();
                    return true;
                }
                return false;
            }
            return updateGPUExpression(expr, val.getValue(), now, intZoom);
        }
        static bool updateGPUExpression(gfx::UniqueGPUExpression&,
                                        const style::ColorRampPropertyValue&,
                                        TimePoint,
                                        bool) {
            return false;
        }
    };

    class Transitionable : public Tuple<TransitionableTypes> {
    public:
        template <class... Us>
        Transitionable(Us&&... us)
            : Tuple<TransitionableTypes>(std::forward<Us>(us)...) {}

        Unevaluated transitioned(const TransitionParameters& parameters, Unevaluated&& prior) const {
            return Unevaluated{this->template get<Ps>().transition(parameters, std::move(prior.template get<Ps>()))...};
        }

        Unevaluated untransitioned() const {
            return Unevaluated{typename Ps::UnevaluatedType(this->template get<Ps>().value)...};
        }

        bool hasDataDrivenPropertyDifference(const Transitionable& other) const {
            bool result = false;
            util::ignore({(result |= this->template get<Ps>().value.hasDataDrivenPropertyDifference(
                               other.template get<Ps>().value))...});
            return result;
        }

        Dependency getDependencies() const noexcept {
            Dependency result = Dependency::None;
            util::ignore({(result |= getDependencies(this->template get<Ps>()))...});
            return result;
        }

    protected:
        template <typename P>
        Dependency getDependencies(const style::Transitionable<PropertyValue<P>>& v) const noexcept {
            return v.value.getDependencies();
        }
        Dependency getDependencies(const style::ColorRampPropertyValue& v) const noexcept {
            return v.getDependencies();
        }
        Dependency getDependencies(const style::Transitionable<ColorRampPropertyValue>& v) const noexcept {
            return v.value.getDependencies();
        }
    };
};

template <class... Ps>
using ConcatenateProperties = typename TypeListConcat<typename Ps::PropertyTypes...>::template ExpandInto<Properties>;

template <typename T, class... Is, class... Ts>
auto constOrDefault(const IndexedTuple<TypeList<Is...>, TypeList<Ts...>>& evaluated) {
    return evaluated.template get<T>().constantOr(T::defaultValue());
}

} // namespace style
} // namespace mbgl
