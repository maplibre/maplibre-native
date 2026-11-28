#include <mln/test/util.hpp>

#include <mln/style/properties.hpp>
#include <mln/style/layers/fill_layer_properties.hpp>
#include <mln/style/layers/heatmap_layer_properties.hpp>
#include <mln/style/light_impl.hpp>
#include <mln/style/expression/dsl.hpp>
#include <mln/renderer/property_evaluator.hpp>
#include <mln/renderer/data_driven_property_evaluator.hpp>

using namespace mln;
using namespace mln::style;
using namespace std::literals::chrono_literals;

float evaluate(Transitioning<PropertyValue<float>>& property, Duration delta = Duration::zero()) {
    ZoomHistory zoomHistory;
    zoomHistory.update(0, TimePoint::min() + delta);

    PropertyEvaluationParameters parameters{zoomHistory, TimePoint::min() + delta, Duration::zero()};

    PropertyEvaluator<float> evaluator{parameters, 0.0f};

    return property.evaluate(evaluator, parameters.now);
}

PossiblyEvaluatedPropertyValue<float> evaluateDataExpression(Transitioning<PropertyValue<float>>& property,
                                                             Duration delta = Duration::zero()) {
    ZoomHistory zoomHistory;
    zoomHistory.update(0, TimePoint::min() + delta);

    PropertyEvaluationParameters parameters{zoomHistory, TimePoint::min() + delta, Duration::zero()};

    DataDrivenPropertyEvaluator<float> evaluator{parameters, 0.0f};

    return property.evaluate(evaluator, parameters.now);
}

TEST(TransitioningPropertyValue, EvaluateDefaultValue) {
    Transitioning<PropertyValue<float>> property;
    ASSERT_EQ(0.0f, evaluate(property));
}

TEST(TransitioningPropertyValue, EvaluateUntransitionedConstant) {
    Transitioning<PropertyValue<float>> property{
        PropertyValue<float>(1.0f), Transitioning<PropertyValue<float>>(), TransitionOptions(), TimePoint::min()};

    ASSERT_EQ(1.0f, evaluate(property));
}

TEST(TransitioningPropertyValue, EvaluateTransitionedConstantWithoutDelay) {
    TransitionOptions transition;
    transition.duration = {1000ms};

    Transitioning<PropertyValue<float>> t0{
        PropertyValue<float>(0.0f), Transitioning<PropertyValue<float>>(), TransitionOptions(), TimePoint::min()};

    Transitioning<PropertyValue<float>> t1{PropertyValue<float>(1.0f), t0, transition, TimePoint::min()};

    ASSERT_FLOAT_EQ(0.0f, evaluate(t1, 0ms));
    ASSERT_FLOAT_EQ(0.823099f, evaluate(t1, 500ms));
    ASSERT_FLOAT_EQ(1.0f, evaluate(t1, 1500ms));
}

TEST(TransitioningPropertyValue, EvaluateTransitionedConstantWithDelay) {
    TransitionOptions transition;
    transition.delay = {1000ms};
    transition.duration = {1000ms};

    Transitioning<PropertyValue<float>> t0{
        PropertyValue<float>(0.0f), Transitioning<PropertyValue<float>>(), TransitionOptions(), TimePoint::min()};

    Transitioning<PropertyValue<float>> t1{PropertyValue<float>(1.0f), t0, transition, TimePoint::min()};

    ASSERT_FLOAT_EQ(0.0f, evaluate(t1, 0ms));
    ASSERT_FLOAT_EQ(0.0f, evaluate(t1, 500ms));
    ASSERT_FLOAT_EQ(0.0f, evaluate(t1, 612ms));
    ASSERT_FLOAT_EQ(0.823099f, evaluate(t1, 1500ms));
    ASSERT_FLOAT_EQ(1.0f, evaluate(t1, 2500ms));
}

TEST(TransitioningDataDrivenPropertyValue, Evaluate) {
    TransitionOptions transition;
    transition.delay = {1000ms};
    transition.duration = {1000ms};

    Transitioning<PropertyValue<float>> t0{
        PropertyValue<float>(0.0f), Transitioning<PropertyValue<float>>(), TransitionOptions(), TimePoint::min()};

    using namespace mln::style::expression::dsl;
    PropertyExpression<float> expression(number(get("property_name")));

    Transitioning<PropertyValue<float>> t1{PropertyValue<float>(expression), t0, transition, TimePoint::min()};

    ASSERT_TRUE(evaluateDataExpression(t0, 0ms).isConstant());
    ASSERT_FALSE(evaluateDataExpression(t1, 0ms).isConstant())
        << "A paint property transition to a data-driven evaluates immediately "
           "to the final value (see "
           "https://github.com/mapbox/mapbox-gl-native/issues/8237).";
}

TEST(TransitionableProperties, PropertyWithoutTransitionTakesNewValueAtOnce) {
    static_assert(FillColor::SupportsTransition);
    static_assert(!FillTranslateAnchor::SupportsTransition);
    static_assert(!HeatmapColor::SupportsTransition);
    static_assert(!LightAnchor::SupportsTransition);

    FillPaintProperties::Transitionable paint;
    auto unevaluated = paint.untransitioned();
    paint.get<FillColor>().value = Color::red();
    paint.get<FillTranslateAnchor>().value = TranslateAnchorType::Viewport;

    TransitionOptions transition;
    transition.duration = {1000ms};
    unevaluated = paint.transitioned({.now = TimePoint::min(), .transition = transition}, std::move(unevaluated));

    EXPECT_TRUE(unevaluated.get<FillColor>().hasTransition());
    EXPECT_FALSE(unevaluated.get<FillTranslateAnchor>().hasTransition());

    ZoomHistory zoomHistory;
    zoomHistory.update(0, TimePoint::min());
    const auto evaluated = unevaluated.evaluate(
        PropertyEvaluationParameters{zoomHistory, TimePoint::min(), Duration::zero()});
    EXPECT_EQ(TranslateAnchorType::Viewport, evaluated.get<FillTranslateAnchor>());
}
